/*-----------------------------------------------------------------------------
    Full nim 'Sharp6158' composed by demodulater mn88472 + tuner MXL301.

    History:
    2011/11/21 by WangWenKai.
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>


#include "nim_sharp6158.h"

#include "MN_I2C.h"
#include "MN_DMD_driver.h"
#include "MN_DMD_device.h"
#include "MN_DMD_common.h"
#include "MN_TCB.h"
#include "MN88472.h"

DMD_ERROR_t DMD_I2C_Write(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t data );
DMD_ERROR_t DMD_I2C_Read(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t *data );

//#define SHARP6158_DEBUG        libc_printf
#define SHARP6158_DEBUG(...)

#define BER_TIMEOUT_VAL  5000

#define ARRAY_SIZE(x)        (sizeof(x)/sizeof(x[0]))

#define NIM_FLAG_ENABLE    0x00000100
#define NIM_SCAN_END       0x00000001

#define LOG_STRING_BUFFER_SIZE    255

#define SHARP6158_LOG        DMD_DEBUG_LOG

#define SHARP6158_LOG_I2C        Sharp6158_log_i2c
//#define SHARP6158_LOG_I2C(...)         do{}while(0)

void Sharp6158_log_i2c(DMD_PARAMETER_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len)
{
    int i;
    if ( ! (param->output_buffer && param->fn_output_string) )
        return;

    if (write)
        SHARP6158_LOG(param, "I2C_Write,0x%02X", (slv_addr&0xFE));
    else
        SHARP6158_LOG(param, "I2C_Read,0x%02X", (slv_addr|1));

    for ( i=0; i<len; ++i )
    {
        SHARP6158_LOG(param, ",0x%02X", data[i]);
    }

    if (err)
        SHARP6158_LOG(param, "\terror");
    SHARP6158_LOG(param, "\r\n");    //\r
}

//void DMD_read_registers( DMD_PARAMETER_t* param )
//{
//    DMD_u32_t   i;
//    DMD_u8_t rd;
//
//    libc_printf("\n DMD_read_registers _DVB-T2--- in! \r\n");
//    for(i=0;;i++)
//    {
//        DMD_I2C_Read(param, (0x38), i, &rd );                // 1st register : (0x1C <<1)
//        libc_printf("Regaddr[0x%x]  =  0x%x     \r\n",i,rd);
//        if( i == 0xff ) break;
//    }
//
//    libc_printf("\n DMD_read_registers _DVB-T-- in! \r\n"); 
//    for(i=0;;i++)
//    {
//        DMD_I2C_Read(param, (0x30), i, &rd );                // 2nd register : (0x18 <<1)
//        libc_printf("Regaddr[0x%x]  =  0x%x     \r\n",i,rd);
//        if( i == 0xff ) break;
//    }
//
//    libc_printf("\n DMD_read_registers out ! \r\n");
//}


static INT32 nim_sharp6158_get_BER_PER_log(struct nim_device *dev)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    DMD_u32_t ber_err=0, ber_sum=0;
    DMD_u32_t per_err=0, per_sum=0;
    OSAL_ER result;
    UINT32 flgptn;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {

        DMD_BER(priv, priv->system, &ber_err, &ber_sum);
        DMD_PER(priv, priv->system, &per_err, &per_sum);
        SHARP6158_LOG(priv,"(ber_err, ber_sum) = (%d, %d), (per_err, per_sum) = (%d, %d)\r\n", ber_err, ber_sum, per_err, per_sum);
        return SUCCESS;
    }
    else
        return ERR_FAILUE;
}


//=============================================================================
// MN_I2C.c
//=============================================================================

/*! Write 1byte */
DMD_ERROR_t DMD_I2C_Write(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t data )
{
/* '11/08/01 : OKAMOTO    Implement I2C read / write handler. */
    UINT8 apData[2];
    UINT16 length;
    DMD_ERROR_t ret = DMD_E_OK;

    osal_mutex_lock(param->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);

    apData[0] = adr;
    apData[1] = data;
    length = (UINT16)sizeof(apData);

    if(SUCCESS != i2c_write(param->tc.ext_dm_config.i2c_type_id, slvadr, apData, length))
        ret = DMD_E_ERROR;
    SHARP6158_LOG_I2C(param, ret, 1, slvadr, apData, length);

    osal_mutex_unlock(param->i2c_mutex_id);
    return ret;
}

/*! Read 1byte */
DMD_ERROR_t DMD_I2C_Read(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t *data )
{
/* '11/08/01 : OKAMOTO    Implement I2C read / write handler. */
    UINT8 apData[1];
    UINT16 length = 1;
    DMD_ERROR_t ret = DMD_E_OK;

    osal_mutex_lock(param->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);

    apData[0] = adr;

    if(SUCCESS != i2c_write(param->tc.ext_dm_config.i2c_type_id, slvadr, apData, length))
        ret = DMD_E_ERROR;
    SHARP6158_LOG_I2C(param, ret, 1, slvadr, apData, length);

    /* '11/08/05 : OKAMOTO Correct I2C read. */
    if(SUCCESS != i2c_read(param->tc.ext_dm_config.i2c_type_id, slvadr, data, 1))
        ret = DMD_E_ERROR;
    SHARP6158_LOG_I2C(param, ret, 0, slvadr, data, 1);

    osal_mutex_unlock(param->i2c_mutex_id);
    return ret;
}

/*! Write/Read any Length*/
/*====================================================*
    DMD_I2C_WriteRead
   --------------------------------------------------
    Description     I2C Write/Read any Length.
    Argument
                    <Common Prametor>
                     slvadr (Slave Addr of demod without R/W bit.)
                    <Write Prametors>
                     adr (Address of demod.) ,
                     wdata (Write data)
                     wlen (Write length)
                    <Read Prametors>
                     rdata (Read result)
                     rlen (Read length)
    Return Value    DMD_ERROR_t (DMD_E_OK:success, DMD_E_ERROR:error)
 *====================================================*/
DMD_ERROR_t DMD_I2C_WriteRead(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u32_t rlen)
{
    {
        //Write
        UINT16 length;
        if((wlen+1)>0xFFFF)
            return DMD_E_ERROR;
        else
            length = (UINT16)wlen + 1;

        osal_mutex_lock(param->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);

        {
            UINT8 *data = NULL;
            UINT8 *pNewBuf = NULL;
            UINT8 buf[128]; //In order to avoid to call malloc() frequently for small size command.
            if (length > 128)
            {
                pNewBuf = (UINT8*)malloc(length);
                if (NULL == pNewBuf)
                    return DMD_E_ERROR;
                data = pNewBuf;
            }
            else
            {
                data = buf;
                if (wlen+1 > 128)
                    return DMD_E_ERROR;
            }

            data[0] = adr;
            MEMCPY(&data[1], wdata, wlen);
            {
                BOOL bRetVal = i2c_write(param->tc.ext_dm_config.i2c_type_id, slvadr, data, length);
                SHARP6158_LOG_I2C(param, bRetVal, 1, slvadr, data, length);
                if (pNewBuf)
                {
                    free (pNewBuf);
                    pNewBuf = NULL;
                }
                if(bRetVal!=SUCCESS)
                {
                    osal_mutex_unlock(param->i2c_mutex_id);
                    return DMD_E_ERROR;
                }
            }
        }
    }

    if(rlen!=0){
        //Read
        BOOL bRetVal;
        UINT16 length;
        length = (UINT16)rlen;
        bRetVal = i2c_read(param->tc.ext_dm_config.i2c_type_id, slvadr, rdata, length);
        SHARP6158_LOG_I2C(param, bRetVal, 0, slvadr, rdata, length);
        if(SUCCESS != bRetVal)
        {
            osal_mutex_unlock(param->i2c_mutex_id);
            return DMD_E_ERROR;
        }
    }
    osal_mutex_unlock(param->i2c_mutex_id);
    return DMD_E_OK;
}

/* **************************************************** */
/*! Write&Read any length from/to Tuner via Demodulator */
/* **************************************************** */
DMD_ERROR_t DMD_TCB_WriteRead(void* nim_dev_priv, UINT8    tuner_address , UINT8* wdata , int wlen , UINT8* rdata , int rlen)
{
    //DMD_BANK_T2_: T2 demodulator address.
    //data[]: data submitted by tuner driver, they will be sent to tuner.
    DMD_u8_t    d[DMD_TCB_DATA_MAX];
    int i;
    DMD_ERROR_t    ret;
    DMD_PARAMETER_t* param = (DMD_PARAMETER_t*)nim_dev_priv;

    if( wlen >= DMD_TCB_DATA_MAX || rlen >= DMD_TCB_DATA_MAX )
        return DMD_E_ERROR;

    /* Set TCB Through Mode */
#if 0
    ret  = DMD_I2C_Write(param,  DMD_BANK_T2_ , DMD_TCBSET , 0x53 );
#else
    /* '11/11/14 : OKAMOTO    Update to "MN88472_Device_Driver_111028". */
    ret  = DMD_I2C_MaskWrite( param, DMD_BANK_T2_ , DMD_TCBSET , 0x7f , 0x53 );
#endif
    ret |= DMD_I2C_Write(param,  DMD_BANK_T2_ , DMD_TCBADR , 0x00 );

    if( (wlen == 0 && rlen == 0) ||  (wlen != 0) )
    {
        //Write
        d[0] = tuner_address & 0xFE;
        for(i=0;i<wlen;i++)
            d[i+1] = wdata[i];
#if 0
//        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , wlen + 2 , 0 , 0 );
        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , wlen + 1 , 0 , 0 );
#else
        /* Read/Write */
        if( !rdata && rlen != 0 )
            return DMD_E_ERROR;
        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , wlen + 1 , rdata , rlen );
#endif
    }
    else
    {
        //Read
        if( !rdata || rlen == 0 )
            return DMD_E_ERROR;

        d[0] = tuner_address | 1;
//        d[1] = DMD_BANK_T2_ | 1;
//        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , 2 , 0 , 0 );
//        ret |= DMD_I2C_Read(param, DMD_BANK_T2_ , DMD_TCBCOM , data);

//        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , 2 , rdata , 1 );
        ret |= DMD_I2C_WriteRead(param, DMD_BANK_T2_ , DMD_TCBCOM , d , 1 , rdata , 1 );
    }
    return ret;
}


/* '11/08/05 : OKAMOTO Implement "DMD_wait". */
/*! Timer wait */
DMD_ERROR_t DMD_wait( DMD_u32_t msecond ){
    if(msecond>0xFFFF)
        return DMD_E_ERROR;

//    timer_delay_msec((UINT16) msecond);
    osal_task_sleep(msecond);
    return DMD_E_OK;
}


DMD_ERROR_t MN88472_CMN_slave_address_control(BOOL bSet, DMD_u8_t* p_sadr){
    static DMD_u8_t static_sadr=0;
    if(bSet){
        if(*p_sadr>1)
            return DMD_E_ERROR;
        static_sadr = *p_sadr;
    }else{
        *p_sadr = static_sadr;
    }
    return DMD_E_OK;
}

DMD_ERROR_t MN88472_CMN_slave_address_select(DMD_u8_t sadr){
    return MN88472_CMN_slave_address_control(TRUE, &sadr);
}

/* '11/08/01 : OKAMOTO    Implement I2C read / write handler. */
/*====================================================*
    MN88472_CMN_slave_address_get
   --------------------------------------------------
    Description     Select slave address.(without R/W bit)
    Argument        sys (Set TV system.)
    Return Value    DMD_u8_t(Current slave addr include R/W bit for Ali I2C address.)
 *====================================================*/
DMD_u8_t MN88472_CMN_slave_address_get(DMD_SYSTEM_t sys){
    DMD_u8_t slave_address;

    switch( sys ){
        case DMD_E_DVBT:
            slave_address = 0x18;
            break;
        case DMD_E_DVBT2:
            slave_address = 0x1c;
            break;
        case DMD_E_DVBC:
            slave_address = 0x1a;
            break;
        default:
            return DMD_E_ERROR;
    };
    {
        DMD_u8_t sadr;
        DMD_ERROR_t result = MN88472_CMN_slave_address_control(FALSE, &sadr);
        if(result != DMD_E_OK){
            return result;
        }
        slave_address+=sadr;
    }
    slave_address = (slave_address<<1); //Ali I2C address, make it include R/W bit.

    return slave_address;
}


/* '11/11/14 : OKAMOTO    Update to "MN88472_Device_Driver_111028". */
/*! Get System Time (ms) */
DMD_ERROR_t DMD_timer( DMD_u32_t* tim )
{
    *tim = osal_get_tick();
    return DMD_E_OK;
}

UINT32 DMD_elapsed_time(UINT32 startTime)
{
    return osal_get_tick() - startTime;
}





//=============================================================================
// MN_DMD_driver.c
//=============================================================================

static void nim_sharp6158_switch_lock_led(struct nim_device *dev, BOOL On)
{
    if(((DMD_PARAMETER_t *)dev->priv)->tc.nim_lock_cb)
        ((DMD_PARAMETER_t *)dev->priv)->tc.nim_lock_cb(On);
}


static INT32 config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 *ptr_lock)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 tuner_id = priv->tuner_id;
    UINT8 lock = 0, Tuner_Retry = 0;

//    set_bypass_mode(dev, TRUE);
    do
    {
        Tuner_Retry++;

        if(priv->tc.nim_tuner_control(tuner_id, freq, bandwidth, 0, NULL, 0)==ERR_FAILUE)
        {
            SHARP6158_LOG(priv,"Config tuner failed, I2c failed!\r\n");
        }

        if(priv->tc.nim_tuner_status(tuner_id, &lock) != SUCCESS)
        {
            //if i2c read failure, no lock state can be report.
            lock = 0;
            SHARP6158_LOG(priv,"Config tuner failed, I2c failed!\r\n");
        }
        SHARP6158_LOG(priv,"Tuner Lock Times=0x%d,Status=0x%d\r\n",Tuner_Retry,lock);

        if(Tuner_Retry > 5)
            break;
    }while(0 == lock);
//    set_bypass_mode(dev, FALSE);

    if(ptr_lock != NULL)
        *ptr_lock = lock;

    if(Tuner_Retry > 5)
    {
        SHARP6158_LOG(priv,"ERROR! Tuner Lock Fail\r\n");

//        osal_flag_set(priv->flag_id, NIM_SCAN_END);
        return ERR_FAILUE;
    }
    return SUCCESS;
}

BOOL wait_for_signal_become_stable(DMD_PARAMETER_t * param)
{
    //UINT8 lock = 0;
    INT32 result = ERR_FAILUE;

//    DMD_wait(300);
    do
    {
//        result = config_tuner(dev, param->Frequency, param->bandwidth, &lock); //The tuner locks the frequency.
//        if(result != SUCCESS)
//            break;

        if( DMD_scan( param ) != DMD_E_OK )  //DEM Locks the DVB-T2 signal.
            break;

        result = SUCCESS;
    }while(0);
    return result;
}

static BOOL need_to_config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, DMD_IF_FREQ_t if_freq)
{
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;
    UINT32 tuner_id = param->tuner_id;
    UINT8 lock = 0;

    return ! ( param->Frequency == freq \
            && param->bw == bandwidth \
            && param->if_freq == if_freq \
            && param->tc.nim_tuner_status(tuner_id, &lock) == SUCCESS );
}

static BOOL need_to_lock_DVBT_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para, BOOL NeedToConfigTuner)
{
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

    if ( change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return FALSE;

    //USAGE_TYPE_CHANCHG: for play program.
    if (change_para->t2_signal) //Do nothing for play DVB-T2 program.
        return FALSE;
    if (param->system != DMD_E_DVBT) //DEM is not be DVB-T mode now.
        return TRUE;

    return NeedToConfigTuner;
}

static INT32 need_to_lock_DVBT2_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para, BOOL NeedToConfigTuner, BOOL *p_change_plp_only)
{
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;

    *p_change_plp_only = FALSE;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE \
        || change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

   //USAGE_TYPE_CHANCHG: for play program.
    if ( ! change_para->t2_signal ) //Do nothing for play DVB-T program.
        return FALSE;

    *p_change_plp_only = TRUE;
    if (param->system != DMD_E_DVBT2) //DEM is not be DVB-T2 mode now.
        return TRUE;

//    *p_change_plp_only = TRUE;
    if (param->plp_index != change_para->plp_index) //Current selected PLP is different with the target PLP.
        return TRUE;
    if (param->plp_id != change_para->plp_id) //Current selected PLP is different with the target PLP.
        return TRUE;

    return NeedToConfigTuner;
}

static INT32 try_to_lock_DVBT_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    UINT8 retry = 0;
    UINT8 lock = 0;
    INT32 result = ERR_FAILUE;
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;
    do
    {
        SHARP6158_LOG(param, "%s(NeedToInitSystem=%d, NeedToConfigTuner=%d)\r\n", __FUNCTION__, NeedToInitSystem, NeedToConfigTuner);
        if ( NeedToInitSystem || param->system != DMD_E_DVBT )
        {
            param->t2_signal = 0;
            param->system = DMD_E_DVBT;
            DMD_set_system( param );

            NeedToConfigTuner = TRUE;
        }

        if ( param->system != DMD_E_DVBT )
            return result;

        if ( NeedToConfigTuner )
        {
            DMD_channel_search_setting( param );

            //config_tuner() shall be run before run DMD_scan( param ),
            //Otherwise it maybe hard to lock DVB-T signal.
            result = config_tuner(dev, param->Frequency, param->bandwidth, &lock); //The tuner locks the frequency.
            if(result != SUCCESS)
            {
                break;
            }

            if( DMD_scan( param ) != DMD_E_OK )  //DEM Locks the DVB-T signal.
            {
                result = ERR_FAILUE;
                break;
            }
        }

        do
        {
            DMD_get_info(param, DMD_E_INFO_DVBT_LOCK);
            if ( DMD_E_LOCKED == param->info[DMD_E_INFO_DVBT_LOCK] )
            {
    //            DMD_READ_REGISTERS(param);
                result = SUCCESS;
                break;
            }
            DMD_wait(100);
            ++retry;
        }while(retry < 6); //Bug fixed: CDT_MN88472_MXL603 need to wait 300ms to get locked status if it scan same channel multiple times.
        if (retry != 0)
            SHARP6158_LOG(param, "%s(NeedToInitSystem=%d, NeedToConfigTuner=%d): check locked retry %d times, %s\r\n", __FUNCTION__, NeedToInitSystem, NeedToConfigTuner, retry, (SUCCESS == result?"DMD_E_LOCKED":"unlocked"));
    }while(0);

    return result;
}


static INT32 DVBT2_change_PLP(struct nim_device *dev, UINT8 plp_index, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    UINT8 lock = 0;
    INT32 result = ERR_FAILUE;
    DMD_PARAMETER_t* param = (DMD_PARAMETER_t *)dev->priv;

    SHARP6158_LOG(param, "%s(plp_index=%d, %d, %d)\r\n", __FUNCTION__, plp_index, NeedToInitSystem,  NeedToConfigTuner);

    fe_mn_api_set_regval( param, DMD_PSEQOP1, 1);        //Disable watchdog in order to avoid MN88472 reset if wait locked for long time.

    do
    {
        if ( NeedToInitSystem || param->system != DMD_E_DVBT2)
        {
            param->t2_signal = 1;
            param->plp_num = 0;
            param->system = DMD_E_DVBT2;
            DMD_set_system( param );

            NeedToConfigTuner = TRUE;
        }

        if ( param->system != DMD_E_DVBT2 )
            break;  // return result;

        param->t2_signal = 1;
        param->plp_index = plp_index;

        DMD_channel_search_setting( param );
    //    DMD_set_PLP_no(param, plp_index);
    //    DMD_device_reset(priv);

        //select PLP No
         DMD_set_info( param , DMD_E_INFO_DVBT2_SELECTED_PLP , plp_index );
         DMD_wait(300);

        if ( NeedToConfigTuner )
        {
            result = config_tuner(dev, param->Frequency, param->bandwidth, &lock); //The tuner locks the frequency.
            if(result != SUCCESS)
            {
//                return result;
                break;
            }
        }

        if( DMD_scan( param ) != DMD_E_OK )  //DEM Locks the DVB-T2 signal.
        {
            result = ERR_FAILUE;
//                return result;
            break;
        }

        DMD_get_info(param, DMD_E_INFO_DVBT2_LOCK);
        if ( DMD_E_LOCKED == param->info[DMD_E_INFO_DVBT2_LOCK] )
        {
//            DMD_READ_REGISTERS(param);
            result = SUCCESS;
        }
        else
        {
            SHARP6158_LOG(param, "%s: fail to lock PLP.\r\n", __FUNCTION__);
        }
    }while(0);

    fe_mn_api_set_regval( param, DMD_PSEQOP1, 0); //Enable watchdog
    return result;
}

static INT32 try_to_lock_next_data_plp(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    INT32 result = ERR_FAILUE;
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;

    if ( DMD_get_next_data_PLP_info( param ) != DMD_E_OK )
    {
//        SHARP6158_LOG(param, "%s() error.\r\n", __FUNCTION__);
        return result;
    }
    result = DVBT2_change_PLP(dev, param->plp_index, FALSE, TRUE);

    return result;
}

static INT32 try_to_lock_DVBT2_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
//    UINT8 PLPIds[255];
//    UINT8 NumPLPs = 0;

    INT32 result = ERR_FAILUE;
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;

    SHARP6158_LOG(param, "%s(NeedToInitSystem=%d, NeedToConfigTuner=%d)\r\n", __FUNCTION__, NeedToInitSystem, NeedToConfigTuner);

    result = DVBT2_change_PLP(dev, 0, NeedToInitSystem, NeedToConfigTuner); //for lock signal before it can call DMD_get_dataPLPs().
//    result = DVBT2_change_PLP(dev, 0, TRUE, TRUE); //for lock signal before it can call DMD_get_dataPLPs().
    if (result != SUCCESS)
    {
        return result;
    }

//    DMD_get_dataPLPs( &param->data_plp_idx, &param->plp_num , param );
    if ( DMD_get_the_first_data_PLP_info( param ) != DMD_E_OK )
    {
        SHARP6158_LOG(param, "%s() error: DMD_get_the_first_data_PLP_info().\r\n", __FUNCTION__);
        return result;
    }

    if ( param->plp_index != 0 )  //Tune to the first data PLP if it is not be PLP 0. PLP 0 had been locked ahead already.
//        result = DVBT2_change_PLP(dev, param->plp_index, FALSE, FALSE);
        result = DVBT2_change_PLP(dev, param->plp_index, TRUE, TRUE);

    return result;
}

static INT32 nim_sharp6158_channel_change_smart(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
    //UINT8 lock = 0, Tuner_Retry = 0;
    //UINT8 WAIT_LOCK_TIME=1;
    //UINT8 try_time;
    INT32 result;

    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;
    //PCOFDM_TUNER_CONFIG_API config_info = &(param->tc);
    DMD_BANDWIDTH_t    bw;
    DMD_IF_FREQ_t    if_freq;
    BOOL change_plp_only, NeedToInitSystem = FALSE, NeedToConfigTuner = FALSE;

    UINT32 flgptn;

    if ((change_para->freq <= 40000) || (change_para->freq >= 900000))
    {
        return ERR_FAILUE;
    }

     switch(change_para->bandwidth)
    {
      case 6:
        bw = DMD_E_BW_6MHZ;
        break;

      case 7:
        bw = DMD_E_BW_7MHZ;
        break;

      case 8:
        bw = DMD_E_BW_8MHZ;
        break;

      default:
        return ERR_FAILUE;
    }

#if 0
     switch(bw)
    {
      case DMD_E_BW_6MHZ:
        if_freq =DMD_E_IF_4500KHZ;    //IF shall be 4MHz for DMD_E_BW_6MHZ.
        break;

      case DMD_E_BW_7MHZ:
        if_freq =DMD_E_IF_4500KHZ;
        break;

      case DMD_E_BW_8MHZ:
        if_freq =DMD_E_IF_5000KHZ;
        break;

      default:         
        return ERR_FAILUE;
    }
#endif
    if_freq = DMD_E_IF_5000KHZ;


    result = osal_flag_wait(&flgptn, param->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 30*1000); //OSAL_WAIT_FOREVER_TIME
    if(OSAL_E_OK != result)
    {
         return ERR_FAILUE;
    }

    SHARP6158_LOG(param, "%s: usage_type %d, freq %d, bandwidth %d, priority %d, t2_signal %d, plp_index %d, plp_id %d\r\n",__FUNCTION__,change_para->usage_type, change_para->freq,change_para->bandwidth, change_para->priority, change_para->t2_signal, change_para->plp_index, change_para->plp_id);

    do
    {
        param->autoscan_stop_flag = 0;
        param->do_not_wait_t2_signal_locked = ( change_para->usage_type == USAGE_TYPE_AERIALTUNE ? 1:0 );

        if ( need_to_config_tuner(dev, change_para->freq, bw, if_freq) )
        {
            if ( param->bandwidth != change_para->bandwidth \
              || param->if_freq != if_freq  )
              NeedToInitSystem = TRUE;

            param->Frequency = change_para->freq;
            param->bandwidth = change_para->bandwidth;
            param->bw = bw;
            param->if_freq = if_freq;

//            result = config_tuner(dev, param->Frequency, param->bandwidth, &lock); //The tuner locks the frequency.
//            if(result != SUCCESS)
//            {
//                break;
//            }
            NeedToConfigTuner = TRUE;
        }

        if ( need_to_lock_DVBT_signal(dev, change_para, NeedToConfigTuner) )
        {
            result = try_to_lock_DVBT_signal(dev, NeedToInitSystem, NeedToConfigTuner);
            if (result == SUCCESS)
            {
                break;
            }
        }
        if (param->autoscan_stop_flag)
        {
            break;
        }

        if ( need_to_lock_DVBT2_signal(dev, change_para, NeedToConfigTuner, &change_plp_only) )
        {
            if (change_plp_only)
            {
                param->do_not_wait_t2_signal_locked = 1;
                result = DVBT2_change_PLP(dev, change_para->plp_index, NeedToInitSystem, NeedToConfigTuner);
//                result = DVBT2_change_PLP(dev, change_para->plp_index, NeedToInitSystem, TRUE);
            }
            else
            {
                if (change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN)
                {
                    result = try_to_lock_next_data_plp(dev, NeedToInitSystem, NeedToConfigTuner);
                }
                else
                {
                    result = try_to_lock_DVBT2_signal(dev, NeedToInitSystem, NeedToConfigTuner);
                    change_para->plp_num = param->plp_num;
                }
                change_para->plp_index = param->plp_index;
                change_para->plp_id = param->plp_id;
                change_para->t2_system_id = param->t2_system_id;
            }
        }
    }while (0);
    change_para->t2_signal = param->t2_signal;

    osal_flag_set(param->flag_id,NIM_SCAN_END);
    return result;
}

#if 0
static INT32 nim_sharp6158_internal_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
    UINT8 lock = 0, Tuner_Retry = 0;
    UINT8 WAIT_LOCK_TIME=1;
    UINT8 try_time;
    INT32 result;

    UINT8 PLPIds[256];
    UINT8 NumPLPs = 0;

    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    PCOFDM_TUNER_CONFIG_API config_info = &(priv->tc);
    DMD_BANDWIDTH_t    bw;

    UINT32 flgptn;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 10*1000); //OSAL_WAIT_FOREVER_TIME
    if(OSAL_E_OK != result)
    {
         return ERR_FAILUE;
    }
//    osal_flag_clear(priv->flag_id, NIM_SCAN_END);

    if ((freq <= 40000) || (freq >= 900000))
    {
        return ERR_FAILUE;
    }

     switch(bandwidth)
    {
      case 6:
        bw = DMD_E_BW_6MHZ;
        break;

      case 7:
        bw = DMD_E_BW_7MHZ;
        break;

      case 8:
        bw = DMD_E_BW_8MHZ;
        break;

      default:
        return ERR_FAILUE;
    }


    SHARP6158_LOG(priv, "%s: freq %d, bandwidth %d,guard_interval %d, fft_mode %d,modulation %d,fec %d,usage_type %d, inverse %d, priority %d \r\n ",
                __FUNCTION__, freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);

//If same config data, and dem work in lock state, not config dem again.
//    DMD_get_info(priv, DMD_E_INFO_DVBT2_LOCK);
//    if ( DMD_E_LOCKED == priv->info[DMD_E_INFO_DVBT2_LOCK] && (freq == priv->Frequency) && (bw == priv->bw))
//    {
//        DMD_get_info(priv, DMD_E_INFO_DVBT2_ALL);
//
//        osal_flag_set(priv->flag_id,NIM_SCAN_END);
//        return SUCCESS;
//    }


    priv->Frequency = freq;
    priv->bw = bw;

#if 0
     switch(bw)
    {
      case DMD_E_BW_6MHZ:
        priv->if_freq =DMD_E_IF_4500KHZ;    //IF shall be 4MHz for DMD_E_BW_6MHZ.
        break;

      case DMD_E_BW_7MHZ:
        priv->if_freq =DMD_E_IF_4500KHZ;
        break;

      case DMD_E_BW_8MHZ:
        priv->if_freq =DMD_E_IF_5000KHZ;
        break;

      default:
        return ERR_FAILUE;
    }
#endif
    priv->if_freq =DMD_E_IF_5000KHZ;    //IF=5MHz if DMD_XTAL_FREQ_t=20.5MHz(DMD_E_XTAL_20500KHZ).


//Try to lock DVB-T signal.
    do
    {
        priv->system = DMD_E_DVBT;
        DMD_set_system( priv );

        DMD_channel_search_setting( priv );

        result = config_tuner(dev, freq, bandwidth, &lock); //The tuner locks the frequency.
        if(result != SUCCESS)
        {
            osal_flag_set(priv->flag_id,NIM_SCAN_END);
            return result;
//            break;
        }

        if( DMD_scan( priv ) != DMD_E_OK )  //DEM Locks the DVB-T signal.
        {
            result = ERR_FAILUE;
            break;
        }

        try_time = 0;
        result = ERR_FAILUE;
        while(1)
        {

            DMD_get_info(priv, DMD_E_INFO_DVBT_LOCK);
            if ( DMD_E_LOCKED == priv->info[DMD_E_INFO_DVBT_LOCK] )
            {
                result = SUCCESS;
                break;
            }

            if(try_time >= WAIT_LOCK_TIME*5)//actually, when TPS lock, COFDM is very easy lock. So we should length TPS time.
            {
                SHARP6158_LOG(priv, "TPS lock, COFDM unlock!\r\n");
                result = ERR_FAILUE;
                break;
            }

            osal_task_sleep(10);
            try_time++;
        }
    }while(0);
    if (result == SUCCESS)
    {
        osal_flag_set(priv->flag_id,NIM_SCAN_END);
        return result;
    }


//Try to lock DVB-T2 signal.
    priv->system = DMD_E_DVBT2;
    DMD_set_system( priv );

    DMD_channel_search_setting( priv );
    DMD_set_PLP_no(priv, 0);
//    DMD_device_reset(priv);


    //DMD_tune()
    result = config_tuner(dev, freq, bandwidth, &lock); //The tuner locks the frequency.
    if(result != SUCCESS)
    {
        osal_flag_set(priv->flag_id,NIM_SCAN_END);
        return result;
    }

    if( DMD_scan( priv ) != DMD_E_OK )  //DEM Locks the DVB-T2 signal.
    {
        osal_flag_set(priv->flag_id,NIM_SCAN_END);
        return result;
    }

    DMD_get_dataPLPs( &PLPIds, &NumPLPs , priv );
    if ( NumPLPs == 0 )
    {
        osal_flag_set(priv->flag_id,NIM_SCAN_END);
        return result;
    }
    DMD_set_PLP_no(priv, PLPIds[0]);
//    DMD_device_reset(priv);
    if( DMD_scan( priv ) != DMD_E_OK )  //DEM Locks the DVB-T2 signal.
    {
        osal_flag_set(priv->flag_id,NIM_SCAN_END);
        return result;
    }

    try_time = 0;
    result = ERR_FAILUE;
    while(1)
    {

        DMD_get_info(priv, DMD_E_INFO_DVBT2_LOCK);
        if ( DMD_E_LOCKED == priv->info[DMD_E_INFO_DVBT2_LOCK] )
        {
//            DMD_get_info(priv, DMD_E_INFO_DVBT2_ALL);
//            DMD_get_dataPLPs( &PLPIds, &NumPLPs , priv );

            SHARP6158_LOG(priv, "SHARP6158_NIM_lock = 0x%2x\r\n",data[0]);
            result = SUCCESS;
            break;
        }

        if(try_time >= WAIT_LOCK_TIME*5)//actually, when TPS lock, COFDM is very easy lock. So we should length TPS time.
        {
            SHARP6158_LOG(priv, "TPS lock, COFDM unlock!\r\n");
            result = ERR_FAILUE;
            break;
        }

        osal_task_sleep(10);
        try_time++;
    }

    osal_flag_set(priv->flag_id,NIM_SCAN_END);
    return result;
}
#endif

//static INT32 nim_sharp6158_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)
//{
//    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
//    *freq_offset = 0;
//    return SUCCESS;
//}

//static INT32 nim_sharp6158_get_hier_mode(struct nim_device *dev, UINT8 *hier)
//{
//    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
//    *hier = priv->lock_info.Hierarchy;
//    return SUCCESS;
//}

/*****************************************************************************
* INT32 nim_sharp6158_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 nim_sharp6158_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    *Inv = priv->info[DMD_E_INFO_DVBT2_FEF_INTERVAL];
    return SUCCESS;
}


static int Calc_T_SSI(struct nim_device *dev)
{
    DMD_PARAMETER_t *param = (DMD_PARAMETER_t *)dev->priv;

    int  Prec=0,  Prel=0,  Pref=0;
    int ssi=0;

    INT32 rxPwrPtr = 0 ;
    if ( nim_sharp6158_ioctl(dev, NIM_TUNER_GET_RF_POWER_LEVEL, (UINT32)&rxPwrPtr) == ERR_FAILUE )
    {
        SHARP6158_LOG(param, "error: Tuner does not support command 'NIM_TUNER_GET_RF_POWER_LEVEL'.\r\n");
        return 0;
    }
//    SHARP6158_LOG(param, "%s(): NIM_TUNER_GET_RF_POWER_LEVEL =%d \r\n", __FUNCTION__, (int)rxPwrPtr);

    Prec=(int)rxPwrPtr/100;
    Pref=Get_RF_T_REF_PWR(param);
    Prel= Prec - Pref;
     //libc_printf("\n%s:Prec=%d , Pref= %d, Prel= %d\n",__FUNCTION__,Prec,Pref,Prel);

    /*
    SSI = 0                                    if  Prel < -15dB
    SSI = (2/3) * ( Prel + 15)           if -15 dB <=  Prel < 0dB
    SSI = 4 *  Prel + 10                   if 0 dB <=   Prel < 20 dB
    SSI = (2/3) * ( Prel - 20) + 90    if 20 dB <=   Prel < 35 dB
    SSI = 100                                if  Prel >= 35 dB
    where
     Prel =  Prec -   Pref
     Prec is referred to signal level expressed in [dBm] at receiver RF signal input.
      Pref is reference signal level value expressed in [dBm] specified in Nordig Unified V2.3 - Table 3.6 for DVB-T and in Table 3.7 for DVB-T2.
    */
    if( Prel<-15)
    {
        ssi=0;
    }else if( Prel<0){
        ssi=(int)(( Prel+15)*2/3);
    }else if( Prel<20){
        ssi=(int)( Prel*4+10);
    }else if( Prel<35){
        ssi=(int)(( Prel-20)*2/3+90);
    }else
        ssi=100;

/*dead_code
    if(ssi>100)
        ssi=100;
    else if(ssi<0)
        ssi=0;
*/
    //libc_printf("%s:ssi=%d\n",__FUNCTION__,ssi);
    return ssi;
}

static int Calc_T2_SSI(struct nim_device *dev)
{
    DMD_PARAMETER_t *param = (DMD_PARAMETER_t *)dev->priv;

    int   Prec=0, Prel=0,  Pref=0;
    int ssi=0;
    static int pre_ssi=0;

    UINT8 rd = 0;
    INT32 rxPwrPtr;
    if ( nim_sharp6158_ioctl(dev, NIM_TUNER_GET_RF_POWER_LEVEL, (UINT32)&rxPwrPtr) == ERR_FAILUE )
    {
        SHARP6158_LOG(param, "error: Tuner does not support command 'NIM_TUNER_GET_RF_POWER_LEVEL'.\r\n");
        return 0;
    }
//    SHARP6158_LOG(param, "%s(): NIM_TUNER_GET_RF_POWER_LEVEL =%d \r\n", __FUNCTION__, (int)rxPwrPtr);

    Prec=(int)rxPwrPtr/100;

    //Get FEF status. S2 = xxx1, mixed frame, means T2 super frame including FEF part.
    DMD_I2C_Read(param, DMD_BANK_T2_ , DMD_P1FLG, &rd ); //troy, 20130131, sometimes SSI return 0 but video play OK.
    if (( Prec <= -98 )&&( rd & 0x01))
    {
        //libc_printf("FEF detected rd=%x.\n",rd);
        return pre_ssi;
    }

    Pref = Get_RF_T2_REF_PWR(param);
    Prel=  Prec- Pref;
     //libc_printf("\n%s:Prec=%d , Pref= %d, Prel= %d\n",__FUNCTION__,Prec,Pref,Prel);
    /*
    SSI = 0                                    if  Prel < -15dB
    SSI = (2/3) * ( Prel + 15)           if -15 dB <= Prel < 0dB
    SSI = 4 * Prel + 10                   if 0 dB <=  Prel < 20 dB
    SSI = (2/3) * (Prel - 20) + 90    if 20 dB <=  Prel < 35 dB
    SSI = 100                                if Prel >= 35 dB
    where
    Prel =   Prec -  Pref
      Prec is referred to signal level expressed in [dBm] at receiver RF signal input.
     Pref is reference signal level value expressed in [dBm] specified in Nordig Unified V2.3 - Table 3.6 for DVB-T and in Table 3.7 for DVB-T2.
    */

    if( Prel<-15)
    {
        ssi=0;
    }else if( Prel<0){
        ssi=(int)(( Prel+15)*2/3);
    }else if( Prel<20){
        ssi=(int)( Prel*4+10);
    }else if( Prel<35){
        ssi=(int)(( Prel-20)*2/3+90);
    }else
        ssi=100;

    /*dead_code
    if(ssi>100)
        ssi=100;
    else if(ssi<0)
        ssi=0;
    */
    pre_ssi=ssi;

    //libc_printf("%s:ssi=%d\n",__FUNCTION__,ssi);
    return ssi;
}

static UINT8 mn88472_modulation_map_to_ali_modulation(DMD_SYSTEM_t system, UINT8 mn88472_modulation)
{
    //T_NODE:    UINT32 modulation : 8;
    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM //T2: (64+1):256 QAM, //0xFF:unknown

    if (system == DMD_E_DVBT)
    {
        switch (mn88472_modulation)
        {
            case DMD_E_DVBT_CONST_QPSK:
                return 4;
            case DMD_E_DVBT_CONST_16QAM:
                return 16;
            case DMD_E_DVBT_CONST_64QAM:
                return 64;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (mn88472_modulation)
        {
            case DMD_E_DVBT2_PLP_MOD_QPSK:
                return 4;
            case DMD_E_DVBT2_PLP_MOD_16QAM:
                return 16;
            case DMD_E_DVBT2_PLP_MOD_64QAM:
                return 64;
            case DMD_E_DVBT2_PLP_MOD_256QAM:
                return (64+1);
            default:
                return 0xFF;   //unknown.
        }
    }
}

static UINT8 mn88472_FEC_map_to_ali_FEC(DMD_SYSTEM_t system, UINT8 mn88472_FEC)
{
    //T_NODE:    UINT16 FEC_inner            : 4;
    //T: 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8  //T2: 5:3/5, 6:4/5, //0xF:unknown

    if (system == DMD_E_DVBT)
    {
        switch (mn88472_FEC)
        {
            case DMD_E_DVBT_CR_1_2:
                return 0;
            case DMD_E_DVBT_CR_2_3:
                return 1;
            case DMD_E_DVBT_CR_3_4:
                return 2;
            case DMD_E_DVBT_CR_5_6:
                return 3;
            case DMD_E_DVBT_CR_7_8:
                return 4;
            default:
                return 0xF;   //unknown.
        }
    }
    else
    {
        switch (mn88472_FEC)
        {
            case DMD_E_DVBT2_CR_1_2:
                return 0;
            case DMD_E_DVBT2_CR_2_3:
                return 1;
            case DMD_E_DVBT2_CR_3_4:
                return 2;
            case DMD_E_DVBT2_CR_5_6:
                return 3;
            case DMD_E_DVBT2_CR_3_5:
                return 5;
            case DMD_E_DVBT2_CR_4_5:
                return 6;
            default:
                return 0xF;   //unknown.
        }
    }
}

static UINT8 mn88472_gi_map_to_ali_gi(DMD_SYSTEM_t system, UINT8 mn88472_gi)
{
    //T_NODE:    UINT32 guard_interval : 8;
    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32  //T2: 128:1/128, (19+128):19/128, 19:19/256, //0xFF:unknown
    if (system == DMD_E_DVBT)
    {
        switch (mn88472_gi)
        {
            case DMD_E_DVBT_GI_1_32:
                return 32;
            case DMD_E_DVBT_GI_1_16:
                return 16;
            case DMD_E_DVBT_GI_1_8:
                return 8;
            case DMD_E_DVBT_GI_1_4:
                return 4;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (mn88472_gi)
        {
            case DMD_E_DVBT2_GI_1_32:
                return 32;
            case DMD_E_DVBT2_GI_1_16:
                return 16;
            case DMD_E_DVBT2_GI_1_8:
                return 8;
            case DMD_E_DVBT2_GI_1_4:
                return 4;
            case DMD_E_DVBT2_GI_1_128:
                return 128;
            case DMD_E_DVBT2_GI_19_128:
                return (19+128);
            case DMD_E_DVBT2_GI_19_256:
                return 19;
            default:
                return 0xFF;   //unknown.
        }
    }
}

static UINT8 mn88472_fft_mode_map_to_ali_fft_mode(DMD_SYSTEM_t system, UINT8 mn88472_fft_mode)
{
    //T_NODE:    UINT32 FFT : 8;
    //2:2k, 8:8k //T2: 4:4k, 16:16k, 32:32k, //0xFF:unknown

    if (system == DMD_E_DVBT)
    {
        switch (mn88472_fft_mode)
        {
            case DMD_E_DVBT_MODE_2K:
                return 2;
            case DMD_E_DVBT_MODE_4K:
                return 4;
            case DMD_E_DVBT_MODE_8K:
                return 8;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (mn88472_fft_mode)
        {
            case DMD_E_DVBT2_MODE_2K:
                return 2;
            case DMD_E_DVBT2_MODE_4K:
                return 4;
            case DMD_E_DVBT2_MODE_8K:
                return 8;
            case DMD_E_DVBT2_MODE_1K:
                return 1;
            case DMD_E_DVBT2_MODE_16K:
                return 16;
            case DMD_E_DVBT2_MODE_32K:
                return 32;
            default:
                return 0xFF;   //unknown.
        }
    }
}

/*****************************************************************************
* INT32 nim_sharp6158_get_modulation(struct nim_device *dev, UINT8 *modulation)
* Description: Read sharp6158 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* modulation
*
* Return Value: INT32
    //T_NODE:    UINT32 modulation : 8;
    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM //T2: (64+1):256 QAM, //0xFF:unknown
*****************************************************************************/
static INT32 nim_sharp6158_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 val;
    UINT32 flgptn;
    OSAL_ER result;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        if (priv->t2_signal) //DVB-T2
        {
            DMD_get_info(priv,DMD_E_INFO_DVBT2_DAT_PLP_MOD);
            val = priv->info[DMD_E_INFO_DVBT2_DAT_PLP_MOD];
        }
        else //for DVB-T mode
        {
            DMD_get_info(priv,DMD_E_INFO_DVBT_CONSTELLATION);
            val = priv->info[DMD_E_INFO_DVBT_CONSTELLATION];
        }
        *modulation    = mn88472_modulation_map_to_ali_modulation(priv->system, val);
    }
    else
    {
        *modulation = 0xFF;   //unknown.
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_sharp6158_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
* Description: Read sharp6158 fft_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* fft_mode
*
* Return Value: INT32
    //T_NODE:    UINT32 FFT : 8;
    //2:2k, 8:8k //T2: 4:4k, 16:16k, 32:32k, //0xFF:unknown
*****************************************************************************/

static INT32 nim_sharp6158_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 val;
    UINT32 flgptn;
    OSAL_ER result;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        if (priv->t2_signal) //DVB-T2
        {
            DMD_get_info(priv,DMD_E_INFO_DVBT2_MODE);
            val = priv->info[DMD_E_INFO_DVBT2_MODE];
        }
        else //for DVB-T mode
        {
            DMD_get_info(priv,DMD_E_INFO_DVBT_MODE);
            val = priv->info[DMD_E_INFO_DVBT_MODE];
        }
        *fft_mode    = mn88472_fft_mode_map_to_ali_fft_mode(priv->system, val);
    }
    else
    {
        *fft_mode = 0xFF;   //unknown.
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_sharp6158_get_GI(struct nim_device *dev, UINT8 *guard_interval)
* Description: Read sharp6158 guard interval
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* guard_interval
*
* Return Value: INT32
    //T_NODE:    UINT32 guard_interval : 8;
    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32  //T2: 128:1/128, (19+128):19/128, 19:19/256, //0xFF:unknown
*****************************************************************************/
static INT32 nim_sharp6158_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 val;
    UINT32 flgptn;
    OSAL_ER result;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        if (priv->t2_signal) //DVB-T2
        {
               DMD_get_info(priv, DMD_E_INFO_DVBT2_GI);
              val = priv->info[DMD_E_INFO_DVBT2_GI];
        }
        else //for DVB-T mode
        {
              DMD_get_info(priv, DMD_E_INFO_DVBT_GI);
              val = priv->info[DMD_E_INFO_DVBT_GI];
        }
        *guard_interval    = mn88472_gi_map_to_ali_gi(priv->system, val);
    }
    else
    {
        *guard_interval = 0xFF;   //unknown.
    }
    return SUCCESS;
}

static INT32 nim_sharp6158_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    DMD_u32_t err = 0;
    DMD_u32_t sum = 0;
    UINT32 flgptn = 0;
    OSAL_ER result;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {

        DMD_BER(priv,priv->system,&err, &sum);
        if((err<sum)&&(sum!=0))
        {
            *vbber =  100 - (err*100)/sum;
        }
        else
        {
            *vbber =0;
        }
    }
    else
    {
        *vbber = 0;
    }

    return SUCCESS;
}

static INT32 nim_sharp6158_get_PER(struct nim_device *dev, UINT8 *per)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    DMD_u32_t err = 0;
    DMD_u32_t sum = 0;
    OSAL_ER result;
    UINT32 flgptn = 0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        DMD_PER(priv, priv->system, &err, &sum);
        if (sum != 0)
            *per =  100 - (err*100)/sum;
        else
            *per =  0;
    }
    else
    {
        *per =  0;
    }
    return SUCCESS;
}

static INT32 nim_sharp6158_get_AGC(struct nim_device *dev, UINT8 *data)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    OSAL_ER result;
    DMD_u32_t  agc, max_agc;
    UINT32 flgptn;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        agc = DMD_AGC(priv);
        max_agc = 0xff*4+3;   //10 bit.
        *data = 100-(agc*100/max_agc);
    }
    else
    {
        *data = 0;
    }
    return SUCCESS;
}

static INT32 nim_sharp6158_get_CNR(struct nim_device *dev, UINT8 *data)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    OSAL_ER result;
    UINT32  flgptn;
    DMD_u32_t cnr_i=0, cnr_d=0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        DMD_CNR(priv, priv->system, &cnr_i , &cnr_d );
        *data = cnr_i;
    }
    else
    {
        *data = 0;
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_sharp6158_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *snr
*
* Return Value: * snr

*****************************************************************************/
static INT32 nim_sharp6158_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    OSAL_ER result;
    UINT32  flgptn;
    //UINT32 per_tmp;
    //UINT32 ber_tmp;
    //UINT8 data[2];
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        *snr = 90;
    }
    else
    {
        *snr = 0;
    }

    return SUCCESS;
}

static INT32 nim_sharp6158_get_SQI(struct nim_device *dev, UINT8 *snr)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 flgptn;
    OSAL_ER result;
       result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
         if (priv->t2_signal) //DVB-T2
         {
            DMD_get_info(priv,DMD_E_INFO_DVBT2_SQI);
            *snr = priv->info[DMD_E_INFO_DVBT2_SQI];
         }
         else
         {
             DMD_get_info(priv,DMD_E_INFO_DVBT_SQI);
            *snr = priv->info[DMD_E_INFO_DVBT_SQI];
         }
    }
    else
    {
        *snr =0;
    }
    //libc_printf("[%s]:SQI=%d\n",__FUNCTION__,*snr);
        return SUCCESS;
}

//UINT8 xx=0;
//UINT8 temp_agc[10];
static INT32 nim_sharp6158_get_SSI(struct nim_device *dev, UINT8 *ssi)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    UINT32 flgptn;
    //UINT8 agc_min,agc_max;
    //UINT8 i=0;
    //UINT16 sum=0;
    OSAL_ER result;
       result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
       {
         if (priv->t2_signal) //DVB-T2
         {
            *ssi =(UINT8)(Calc_T2_SSI(dev));
         }
         else
         {
             *ssi=(UINT8)(Calc_T_SSI(dev));
         }
    }
    else
    {
        *ssi =0;
    }
    //libc_printf("[%s]:SSI=%d\n",__FUNCTION__,*ssi);

    /*
    if(xx==10)
    {
        xx=0;
    }
    temp_agc[xx]=*ssi;
    libc_printf("temp_agc[%d]=%d\n",xx,temp_agc[xx]);
    if(xx==9)
    {
        agc_min = temp_agc[0];
        agc_max = temp_agc[0];
        for(i;i<10;i++)
        {
            libc_printf("--temp_agc[%d]--=%d\n",i,temp_agc[i]);
            if(temp_agc[i]>agc_max)
            {
                agc_max = temp_agc[i];
            }
            if(temp_agc[i]<agc_min)
            {
                agc_min = temp_agc[i];
            }
            sum+=temp_agc[i];
            libc_printf("sum=%d,agc_min=%d,agc_max=%d\n",sum,agc_min,agc_max);
        }
        *ssi=(sum-agc_min-agc_max)/8;
        libc_printf("*ssi=%d\n",*ssi);
    }
    xx+=1;*/

        return SUCCESS;
}

static INT32 nim_sharp6158_get_FEC(struct nim_device *dev, UINT8* FEC)
{
    DMD_PARAMETER_t * param = (DMD_PARAMETER_t *)dev->priv;
    UINT32 val;
    UINT32 flgptn;
    OSAL_ER result;
    UINT8 FEC_type;
    result = osal_flag_wait(&flgptn, param->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        if(param->t2_signal) //DVB-T2
        {
            DMD_get_info(param, DMD_E_INFO_DVBT2_DAT_PLP_COD);
            val = param->info[DMD_E_INFO_DVBT2_DAT_PLP_COD];

            DMD_get_info(param, DMD_E_INFO_DVBT2_DAT_PLP_FEC_TYPE);
            FEC_type = param->info[DMD_E_INFO_DVBT2_DAT_PLP_FEC_TYPE];   //DMD_DVBT2_FEC_TYPE_t: LDPC_16K, LDPC_64K
        }
        else //for DVB-T mode
        {
            DMD_DVBT_HIERARCHY_t   hierarchy_exist;
            DMD_DVBT_HIER_SEL_t hierarchy_selection;
            DMD_u32_t DMD_id;

            // Check HP stream exist or not
            DMD_get_info(param, DMD_E_INFO_DVBT_HIERARCHY);
            hierarchy_exist= param->info[DMD_E_INFO_DVBT_HIERARCHY];

            DMD_get_info(param, DMD_E_INFO_DVBT_HIERARCHY_SELECT);
            hierarchy_selection = param->info[DMD_E_INFO_DVBT_HIERARCHY_SELECT];

            if (hierarchy_exist != DMD_E_DVBT_HIERARCHY_NO)
            {
                if (hierarchy_selection == DMD_E_DVBT_HIER_SEL_HP)//Hierarchy, Alpha 1,2,4 & Customer chooses to decode HP data stream
                {
                    DMD_id = DMD_E_INFO_DVBT_HP_CODERATE;
                }
                else //Non-hierarchy OR Customer choose LP when HP&LP both transmitted.
                {
                    DMD_id = DMD_E_INFO_DVBT_LP_CODERATE;
                }
            }
            else //no hierarchy
            {
              //if not open HIERARCHY, DVB-T code rate gets from HP area.
                 DMD_id = DMD_E_INFO_DVBT_HP_CODERATE;
            }

            DMD_get_info(param, DMD_id);   //DMD_E_INFO_DVBT_LP_CODERATE
            val = param->info[DMD_id];
        }
        *FEC = mn88472_FEC_map_to_ali_FEC(param->system, val);
    }
    else
    {
        *FEC = 0xF; //unknown
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_sharp6158_get_freq(struct nim_device *dev, UINT32 *freq)
* Read M3327 frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *freq         :
*
* Return Value: void
*****************************************************************************/
static INT32 nim_sharp6158_get_freq(struct nim_device *dev, UINT32 *freq)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    *freq = priv->Frequency;
    return SUCCESS;
}

static INT32 nim_sharp6158_get_lock(struct nim_device *dev, UINT8 *lock)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    //UINT8 data=0;
    OSAL_ER result;
    UINT32 flgptn;

    //int i;
    //static DMD_PARAMETER_t    pp; //for debug.


    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        DMD_get_info(priv, DMD_E_INFO_LOCK);

#if 0
        DMD_get_info( priv , DMD_E_INFO_LOCK );
        if( priv->info[DMD_E_INFO_LOCK] == DMD_E_LOCKED )
        {
            DMD_get_info( priv , DMD_E_INFO_ALL );
            for(i=DMD_E_INFO_DVBT2_MODE;i<priv->info[DMD_E_INFO_ALL];i++)
            {
                if( pp.info[i] != priv->info[i] )
                {
                    libc_printf( "%25s :",DMD_info_title( priv->system , i ) );
                    libc_printf( "%15s :",DMD_info_value( priv->system , i , priv->info[i] ) );
                    libc_printf(" %d\r\n" , priv->info[i]);
                }
            }
            for(i=0;i<priv->info[DMD_E_INFO_ALL];i++)
            {
                pp.info[i] = priv->info[i];
            }
        }
#endif


        if ( DMD_E_LOCKED == (priv->info[DMD_E_INFO_LOCK]) )
        {
            *lock = 1;
        }
        else
        {
            *lock = 0;
        }

        //put light the panel lock here. if *lock=ff, should not light on the led.
        nim_sharp6158_switch_lock_led(dev, (*lock)?TRUE:FALSE);
    }
    else
    {
        *lock=0x0;
    }

    return SUCCESS;
}


#if 0
static INT32 nim_sharp6158_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    INT32 chsearch;

    chsearch=nim_sharp6158_internal_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse,priority);

    return chsearch;
}

/*****************************************************************************
* INT32 nim_sharp6158_channel_search(struct nim_device *dev, UINT32 freq);

* Description: sharp6158 channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_sharp6158_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT16 freq_offset, UINT8 priority)
{
    return nim_sharp6158_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);
}
#endif

static INT32 nim_sharp6158_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
//    nim_ad_gain_table_t *agt, *d_agt;
//    nim_rf_if_ad_val_t *p_rf_if_ad_val;
//    nim_rec_performance_t * p_nim_rec_performance;
    //INT32 freq_offset;
    //UINT8 data[2];
    INT32 ret_val = SUCCESS;
//    UINT16 if_ad_val, rf_ad_val;

    if(cmd & NIM_TUNER_COMMAND)
    {
        if(NIM_TUNER_CHECK == cmd)
            return SUCCESS;

        if(priv->tc.nim_tuner_command != NULL)
        {
//            set_bypass_mode(dev, TRUE);
            ret_val = priv->tc.nim_tuner_command(priv->tuner_id, cmd, param);
//            set_bypass_mode(dev, FALSE);
        }
        else
            ret_val = ERR_FAILUE;
        return ret_val;
    }

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            priv->autoscan_stop_flag = param;
            break;

        case NIM_DRIVER_DISABLE_DEBUG_LOG:
            param = (UINT32)NULL;
        case NIM_DRIVER_ENABLE_DEBUG_LOG:
            if(NULL == priv->output_buffer)
            {
                priv->output_buffer = (char *)MALLOC(LOG_STRING_BUFFER_SIZE);
                ASSERT(priv->output_buffer != NULL);
            }
            priv->fn_output_string = (LOG_STRING_FUNCTION)param;
            break;

//        case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
//            p_nim_rec_performance = (nim_rec_performance_t *)param;
//            nim_sharp6158_get_lock(dev, &(p_nim_rec_performance->lock));
//
//            if(p_nim_rec_performance->lock == 1)
//            {
//                if(priv->rec_ber_cnt != priv->per_tot_cnt)
//                {
//                    priv->rec_ber_cnt = priv->per_tot_cnt;
//                    p_nim_rec_performance->ber = priv->snr_ber;
//                    p_nim_rec_performance->per = priv->snr_per;
//                    p_nim_rec_performance->valid = TRUE;
//                }
//                else
//                {
//                    p_nim_rec_performance->valid = FALSE;
//                }
//            }
//            break;

        case NIM_DRIVER_DISABLED:
            break;
        case NIM_DRIVER_GET_AGC:
              //return nim_sharp6158_get_AGC(dev, (UINT8 *)param);
                return nim_sharp6158_get_SSI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_BER:
            return nim_sharp6158_get_BER(dev, (UINT32 *)param);
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return nim_sharp6158_get_GI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_FFT_MODE:
            return nim_sharp6158_get_fftmode(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_MODULATION:
            return nim_sharp6158_get_modulation(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_SPECTRUM_INV:
            break;
        default:
            ret_val = ERR_FAILUE;
            break;
    }

    return ret_val;
}


static INT32 nim_sharp6158_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
    struct NIM_CHANNEL_CHANGE *change_para;
//    struct NIM_CHANNEL_SEARCH *search_para;
    switch (cmd)
    {
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            return nim_sharp6158_channel_change_smart(dev, change_para);
            break;
/*        case NIM_DRIVER_CHANNEL_SEARCH:
            search_para = (struct NIM_CHANNEL_SEARCH *)param_list;
            return nim_sharp6158_channel_search(dev, search_para->freq, search_para->bandwidth, \
                search_para->guard_interval, search_para->fft_mode, search_para->modulation, \
                search_para->fec, search_para->usage_type, search_para->inverse, \
                search_para->freq_offset, search_para->priority);
            break;*/
        case NIM_DRIVER_GET_BER_VALUE:
            return nim_sharp6158_get_BER_PER_log(dev);
            break;
        default:
            break;
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_sharp6158_close(struct nim_device *dev)
* Description: sharp6158 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_sharp6158_close(struct nim_device *dev)
{
    //UINT8 data;
    //DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;

    // tuner power off
    nim_sharp6158_ioctl(dev, NIM_TUNER_POWER_CONTROL, TRUE);

    nim_sharp6158_switch_lock_led(dev, FALSE);
    return SUCCESS;
}

__ATTRIBUTE_REUSE_
static INT32 nim_sharp6158_open(struct nim_device *dev)
{
    DMD_PARAMETER_t * priv = (DMD_PARAMETER_t *)dev->priv;
    //struct COFDM_TUNER_CONFIG_API * config_info = &(priv->tc);
    //UINT8 data;
    //UINT8 data_arr[4];
    //UINT8 adc_fmt=0;

    DMD_init(priv);
    DMD_set_system( priv );

    // tuner power on
    nim_sharp6158_ioctl(dev, NIM_TUNER_POWER_CONTROL, FALSE);

    #ifdef DEBUG_USBPRINTF
    nim_sharp6158_ioctl(dev,NIM_DRIVER_ENABLE_DEBUG_LOG,usb_printf);
    #endif

    osal_flag_set(priv->flag_id, NIM_SCAN_END);

#if 0
    OSAL_T_CTSK t_ctsk;
    OSAL_ID thread_id;

    t_ctsk.stksz = 0x1000;
    t_ctsk.quantum = 5;
    t_ctsk.task = nim_sharp6158_main_thread;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.para1 = (UINT32)dev;
    thread_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == thread_id)
    {
        ASSERT(0);
    }
#endif

    return SUCCESS;
}

static ID create_i2c_mutex(UINT32 i2c_type_id)
{
    struct nim_device *dev;
    DMD_PARAMETER_t * priv;
    UINT16 dev_id = 0;
    UINT16 mutex_id = OSAL_INVALID_ID;

    while(1)
    {
        dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, dev_id++);
        if(NULL == dev)
            break;
        priv = (DMD_PARAMETER_t *)dev->priv;
//        if((NIM_COFDM_ONLY_MODE == priv->tc.work_mode) && (i2c_type_id == priv->tc.ext_dm_config.i2c_type_id))
//        {
//            mutex_id = priv->i2c_mutex_id;
//            break;
//        }
    }
//    if(OSAL_INVALID_ID == mutex_id)
//        mutex_id = osal_mutex_create();

    mutex_id = osal_mutex_create();
    return mutex_id;
}

/*****************************************************************************
* INT32  nim_m31_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
* Description: sharp6158 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
__ATTRIBUTE_REUSE_
INT32 nim_sharp6158_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
//struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner, UINT8 mode)
{
    struct nim_device *dev;
    DMD_PARAMETER_t *priv;
    DEM_WRITE_READ_TUNER ThroughMode;

//    if(m_dev_num >= MAX_TUNER_SUPPORT_NUM)
//        return ERR_FAILUE;

    if(NULL == pConfig)
    {
        //SHARP6158_DEBUG("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if(NULL == dev)
    {
        //SHARP6158_DEBUG("Error: Alloc nim device error!\r\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv = (DMD_PARAMETER_t*)MALLOC(sizeof(DMD_PARAMETER_t));
    if(NULL == priv)
    {
        dev_free(dev);
        //SHARP6158_DEBUG("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    MEMSET((UINT8*)priv, 0, sizeof(DMD_PARAMETER_t));

    MEMCPY((UINT8*)&(priv->tc), (UINT8*)pConfig, sizeof(struct COFDM_TUNER_CONFIG_API));
    priv->system = DMD_E_DVBT2;
    priv->bw = DMD_E_BW_8MHZ;
    priv->if_freq = DMD_E_IF_5000KHZ;
    if (NIM_COFDM_TS_SSI == priv->tc.ts_mode)
        priv->ts_out = DMD_E_TSOUT_SERIAL_VARIABLE_CLOCK;
    else
        priv->ts_out = DMD_E_TSOUT_PARALLEL_VARIABLE_CLOCK;


    dev->priv = (void*)priv;

//    priv->i2c_device_id = priv->tc.ext_dm_config.i2c_type_id;
//    dev->base_addr = priv->tc.ext_dm_config.i2c_base_addr;

    dev->init = NULL;
    dev->open = nim_sharp6158_open;
    dev->stop = nim_sharp6158_close;
    dev->do_ioctl = nim_sharp6158_ioctl;
    dev->do_ioctl_ext = nim_sharp6158_ioctl_ext;

    dev->get_lock = nim_sharp6158_get_lock;
    dev->get_freq = nim_sharp6158_get_freq;
    dev->get_fec = nim_sharp6158_get_FEC;
    dev->get_snr = nim_sharp6158_get_SQI;//nim_sharp6158_get_PER

    dev->disable = NULL;    //nim_sharp6158_disable;
    dev->get_agc = nim_sharp6158_get_SSI;//nim_sharp6158_get_AGC;
    dev->get_ber = nim_sharp6158_get_BER;

    dev->channel_change = NULL; //nim_sharp6158_channel_change
    dev->channel_search = NULL; //nim_sharp6158_channel_search

    dev->get_guard_interval = nim_sharp6158_get_GI;
    dev->get_fftmode = nim_sharp6158_get_fftmode;
    dev->get_modulation =nim_sharp6158_get_modulation;
    dev->get_spectrum_inv = NULL; //nim_sharp6158_get_specinv

    dev->get_hier= NULL;
    dev->get_freq_offset = NULL;    //nim_sharp6158_get_freq_offset

    priv->flag_id = OSAL_INVALID_ID;
    priv->i2c_mutex_id = OSAL_INVALID_ID;

    priv->flag_id = osal_flag_create(NIM_FLAG_ENABLE);
    if (priv->flag_id==OSAL_INVALID_ID)
    {
        SHARP6158_LOG(priv, "%s: no more flag\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

    priv->i2c_mutex_id = create_i2c_mutex(priv->tc.ext_dm_config.i2c_type_id);
    if(priv->i2c_mutex_id == OSAL_INVALID_ID)
    {
        SHARP6158_LOG(priv, "%s: no more mutex\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

    /* Add this device to queue */
    if(dev_register(dev) != SUCCESS)
    {
        //SHARP6158_DEBUG("Error: Register nim device error!\r\n");
        FREE(priv);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if(priv->tc.nim_tuner_init != NULL)
    {
        if(priv->tc.nim_tuner_init(&(priv->tuner_id), &(priv->tc.tuner_config)) != SUCCESS)
        {
            //SHARP6158_DEBUG("Error: Init Tuner Failure!\r\n");
            return ERR_NO_DEV;
        }

        ThroughMode.nim_dev_priv = dev->priv;
        ThroughMode.dem_write_read_tuner = (void *)DMD_TCB_WriteRead;
        nim_sharp6158_ioctl(dev, NIM_TUNER_SET_THROUGH_MODE, (UINT32)&ThroughMode);
    }

    return SUCCESS;
}





