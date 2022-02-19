/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    This file contains m3327 basic function in LLD.
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    4.14.2005        Sam Chen      Ver 0.1       Create file.
*
*****************************************************************************/
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
#include "nim_mt353.h"
#if(SYS_DEM_MODULE==MT353 )

#define NIM_PRINTF(...)
#define NIM_MT353_PRINTF(...)

#define CE6353_DEBUG_FLAG    0
#if(CE6353_DEBUG_FLAG)
    #define NIM_CE6353_PRINTF  soc_printf
#else
    #define NIM_CE6353_PRINTF(...)
#endif

//#define soc_printf(...)
#define Delay_Number    1
#define WAIT_Number    1
#define SYS_WAIT_MS(x) osal_delay((x*1000*WAIT_Number))//osal_task_sleep(x*WAIT_Number)    //osal_delay((x*1000))

#define abs(x)    (x<0) ? -x : x
#define sign(y)    (y>0) ? 1 : -1

#define mt353_rule
#define Mode_a


#define    NIM_353_FLAG_ENABLE             0x00000100    //050513 Tom
#define NIM_353_SCAN_END                        0x00000001
#define NIM_353_CHECKING            0x00000002

static UINT32 SYS_TUN_TYPE = 0;

ID f_mt353_tracking_task_id = OSAL_INVALID_ID;

ID      nim_353_flag = OSAL_INVALID_ID;
struct MT353_Lock_Info                 *MT353_CurChannelInfo;
static struct MT353_Demod_Info                MT353_COFDM_Info;

INT8 mt353_autoscan_stop_flag = 0;    //50622-01Angus
ID f_IIC_Sema_ID = OSAL_INVALID_ID;

/**************************************************************************************

****************************************************************************************/
#define SYS_TS_INPUT_INTERFACE   TSI_SPI_1
#define WAIT_TUNER_LOCK 200
#define WAIT_OFDM_LOCK  200
#define WAIT_FEC_LOCK     512

#define Sint32T_MAX 2147483647


static UINT32 mt353_ber = 0;
static UINT32 mt353_per = 0;
static UINT32 mt353_per_tot_cnt = 0;



static char nim_mt353_name[HLD_MAX_NAME_SIZE] = "NIM_COFDM_0";


static INT32 f_mt353_read(UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    INT32 err;

    osal_semaphore_capture(f_IIC_Sema_ID,TMO_FEVR);

    data[0] = reg_add;
    err = i2c_write_read(0, dev_add, data, 1, len);
    if(err!=0)
    {
        NIM_CE6353_PRINTF("nim_mt353_read: err = %d ,reg_add=%x!\n", err,reg_add);
        ;
    }
//        osal_delay(1000);

    osal_semaphore_release(f_IIC_Sema_ID);
    return err;

}

static INT32 f_mt353_write(UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    UINT8  i, buffer[8];
    INT32 err;

    osal_semaphore_capture(f_IIC_Sema_ID,TMO_FEVR);
    if (len > 7)
    {
        osal_semaphore_release(f_IIC_Sema_ID);
        return ERR_FAILUE;
    }
    buffer[0] = reg_add;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = data[i];
    }

    err=i2c_write(0, dev_add, buffer, len + 1);
        if(err!=0)
    {
        NIM_CE6353_PRINTF("nim_mt353_write: err = %d !\n", err);
        ;
    }
    osal_semaphore_release(f_IIC_Sema_ID);
    return  err;
}

void f_mt353_dump_register(struct nim_device *dev)
{
        UINT8  i=0;
        UINT8 data[5];
        for(i=0;i<=0xff;i++)
        {
            f_mt353_read(dev->base_addr,i,data,1);
             NIM_CE6353_PRINTF("reg=0x%x, data=0x%x\n",i,data[0]);
             osal_delay(100);
        }

}
/*****************************************************************************
* INT32  f_mt353_attach()
* Description: mt353 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
INT32 f_mt353_attach(struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner)    //51117-01Angus
{

    struct nim_device *dev;
    struct nim_mt353_private * priv_mem;
       UINT32 tuner_id;

    if ((ptrCOFDM_Tuner == NULL))
    {
        NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(nim_mt353_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (struct nim_mt353_private *)MALLOC(sizeof(struct nim_mt353_private));
    if ((void*)priv_mem == NULL)
    {
        dev_free(dev);

        NIM_CE6353_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    MEMCPY((void*)&(priv_mem->Tuner_Control), (void*)ptrCOFDM_Tuner, sizeof(struct COFDM_TUNER_CONFIG_API));
    dev->priv = (void*)priv_mem;

    /* Function point init */

    dev->base_addr = SYS_COFDM_MT353_CHIP_ADRRESS;   //please check here
    dev->init = f_mt353_attach;
    dev->open = f_mt353_open;
    dev->stop = f_mt353_close;//Sam
    dev->disable=f_mt353_disable;//Sam
    dev->do_ioctl = f_mt353_ioctl;//nim_m3327_ioctl;//050810 yuchun for stop channel search
    dev->channel_change = f_mt353_channel_change;
    dev->channel_search = f_mt353_channel_search;//NULL;//f_mt353_channel_search;//nim_m3327_channel_search;
    dev->get_lock = f_mt353_get_lock;
    dev->get_freq = f_mt353_get_freq;
    dev->get_FEC = f_mt353_get_code_rate;
    dev->get_AGC = f_mt353_get_AGC;
    dev->get_SNR = f_mt353_get_SNR;
    //dev->get_BER = f_mt353_get_BER;
    dev->get_guard_interval = f_mt353_get_GI;
    dev->get_fftmode = f_mt353_get_fftmode;
    dev->get_modulation = f_mt353_get_modulation;
    dev->get_spectrum_inv = f_mt353_get_specinv;
    //dev->get_HIER= f_mt353_get_hier_mode;
    //dev->get_priority=f_mt353_priority;
    dev->get_freq_offset =    f_mt353_get_freq_offset;

    f_IIC_Sema_ID=osal_semaphore_create(1);

    /*if((((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cChip)!=Tuner_Chip_QUANTEK )
    {
        ptrCOFDM_Tuner->tuner_config.Tuner_Write=NULL;
        ptrCOFDM_Tuner->tuner_config.Tuner_Read=NULL;
    }*/

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if (((struct nim_mt353_private*)dev->priv)->Tuner_Control.nim_Tuner_Init != NULL)
    {
        if((((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cChip)==Tuner_Chip_QUANTEK || (((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cChip)==Tuner_Chip_MAXLINEAR)
        {
            f_mt353_PassThrough(dev,TRUE);


            //if (((struct nim_mt353_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(&((struct nim_mt353_private*)dev->priv)->tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            if (((struct nim_mt353_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(&tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");

                f_mt353_PassThrough(dev,FALSE);

                return ERR_NO_DEV;
            }

            f_mt353_PassThrough(dev,FALSE);
        }
        else
        {
            if (((struct nim_mt353_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(0, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");

                return ERR_NO_DEV;
            }

        }

    }


    return SUCCESS;
}


/*****************************************************************************
* INT32 f_mt353_open(struct nim_device *dev)
* Description: mt353 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_open(struct nim_device *dev)
{

    UINT8    data = 0;
    UINT8    tdata[5] ;
    UINT8    lock = 0;
    struct nim_mt353_private * priv_mem = (struct nim_mt353_private *)(dev->priv);
    struct COFDM_TUNER_CONFIG_API * config_info = &(priv_mem->Tuner_Control);


    NIM_PRINTF("f_mt353_open\n");

    nim_353_flag = osal_flag_create(NIM_353_FLAG_ENABLE);//050513 Tom
#if 1

    if (nim_353_flag==OSAL_INVALID_ID)
        return ERR_FAILUE;


MT353_CurChannelInfo = (struct MT353_Lock_Info *)MALLOC(sizeof(struct MT353_Lock_Info));
    if (MT353_CurChannelInfo == NULL)
    {
        NIM_PRINTF("f_MT353_open MALLOC fail!\n");
        return ERR_FAILUE;
    }
    MEMSET(MT353_CurChannelInfo, 0, sizeof(struct MT353_Lock_Info));
#endif

        f_mt353_hw_init(dev);

      f_mt353_tracking_task_init(dev);

        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);

    
        return SUCCESS;
}
/*****************************************************************************
* void  f_mt353_get_chip_id(struct nim_device *dev)
* Description:  get  mt353chip_id
*
* Arguments: struct nim_device *dev*
*
* Return Value:
*****************************************************************************/

void f_mt353_get_chip_id(struct nim_device *dev)
{
    UINT8 data[1];
    f_mt353_read(dev->base_addr,REG353_CHIP_ID ,data,1);
    NIM_CE6353_PRINTF("CH_ID=%x\n",data[0]);
}


void Tnim_SetITB(struct nim_device *dev,UINT16 wTuner_IF_Freq)
{
    // EGOR Fixed to Default -> This is a simple Calc to do.
    UINT16 lTemp;//0xCD42 :ZL10353_2060
    UINT8 data[5];

    if ((wTuner_IF_Freq<= 5120) &&(wTuner_IF_Freq >= 4000))
    {
        //lTemp = 0x19F8;///4.57_LDW
        data[0]=0x19;//INPUT_FREQ_1
        data[1]=0xF8;//INPUT_FREQ_0
    }
    else
    {
        //lTemp = 0xCD7E;///36.125_LDW
        data[0]=0xCD;//INPUT_FREQ_1
        data[1]=0x7E;//INPUT_FREQ_0
    }

    f_mt353_write(dev->base_addr,REG353_ITBFREQ ,data,2);
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_ITBFREQ ,data,2);
    lTemp=(data[0]<<8)|data[1];
    NIM_CE6353_PRINTF("Tuner_IF=%d,Set ITB=%x\n",wTuner_IF_Freq,lTemp);
    #endif
}


/*****************************************************************************
* INT32  f_mt353_hw_init()
* Description: mt353 initialization (set initial register)
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/

void f_mt353_hw_init(struct nim_device *dev)
{
    UINT8 data[5];
    int ret;
    UINT16 i;
    struct nim_mt353_private * priv_mem = (struct nim_mt353_private *)(dev->priv);
    struct COFDM_TUNER_CONFIG_API * config_info = &(priv_mem->Tuner_Control);

    //f_mt353_gpio_reset();        //60113-01Angus move to root.c (customer requset)

    /*------------------------------------------------------------------------*/
    /* (1) Initialization */
    /*------------------------------------------------------------------------*/


    data[0]=REG353_ENABLE_CHIPEN | REG353_ENABLE_ADCEN;
    f_mt353_write(dev->base_addr,REG353_ENABLE ,data,1);

    if ((config_info->tuner_config.wTuner_IF_Freq <= 5120) && (config_info->tuner_config.wTuner_IF_Freq >= 4000))
    {
        MEMCPY(data, MT353_ClockMode_Low_IF,4);
        NIM_CE6353_PRINTF("ClockMode_Low_IF \n");
    }
    else
    {
        MEMCPY(data, MT353_ClockMode_IF,4);
        NIM_CE6353_PRINTF("ClockMode_IF \n");
    }
    f_mt353_write(dev->base_addr,REG353_CLKCTL0 ,data,4);
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,4);
    for (i=0; i<4; i++)
    {
        NIM_CE6353_PRINTF("REG[%x]=%x\n",i,data[i]);
    }
    #endif

    osal_delay(200);

    data[0] =REG353_RESET_FULL ;
    f_mt353_write(dev->base_addr,REG353_RESET,data,1);

    MT353_CurChannelInfo->ADC_CLK=45056;
       MT353_CurChannelInfo->FEC_CLK=56320;

    f_mt353_get_chip_id(dev);

    Tnim_SetITB(dev,config_info->tuner_config.wTuner_IF_Freq);

    #if 0
    f_mt353_write(dev->base_addr,REG353_AGCTARGET,(((struct nim_mt353_private*)dev->priv)->Tuner_Control.config_data.ptMT353),2);
    f_mt353_write(dev->base_addr,0x9C,(((struct nim_mt353_private*)dev->priv)->Tuner_Control.config_data.ptMT353)+2,1);
    f_mt353_write(dev->base_addr,REG353_AGC_CTL,(((struct nim_mt353_private*)dev->priv)->Tuner_Control.config_data.ptMT353)+3,1);
    #else
    for (i=0; i<g_ucInitTable_size/2; i++)
    {
            f_mt353_write(dev->base_addr,g_ucInitTable[2*i],&g_ucInitTable[2*i+1],1);
            #if(CE6353_DEBUG_FLAG)
            f_mt353_read(dev->base_addr,g_ucInitTable[2*i],data,1);
            NIM_CE6353_PRINTF("REG %x=%x\n",g_ucInitTable[2*i],data[0]);
            #endif
    }
    #endif



    /*------------------------------------------------------------------------*/
    /*                                                                        */
    /* (2) AGC t settings                              */
    /*                                                                        */
    /*------------------------------------------------------------------------*/


    if((config_info->config_data.Cofdm_Config & 0xf) == 0x04)
    {
        NIM_CE6353_PRINTF("DUAL AGC ACTIVE\n");

        data[0]=0x28;//change target for 1611+mt353 dual agc
        f_mt353_write(dev->base_addr,REG353_AGCTARGET ,data,1);
        f_mt353_read(dev->base_addr,REG353_AGCTARGET,data,1);
        NIM_CE6353_PRINTF("AGCTARGET %x\n",data[0]);

        /*NoLock_Per(3:0) :Defines the period between AGC_NoLock interrupts which are generated whenever the AGC is out of lock.*/
        data[0]=0x3;//200 ms
        f_mt353_write(dev->base_addr,REG353_AGC_CTL ,data,1);
        f_mt353_read(dev->base_addr,REG353_AGC_CTL ,data,1);
        NIM_CE6353_PRINTF("AGC_CTL  %x\n",data[0]);


        /* Imp_Ctl*/
        data[0] =0xA0 ;
        f_mt353_write(dev->base_addr,MT353REG_UK_REG1 ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_UK_REG1 ,data,1);
        NIM_CE6353_PRINTF("UK_REG1  %x\n",data[0]);

        f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
        data[0] = data[0] & 0xbf;
        f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);
        f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
        NIM_CE6353_PRINTF("OUTCTL0  %x\n",data[0]);


        data[0] = 0x00;
        f_mt353_write(dev->base_addr,MT353REG_AGC_CTRL_5 ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_CTRL_5 ,data,1);
        NIM_CE6353_PRINTF("AGC_CTRL_5  %x\n",data[0]);


        data[0] = 0x00;
        f_mt353_write(dev->base_addr,MT353REG_AGC_IF_LOLIM ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_IF_LOLIM ,data,1);
        NIM_CE6353_PRINTF("IF_LOLIM  %x\n",data[0]);


        data[0] = 0xFF;
        f_mt353_write(dev->base_addr,MT353REG_AGC_RF_HILIM ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_RF_HILIM ,data,1);
        NIM_CE6353_PRINTF("AGC_RF_HILIM  %x\n",data[0]);



        data[0] = 0xFF;
        f_mt353_write(dev->base_addr,MT353REG_AGC_IF_MAX ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_IF_MAX ,data,1);
        NIM_CE6353_PRINTF("MT353REG_AGC_IF_MAX  %x\n",data[0]);

        data[0] = 0x00;
        f_mt353_write(dev->base_addr,MT353REG_AGC_IF_MIN ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_IF_MIN ,data,1);
        NIM_CE6353_PRINTF("MT353REG_AGC_IF_MIN  %x\n",data[0]);



        data[0] = 0xFF;
        f_mt353_write(dev->base_addr,MT353REG_AGC_RF_MAX ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_RF_MAX ,data,1);
        NIM_CE6353_PRINTF("AGC_RF_MAX  %x\n",data[0]);

        data[0] = 0x00;
        f_mt353_write(dev->base_addr,MT353REG_AGC_RF_MIN ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_RF_MIN ,data,1);
        NIM_CE6353_PRINTF("AGC_RF_MIN  %x\n",data[0]);

        data[0] = 0x3F;
        f_mt353_write(dev->base_addr,MT353REG_AGC_KIF ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_KIF ,data,1);
        NIM_CE6353_PRINTF("AGC_KIF  %x\n",data[0]);


        data[0] = 0x3F;
        f_mt353_write(dev->base_addr,MT353REG_AGC_KRF ,data,1);
        f_mt353_read(dev->base_addr,MT353REG_AGC_KRF ,data,1);
        NIM_CE6353_PRINTF("AGC_KRF  %x\n",data[0]);


#ifdef RFAGC_ADJUST
        data[0] = 0x44;
        f_mt353_write(dev->base_addr,0x63 ,data,1);
#endif
    }
    else
    {
        f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
        data[0]=data[0]|0x40;//disable rf agc
        f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);
        NIM_CE6353_PRINTF("SINGLE AGC ACTIVE\n");

    }

    /*------------------------------------------------------------------------*/
    /*                                                                        */
    /* (3) Configure hardware dependent settings                              */
    /*                                                                        */
    /*------------------------------------------------------------------------*/


    //f_mt353_init_reg(dev);
    #if ((SYS_TS_INPUT_INTERFACE ==  TSI_SPI_0)||(SYS_TS_INPUT_INTERFACE ==  TSI_SPI_1))
    f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
    data[0] |=REG353_OUTCTL0_EN_TEI|REG353_OUTCTL0_BKERR_INV|REG353_OUTCTL0_BG12_5B;////data[0] =0x4D wo swap
    //|REG353_OUTCTL0_RFADC_EN|REG353_OUTCTL0_MCLKRAT_EN|REG353_OUTCTL0_MCLKIN_EN|REG353_OUTCTL0_MDO_SWP ;
    f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);

    f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,1);
    data[0]=data[0]|REG353_CLKCTL0_MCLKINVERT|REG353_CLKCTL0_HIGHSAMPLE;
    f_mt353_write(dev->base_addr,REG353_CLKCTL0 ,data,1);

    NIM_CE6353_PRINTF("TSI_SPI_1\n");
#elif (SYS_TS_INPUT_INTERFACE ==  TSI_SSI_0)
    f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,1);
    data[0]|=REG353_CLKCTL0_MCLKINVERT| REG353_CLKCTL0_SERIALOUT|REG353_CLKCTL0_HIGHSAMPLE;     //MCLK falling,SSI
    f_mt353_write(dev->base_addr,REG353_CLKCTL0 ,data,1);

    f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
    data[0]| =REG353_OUTCTL0_EN_TEI|REG353_OUTCTL0_BKERR_INV|REG353_OUTCTL0_BG12_5B;    //ERR high act,d0 msb
    f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);
    NIM_CE6353_PRINTF("TSI_SSI_0\n");

#elif(SYS_TS_INPUT_INTERFACE ==  TSI_SSI_1)
    f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
    data[0] |=REG353_OUTCTL0_EN_TEI|REG353_OUTCTL0_BKERR_INV|REG353_OUTCTL0_MDO_SWP|REG353_OUTCTL0_BG12_5B;    //ERR high act,d7 msb
    f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);

     f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,1);
     data[0]|=REG353_CLKCTL0_MCLKINVERT| REG353_CLKCTL0_SERIALOUT|REG353_CLKCTL0_HIGHSAMPLE;
        f_mt353_write(dev->base_addr,REG353_CLKCTL0 ,data,1);
    NIM_CE6353_PRINTF("TSI_SSI_1\n");

#else//TSI_SPI_1
    f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
    data[0] |=REG353_OUTCTL0_EN_TEI|REG353_OUTCTL0_BKERR_INV|REG353_OUTCTL0_BG12_5B;////data[0] =0x4D wo swap
    f_mt353_write(dev->base_addr,REG353_OUTCTL0 ,data,1);

    f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,1);
    data[0]|=REG353_CLKCTL0_MCLKINVERT|REG353_CLKCTL0_HIGHSAMPLE;
    f_mt353_write(dev->base_addr,REG353_CLKCTL0 ,data,1);
    NIM_CE6353_PRINTF("TSI_SPI_1\n");

 #endif


    /************************************
     *Register RS_ERR_PER_1       RS_ERR_PER_0
        *Address     0x60                       0x61
         *Bits           15:8                        7:0
    ****************************************/
    data[0] =0x00;
    data[1] =0x16;
    f_mt353_write(dev->base_addr,  REG353_RSERRPER,data, 2);
    f_mt353_read(dev->base_addr,  REG353_RSERRPER,data, 2);
    NIM_CE6353_PRINTF("RSPER  %x\n",data[0]);

    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_CLKCTL0 ,data,1);
     NIM_CE6353_PRINTF("RE_51=%x\n",data[0]);

     f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
     NIM_CE6353_PRINTF("RE_5A=%x\n",data[0]);

    f_mt353_read(dev->base_addr,0x5b ,data,1);
     NIM_CE6353_PRINTF("RE_5B=%x\n",data[0]);
     #endif

         /*------------------------------------------------------------------------*/
    /*                                                                        */
    /* (4) reset again                             */
    /*                                                                        */
    /*------------------------------------------------------------------------*/


    data[0] = REG353_RESET_PART  ;//ADD PART353 no this
    f_mt353_write(dev->base_addr,REG353_RESET,data,1);

    data[0]=REG353_ADCCTL0_CAL ;//recalibration
    f_mt353_write(dev->base_addr,REG353_ADCCTL0,data,1);

    data[0]=REG353_ADCCTL0_CAL_RESET ;//recalibration back
    f_mt353_write(dev->base_addr,REG353_ADCCTL0 ,data,1);

    //data[0] = SYS_TUN_BASE_ADDR;
    //data[0] = ((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.c_tuner_base_addr;
    //f_mt353_write(dev->base_addr,REG353_TUNADDRESS,data,1);

    return;

}

/*****************************************************************************
* INT32  f_mt353_init_reg()
* Description: mt353 register initialization (set initial register)
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#if 0
static INT32 f_mt353_init_reg(struct nim_device *dev)
{
    UINT8 data[5];
    UINT16 i;



    return  SUCCESS;
}
#endif


void f_mt353_PassThrough(struct nim_device *dev,BOOL Open)
{

    UINT8 data;

    f_mt353_read(dev->base_addr,REG353_SCANCTL,&data,1);
       data &= ~(REG353_SCANCTL_BUSMASTER|REG353_SCANCTL_PASSTHRO);
        if (Open)
        {
        data = data|REG353_SCANCTL_PASSTHRO;
        //NIM_CE6353_PRINTF("ON\n");
        }
        else
        {
            data |= REG353_SCANCTL_BUSMASTER;
      // NIM_CE6353_PRINTF("OFF\n");
        }

        //TNIM_WRITE(REG353_SCANCTL);
    f_mt353_write(dev->base_addr,REG353_SCANCTL,&data,1);
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_SCANCTL,&data,1);
    NIM_CE6353_PRINTF("BP=%x\n",data);
    #endif

}

/*****************************************************************************
void f_mt353_gpio_reset()
* Description: gpio_reset
* Arguments:
*  Parameter1:
* Return Value:
*****************************************************************************/

void f_mt353_gpio_reset()  //0904 Sam chen  for HW RESET
{

#if (POS_COFDM_RESET != GPIO_NULL)//051109 yuchun
    HAL_GPIO_BIT_DIR_SET(POS_COFDM_RESET, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(POS_COFDM_RESET,0);
    osal_delay(50000);

    HAL_GPIO_BIT_DIR_SET(POS_COFDM_RESET    , HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(POS_COFDM_RESET,1);
    osal_delay(100);
#endif
}

/*****************************************************************************
* INT32 f_mt353_close(struct nim_device *dev)
* Description: mt353 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_close(struct nim_device *dev)
{
    UINT8 data = 0;

#if 0
      data = data | REG353_ENABLE_MPEGDIS;
        f_mt353_write(dev->base_addr,REG353_ENABLE,&data,1);
#else //051117 yuchun add
    data = REG353_RESET_ALL;//reset lock LED
    f_mt353_write(dev->base_addr,REG353_RESET,&data,1);

    f_mt353_read(dev->base_addr,REG353_ENABLE,&data,1);
    data &= ~REG353_ENABLE_CHIPEN;//disable ZL100353
    f_mt353_write(dev->base_addr,REG353_ENABLE,&data,1);
#endif


    return SUCCESS;
}



/*****************************************************************************
* INT32 f_mt353_disable(struct nim_device *dev)
* Description: mt353 disable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_mt353_disable(struct nim_device *dev)
{
    UINT8 data = 0;
#if 0
    f_mt353_read(dev->base_addr,MT353_CONFIG ,&data,1);
    data=data|0x40;   //tri_state
    f_mt353_write(dev->base_addr,MT353_CONFIG ,&data,1);
#else

    data = REG353_RESET_ALL;
    f_mt353_write(dev->base_addr,REG353_RESET,&data,1);

    //f_mt353_init_reg(dev);
    #endif
    return SUCCESS;
}
/*****************************************************************************
* INT32 f_mt353_channel_search(struct nim_device *dev, UINT32 freq);

* Description: mt353 channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency*
* Return Value: INT32
*****************************************************************************/
//050803 yuchun  add
static INT32 f_mt353_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority)
{
    UINT8     i,data[1];
    INT32    chsearch=ERR_FAILED ,center_freq;
    INT32 freq_offset_reg;

     UINT32    Center_NPRO,Search_NPRO,tmp_freq;
    UINT32 First_Frequency;
    UINT8 num,freq_step;
    UINT8 j=0,k = 0;
    INT32 get_freq_offset;
    INT32 remainder;
    UINT16 temp_Step_Freq;



    osal_flag_clear(nim_353_flag,NIM_353_SCAN_END);    

#if     1
    chsearch=f_mt353_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);
    return chsearch;
#else
    First_Frequency = freq;
    Center_NPRO=(((freq+36250)*6)/100);


    num=freq_offset/166;

#ifdef ALiS01_M3330_MODEL1
//#if 1
    j = 2;
    k = 10;
#else
    while((3+7*j)<(num-1))
    {
        j++;
    }
    j=1+2*j;
    k = 30;
#endif
    for(i=0;i<j;i++)
    {
        chsearch=ERR_FAILED;
        if (mt353_autoscan_stop_flag)    //50622-01Angus
        {
            NIM_PRINTF("channel search break!!=%d\n",i);
            chsearch=ERR_FAILED;
            break;
        }


         if(i%2)
        {
            Search_NPRO=Center_NPRO+(i+1)*k;
         }
        else
        {
            Search_NPRO=Center_NPRO-i*k;
        }

        tmp_freq=(Search_NPRO*100)/6;

         if(tmp_freq>36250)
            freq=tmp_freq-36250;

        f_mt353_read(dev->base_addr,0x7c,data,1); //MCLKCTL
        data[0] &=~1;
        f_mt353_write(dev->base_addr,0x7c,data,1); //MCLKCTL

        chsearch=f_mt353_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);
            if(chsearch==SUCCESS)
               {
                    f_mt353_freq_offset(dev, bandwidth,  MT353_CurChannelInfo->Mode, &get_freq_offset);

#if 0
            freq-=get_freq_offset;
            rem = (freq % (((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq));
            if (rem > ((((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq)>>1) )
                freq += ((((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq) - rem);
            else
                freq -= rem;
#endif

            freq-=get_freq_offset;

            remainder=(freq%1000);
            freq-=remainder;

            freq_step=(remainder*(1000/(((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq))+500)/1000;    //51117-01Angus
            if(freq_step==(1000/(((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq)))                //51117-01Angus
            {
                freq+=1000;
            }
            else
            {
                if ((((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq) == 166)
                    temp_Step_Freq = 1667;
                else
                    temp_Step_Freq = (((struct nim_mt353_private*)dev->priv)->Tuner_Control.tuner_config.cTuner_Step_Freq) * 10;
                freq+=freq_step*temp_Step_Freq/10;            //51117-01Angus
            }
            chsearch=f_mt353_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse,priority);
                   break;
               }
    }
    if((i==j) && (chsearch!=SUCCESS) && (usage_type != MODE_AUTOSCAN))
        {
        freq=First_Frequency;//MT352_CurChannelInfo->First_Frequency;
        chsearch = f_mt353_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,MODE_CHANCHG,inverse, priority);
        }

    f_mt353_read(dev->base_addr,0x7c,data,1); //MCLKCTL
    data[0] |=1;
    f_mt353_write(dev->base_addr,0x7c,data,1); //MCLKCTL

    osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
    return chsearch;
#endif
}


static INT32 f_mt353_freq_offset(struct nim_device *dev, UINT32 bandwidth, UINT8 fft_mode, INT32 *freq_offset)
{
    UINT8 data[4];
    UINT8 mode;
    UINT32 freq_offset_tmp = 0;
    f_mt353_read(dev->base_addr,REG353_FREQOFFSET,data,3);
    freq_offset_tmp=((data[0]<<16)|(data[1]<<8)|data[2]);

    if(data[0] & 0x80)
    {
        freq_offset_tmp = (~freq_offset_tmp + 1) & 0xffffff;
    }
    else
        *freq_offset = freq_offset_tmp;

    switch(fft_mode)
        {
        case MODE_2K:
            mode = 1;
            break;
        case MODE_8K:
            mode = 4;
            break;
            default:
            mode = 4;
                break;
        }
    freq_offset_tmp= freq_offset_tmp/8192; //(8192*7*2048*8);
    freq_offset_tmp = freq_offset_tmp*64000/7;
    freq_offset_tmp= freq_offset_tmp*bandwidth/(8*2048*mode);


    if(data[0] & 0x80)
        *freq_offset = -freq_offset_tmp;
    else
        *freq_offset = freq_offset_tmp;


    //soc_printf("freq_offset = %d\n",*freq_offset);
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_mt353_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type,UINT8 inverse);

* Description: stv0360 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency
*  Parameter3: UINT32 bandwidth
*  Parameter4: UINT8  guard_interval
*  Parameter5: UINT8  fft_mode
*  Parameter6: UINT8  modulation
*  Parameter7: UINT8  fec
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority)
{

    UINT8    data[6];    //51117-01Angus
    UINT32    temp;
    UINT8    ret_flag = SCAN_TIMEOUT;// LOCK_OK;
    TMO     start_time, temp_time;
    struct nim_mt353_private *dev_priv;    //51117-01Angus
    UINT8 code_rate,Tuner_Retry=0,Can_Tuner_Retry=0;
    UINT8 lock=0;

    dev_priv = (struct nim_mt353_private *)dev->priv;

    UINT32 tuner_id = 0;//dev_priv->tuner_id;

    osal_flag_clear(nim_353_flag, NIM_353_CHECKING);

    NIM_CE6353_PRINTF("freq %d, bandwidth %d\n",freq,bandwidth);
    // NIM_PRINTF("freq %d, bandwidth %d,guard_interval %d, fft_mode %d,
    //modulation %d,fec %d,usage_type %d,inverse %d\n ",freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse);
    osal_flag_clear(nim_353_flag,NIM_353_SCAN_END);

    MT353_CurChannelInfo->Frequency = freq;
    MT353_CurChannelInfo->ChannelBW = bandwidth;

    dev_priv = dev->priv;

    /*-------------------------------------------------------------------------*/
    /* (1)Set demod                            */
    /*-------------------------------------------------------------------------*/

    f_mt353_read(dev->base_addr,REG353_ACQCTL,data,1);
    data[0] &= REG353_ACQ_CTL_AUTO;    //ACQ_CTL_AUTO
    f_mt353_write(dev->base_addr,REG353_ACQCTL,data,1);

    temp = MT353_HP;
    switch(modulation)
    {
        case TPS_CONST_QPSK:
            temp |= MT353_MODU_QPSK;
            break;
        case TPS_CONST_16QAM:
            temp |= MT353_MODU_16QAM;
            break;
        case TPS_CONST_64QAM:
            temp |= MT353_MODU_64QAM;
            break;
        default:
            temp |= MT353_MODU_64QAM;
            break;
    }

    switch(fec)
    {
        case FEC_1_2:
            temp |= MT353_FEC_1_2;
            break;
        case FEC_2_3:
            temp |= MT353_FEC_2_3;
            break;
        case FEC_3_4:
            temp |= MT353_FEC_3_4;
            break;
        case FEC_5_6:
            temp |= MT353_FEC_5_6;
            break;
        case FEC_7_8:
            temp |= MT353_FEC_7_8;
            break;
        default:
            temp |= MT353_FEC_2_3;
            break;
    }

    switch(guard_interval)
    {
        case GUARD_1_32:
            temp |= MT353_GUARD_1_32;
            break;
        case GUARD_1_16:
            temp |= MT353_GUARD_1_16;
            break;
        case GUARD_1_8:
            temp |= MT353_GUARD_1_8;
            break;
        case GUARD_1_4:
            temp |= MT353_GUARD_1_4;
            break;
        default:
            temp |= MT353_GUARD_1_32;
            break;
    }

    switch(fft_mode)
    {
        case MODE_2K:
            temp |= MT353_MODE_2K;
            break;
        case MODE_8K:
            temp |= MT353_MODE_8K;
            break;
        default:
            temp |= MT353_MODE_8K;
            break;
    }


    data[0] = temp >> 8;//H_BIT
    data[1] = temp & 0xff;//L_BIT


    if(priority==LPSEL)
        data[0]|=TPS_LPSEL ;
    else
         data[0] &=~(TPS_LPSEL) ;

    f_mt353_write(dev->base_addr,REG353_TPSGIVEN,data,2);


    f_mt353_read(dev->base_addr, 0x64,data,1);
    data[0] &=~3;
    switch(bandwidth)
    {
        case 6:
            data[0] |= 0;
            break;
        case 7:
            data[0] |= 1;
            break;
        case 8:
            data[0] |= 2;
            break;
        default:
            data[0] |= 2;
            break;
    }
    f_mt353_write(dev->base_addr, 0x64,data,1);
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,0x64,data,1); //
    NIM_CE6353_PRINTF("OFDMBW=%x\n",data[0]);
    #endif
    switch(bandwidth)
    {
        case 6:
            data[0]  = 0xdd;
            break;
        case 7:
            data[0]  = 0x73;
            break;
        case 8:
            data[0]  = 0x73;
            break;
        default:
            data[0]  = 0x73;
            break;
    }
    //soc_printf("cc= %x\n",data[0] );
    f_mt353_write(dev->base_addr, 0xcc,data ,1);

    data[0] =MT353_OFDM_8K_WAIT;/* 8k or 2k/8k*///512ms
    f_mt353_write(dev->base_addr, REG353_OFDM_LOCK_TIME,data,1);

#if 1
    //data[0] = 0x48;//0x0A; //    Ti2c Speed
    data[0] = 0x4A;
    f_mt353_write(dev->base_addr,REG353_SCANCTL ,data,1);
#endif

    f_mt353_read(dev->base_addr, REG353_CAPTURE , data, 1);
    data[0] &= 0xF8;
    //data[0] |=0x17;
#ifdef ALiL00_M3330_MODEL1
    data[0] |= 1;
#else
    data[0] |= ((bandwidth==8) ? 0x03:0x04);//500KHz
#endif
    f_mt353_write(dev->base_addr,REG353_CAPTURE ,data,1);

 /*Program the TRL register, according to the bandwidth: TRL_nom_rate= 2^16 (64/7) * (ucBW/8) *M / Fadc(MHz)
    = 2^16*64*4000/7/8 * ucBW *M/(4*Fadc(kHz))    =299593142.86 *ucBW*M/(4*Fadc(kHz))    where M i2 2 for high sample rate, 1 otherwise */

    temp = 299593143;
        temp  *= bandwidth; /* still less than 2^32*/
        temp  /= MT353_CurChannelInfo->ADC_CLK;
        if (MT353_CurChannelInfo->ADC_CLK >30000) temp *=2;
        temp +=2;
        temp/=4;

    data[0]=temp>>8;
    data[1]=temp&0xff;
    f_mt353_write(dev->base_addr,REG353_TRLRATE, data, 2); //OFDMCLKCTL
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_TRLRATE, data, 2);
    NIM_CE6353_PRINTF("TRLRATE=%x\n",((data[0]<<8)|data[1]));
    #endif

    #if 0
    data[0] = 0x50;
    #else
    /* program MCLK_CTL to optimize the MPEG data clock MCLK_CTL = 128 *8/BW * FECclk(kHz)/61440  = FEC_Clk/BW /60    */
       temp=MT353_CurChannelInfo->FEC_CLK;   //khz
    temp/=bandwidth;
    temp/=60;
    #endif

    f_mt353_write(dev->base_addr,REG353_MCLKCTL,data,1); //MCLKCTL
    #if(CE6353_DEBUG_FLAG)
    f_mt353_read(dev->base_addr,REG353_MCLKCTL,data,1);
    NIM_CE6353_PRINTF("MCLKCTL=%x\n",data[0]);
    #endif

    /*-------------------------------------------------------------------------*/
    /* (2)Set Tuner   & Channel Acquisition:                             */
    /*-------------------------------------------------------------------------*/
    if(SYS_TUN_TYPE != ED5065)
    {
        f_mt353_PassThrough(dev,TRUE);
        if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,FAST_TIMECST_AGC,data,_1st_i2c_cmd)!=SUCCESS)        //51117-01Angus
        {
            NIM_CE6353_PRINTF("WRITE tuner fail\n");

        }
        else
        {
            if(dev_priv->Tuner_Control.tuner_config.cChip==Tuner_Chip_INFINEON)
            {
                if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,FAST_TIMECST_AGC,data,_2nd_i2c_cmd)!=SUCCESS)        //51117-01Angus
                {
                    NIM_CE6353_PRINTF("WRITE tuner fail\n");

                }
            }
        }
        if(usage_type==MODE_CHANCHG)
        {
            if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,SLOW_TIMECST_AGC,data,_1st_i2c_cmd)!=SUCCESS)        //51117-01Angus
            {
                NIM_CE6353_PRINTF("WRITE tuner fail\n");
            }
            else
            {
                if(dev_priv->Tuner_Control.tuner_config.cChip==Tuner_Chip_INFINEON)
                {
                    if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,SLOW_TIMECST_AGC,data,_2nd_i2c_cmd)!=SUCCESS)        //51117-01Angus
                    {
                        NIM_CE6353_PRINTF("WRITE tuner fail\n");
                    }
                }
            }
        }
        f_mt353_PassThrough(dev,FALSE);

        data[0] = REG353_FSMGO_GO;
        f_mt353_write(dev->base_addr,REG353_FSMGO,data,1);

        osal_task_sleep(50);

    /*-------------------------------------------------------------------------*/
    /* (3)WAIT TUNER_LOCK                              */
    /*-------------------------------------------------------------------------*/
        //start_time = osal_get_tick();

            Tuner_Retry=0;
            do
            {
                //if((dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_INFINEON|| (dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_MAXLINEAR)
                    Tuner_Retry++;
                    f_mt353_PassThrough(dev,TRUE);
                    if(dev_priv->Tuner_Control.nim_Tuner_Status(tuner_id ,&lock)==ERR_FAILUE)
                    {
                        NIM_CE6353_PRINTF("read tuner fail\n");
                        ;
                    }
                    f_mt353_PassThrough(dev,FALSE);
                                        NIM_CE6353_PRINTF("Wait Tuner:lock=%d\n ",lock);
                    if(lock==1)
                            break;

                    else if(Tuner_Retry>5)
                    {
                        NIM_CE6353_PRINTF("Tuner_ULock\n");
                        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
                        return ERR_FAILED;
                    }

            }while(lock==0);

    }
    else
    {

    /*-------------------------------------------------------------------------*/
    /* (2)Set Tuner   & Channel Acquisition:                             */
    /*-------------------------------------------------------------------------*/


        data[0] =dev_priv->Tuner_Control.tuner_config.c_tuner_base_addr;
        f_mt353_write(dev->base_addr,REG353_TUNADDRESS,data,1);

        if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,FAST_TIMECST_AGC,data,_1st_i2c_cmd)!=SUCCESS)
        {
            NIM_CE6353_PRINTF("WRITE tuner fail\n");
            ;
        }

        f_mt353_write(dev->base_addr,REG353_CHAN_START ,data,2);
        f_mt353_write(dev->base_addr,REG353_TUNER_CONT_1,data+2,2);
        f_mt353_write(dev->base_addr,REG353_CHAN_STOP,data,2);

        //if((dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_INFINEON)        //51117-01Angus
        {

            data[0]=REG353_TUNERGO_GO ;
            f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);

            osal_delay(50);

            if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,FAST_TIMECST_AGC,data,_2nd_i2c_cmd)!=SUCCESS)
            {
                NIM_PRINTF("invalid tuner freq\n");
                ;
            }

            f_mt353_write(dev->base_addr,REG353_CHAN_START ,data,2);
            f_mt353_write(dev->base_addr,REG353_TUNER_CONT_1,data+4,1);
            f_mt353_write(dev->base_addr,REG353_TUNER_CONT_0,data+3,1);
            f_mt353_write(dev->base_addr,REG353_CHAN_STOP,data,2);
        }

        if(usage_type==MODE_CHANCHG)
        {
            data[0]=REG353_TUNERGO_GO ;
            f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);
            osal_task_sleep(50);

            if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,SLOW_TIMECST_AGC,data,_1st_i2c_cmd)!=SUCCESS)
            {
                NIM_CE6353_PRINTF("WRITE tuner fail\n");
                ;
            }

            f_mt353_write(dev->base_addr,REG353_CHAN_START ,data,2);
            f_mt353_write(dev->base_addr,REG353_TUNER_CONT_1,data+2,2);
            f_mt353_write(dev->base_addr,REG353_CHAN_STOP,data,2);

            //if((dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_INFINEON)        //51117-01Angus
            {

                data[0]=REG353_TUNERGO_GO ;
                f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);

                osal_delay(50);

                if(dev_priv->Tuner_Control.nim_Tuner_Control(tuner_id, freq,bandwidth,SLOW_TIMECST_AGC,data,_2nd_i2c_cmd)!=SUCCESS)
                {
                    NIM_PRINTF("invalid tuner freq\n");
                    ;
                }

                f_mt353_write(dev->base_addr,REG353_CHAN_START ,data,2);
                f_mt353_write(dev->base_addr,REG353_TUNER_CONT_1,data+4,1);
                f_mt353_write(dev->base_addr,REG353_TUNER_CONT_0,data+3,1);
                f_mt353_write(dev->base_addr,REG353_CHAN_STOP,data,2);
            }

        }
        data[0]=REG353_TUNERGO_GO ;
        f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);
        //data[0] = REG353_FSMGO_GO;
        //f_mt353_write(dev->base_addr,REG353_FSMGO,data,1);
        osal_task_sleep(50);

    /*-------------------------------------------------------------------------*/
    /* (3)WAIT TUNER_LOCK                              */
    /*-------------------------------------------------------------------------*/

            Can_Tuner_Retry=0;
            do
            {
                //if((dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_INFINEON|| (dev_priv->Tuner_Control.tuner_config.cChip)==Tuner_Chip_MAXLINEAR)
                    Can_Tuner_Retry++;
                    f_mt353_read(dev->base_addr, REG353_STATUS_2, data,1);
                    lock = (data[0] & 0x40) >> 6;
                    NIM_CE6353_PRINTF("Wait Tuner:STATUS_2 = %x,lock=%d\n ",data[0],lock);

                    if(lock==1)
                            break;
                    
                    else if(Can_Tuner_Retry>5)
                    {
                        NIM_CE6353_PRINTF("Tuner_ULock \n");
                        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
                        return ERR_FAILED;
                    }

            }while(lock==0);
    }


    /*-------------------------------------------------------------------------*/
    /* (4)WAIT SYMBOL_LOCK                         */
    /*-------------------------------------------------------------------------*/

    start_time = osal_get_tick();
    while(1)
    {
        //osal_task_sleep(1000);
        f_mt353_read(dev->base_addr, REG353_STATUS_0, data, 1);
        //soc_printf("MT353_FECSTAT = %x\n ",data[0]);
        if ((data[0]&MT353_SYMBOL_LOCK_FLAG)== MT353_SYMBOL_LOCK_FLAG)
        {
            ret_flag = LOCK_OK;
            NIM_MT353_PRINTF("SPECTRUM INVER=%d\n",(data[0]>>6));
            NIM_CE6353_PRINTF("REG353_STATUS_1=0x%x , LOCK TIME=%d\n",data[0],osal_get_tick() - start_time);
            //NIM_MT353_PRINTF("MT353_FEC_LOCK !!! \n" );
            break;
        }
        osal_task_sleep(5);

        temp_time = osal_get_tick();
        if(temp_time >= start_time ?  ( temp_time - start_time >256/*512*/) : (start_time - temp_time < 0xFFFFFFFF - 256/*512*/))
        {
            ret_flag = SCAN_TIMEOUT;
            NIM_CE6353_PRINTF("MT353_SYMBOL_NO_LOCK data[0] =0x%x ,ret=%d \n", data[0],ret_flag);
            osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
            return ERR_FAILED;
        }
    }



    /*-------------------------------------------------------------------------*/
    /* (5)WAIT OFDM_LOCK                         */
    /*-------------------------------------------------------------------------*/

    start_time = osal_get_tick();
    while(1)
    {
        //osal_task_sleep(1000);
        f_mt353_read(dev->base_addr,REG353_STATUS_0 , data, 1);
        NIM_MT353_PRINTF("MT353_OFDMSTAT = %x\n ",data[0]);
        if ((data[0] & MT353_OFDM_LOCK_FLAG) == MT353_OFDM_LOCK_FLAG)
        {
            NIM_CE6353_PRINTF("MT353_OFDM_LOCK =0x%x , LOCK TIME=%d\n",data[0],osal_get_tick() - start_time);
            break;
        }
        osal_task_sleep(20);

        temp_time = osal_get_tick();

            if(temp_time >= start_time ?  ( temp_time - start_time > 512) : (start_time - temp_time < 0xFFFFFFFF - 512))
            {
                ret_flag = SCAN_TIMEOUT;
                //NIM_MT353_PRINTF("MT353_OFDM_NO_LOCK end_time = %d ret=%d\n",osal_get_tick(),ret_flag);
                // NIM_CE6353_PRINTF("REG353_STATUS_0=0x%x ,ret=%d ,time = %d\n", data[0],ret_flag,temp_time - start_time);
                //if (usage_type == MODE_CHANCHG)
                {
                    if(temp_time >= start_time ?  ( temp_time - start_time > 1024 ) : (start_time - temp_time < 0xFFFFFFFF - 1024))
                    {
                        f_mt353_read(dev->base_addr, REG353_STATUS_0, data,3);
                        NIM_CE6353_PRINTF("OFDM TIME OUT !REG353_STATUS_0 = %x\n ",data[0]);
                        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
                        if (usage_type == MODE_CHANCHG)
                        osal_flag_set(nim_353_flag, NIM_353_CHECKING);
                        return ERR_FAILED;
                    }
                    data[0]=0x10 ;//init ch
                    f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);
                    continue;
                }
                //else
                //{
                //    osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
                //    return ERR_FAILED;
                //}
        }
    }

    /*-------------------------------------------------------------------------*/
    /* (6)WAIT FEC_LOCK                       */
    /*-------------------------------------------------------------------------*/
    start_time = osal_get_tick();
    while(1)
    {
        //osal_task_sleep(1000);
        f_mt353_read(dev->base_addr, REG353_STATUS_1, data, 1);
        //soc_printf("MT353_FECSTAT = %x\n ",data[0]);
        if ((data[0]&MT353_FEC_LOCK_FLAG)== MT353_FEC_LOCK_FLAG)
        {
            ret_flag = LOCK_OK;
            NIM_MT353_PRINTF("SPECTRUM INVER=%d\n",(data[0]>>6));
            NIM_CE6353_PRINTF("REG353_STATUS_1=0x%x , LOCK TIME=%d\n",data[0],osal_get_tick() - start_time);
            //NIM_MT353_PRINTF("MT353_FEC_LOCK !!! \n" );
            break;
        }
        osal_task_sleep(10);

        temp_time = osal_get_tick();
        if(temp_time >= start_time ?  ( temp_time - start_time >512/*512*/) : (start_time - temp_time < 0xFFFFFFFF - 512/*512*/))
        {
            ret_flag = SCAN_TIMEOUT;
            //NIM_MT353_PRINTF("MT353_FEC_NO_LOCK end_time = %d ret=%d\n",osal_get_tick(),ret_flag);
            NIM_MT353_PRINTF("MT353_FEC_NO_LOCK data[0] =0x%x ,ret=%d \n", data[0],ret_flag);

            break;
        }
    }

    /*-------------------------------------------------------------------------*/
    /* (7)Status update:                            */
    /*-------------------------------------------------------------------------*/

    if (ret_flag == LOCK_OK  || ret_flag == TPS_UNLOCK || ret_flag == FEC_UNLOCK)
    {

        MT353_CurChannelInfo->Frequency = freq;
        MT353_CurChannelInfo->ChannelBW = (UINT8)bandwidth;

        f_mt353_read(dev->base_addr,REG353_OUTCTL0 ,data,1);
         NIM_CE6353_PRINTF("RE_5A=%x\n",data[0]);

        f_mt353_getinfo(dev, &code_rate, &guard_interval, &fft_mode, &modulation);

        //MT352_CurChannelInfo->HPRates = code_rate;
        MT353_CurChannelInfo->Guard= guard_interval;
        MT353_CurChannelInfo->Mode= fft_mode;
        MT353_CurChannelInfo->Modulation= modulation;
        MT353_CurChannelInfo->FECRates = code_rate;

        MT353_CurChannelInfo->Hierarchy = f_mt353_hier_mode(dev);
        MT353_CurChannelInfo->Priority=priority;
        MT353_CurChannelInfo->lock_status=1;

        f_mt353_read(dev->base_addr, REG353_STATUS_0, data,3);
        NIM_CE6353_PRINTF("REG353_STATUS_0 = %x\n ",data[0]);
        NIM_CE6353_PRINTF("REG353_STATUS_1 = %x\n ",data[1]);
        NIM_CE6353_PRINTF("REG353_STATUS_2 = %x\n ",data[2]);

        f_mt353_read(dev->base_addr, 0x00, data,6);

        NIM_CE6353_PRINTF("REG353_0 = %x\n ",data[0]);
        NIM_CE6353_PRINTF("REG353_1 = %x\n ",data[1]);
        NIM_CE6353_PRINTF("REG353_2 = %x\n ",data[2]);
        NIM_CE6353_PRINTF("REG353_3 = %x\n ",data[3]);
        NIM_CE6353_PRINTF("REG353_4 = %x\n ",data[4]);
        NIM_CE6353_PRINTF("REG353_5 = %x\n ",data[5]);
        NIM_CE6353_PRINTF("SUCCESS end_time = %d\n",osal_get_tick()-start_time);

        #if(CE6353_DEBUG_FLAG)
        // f_mt353_dump_register(dev);
        #endif
        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
        return SUCCESS;
    }
    else
    {
          MT353_CurChannelInfo->lock_status=0;
        NIM_CE6353_PRINTF("ERR_FAILED end_time = %d ret=%d\n",osal_get_tick()-start_time,ret_flag);

        f_mt353_read(dev->base_addr, REG353_STATUS_0, data,3);
        NIM_CE6353_PRINTF("REG353_STATUS_0 = %x\n ",data[0]);
        NIM_CE6353_PRINTF("REG353_STATUS_1 = %x\n ",data[1]);
        NIM_CE6353_PRINTF("REG353_STATUS_2 = %x\n ",data[2]);

        f_mt353_read(dev->base_addr, 0x00, data,6);

        NIM_CE6353_PRINTF("REG353_0 = %x\n ",data[0]);
        NIM_CE6353_PRINTF("REG353_1 = %x\n ",data[1]);
        NIM_CE6353_PRINTF("REG353_2 = %x\n ",data[2]);
        NIM_CE6353_PRINTF("REG353_3 = %x\n ",data[3]);
        NIM_CE6353_PRINTF("REG353_4 = %x\n ",data[4]);
        NIM_CE6353_PRINTF("REG353_5 = %x\n ",data[5]);
        #if(CE6353_DEBUG_FLAG)
        // f_mt353_dump_register(dev);
        #endif

        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
        return ERR_FAILED;
    }
}



static INT32 f_mt353_getinfo(struct nim_device *dev, UINT8 *code_rate, UINT8 *guard_interval, UINT8 *fft_mode, UINT8 *modulation)
{
    UINT8 data[2];
    UINT16 tdata;
    UINT16 i;

    for(i = 0; i < 500; i++)
    {
        f_mt353_read(dev->base_addr, REG353_TPSRECEIVED ,data,2);
        if((data[0]&0x80) == 0x80)
            break;
        else
            //osal_task_sleep(1);
            osal_delay(100);
    }

        tdata=((data[0]<<8)+data[1]);
        tdata=((tdata&0x0380)>>7);
        switch (tdata)
        {
            case 0:
                *code_rate =FEC_1_2;
                break;
            case 1:
                *code_rate =FEC_2_3;
            break;
            case 2:
                *code_rate =FEC_3_4;
                break;
            case 3:
                *code_rate =FEC_5_6;
                break;
            case 4:
                *code_rate =FEC_7_8;
                break;
            default:
                *code_rate = 0; /* error */
            break;
        }
                        
            tdata= (UINT16)((data[1]&0x0C) >> 2);
            switch (tdata)
        {
            case 0:
                *guard_interval = guard_1_32;
                break;
            case 1:
                *guard_interval = guard_1_16;
                break;
            case 2:
                *guard_interval = guard_1_8;
                break;
            case 3:
                *guard_interval = guard_1_4;
                break;
            default:
                *guard_interval = 0xff; /* error */
                break;
        }

        switch (data[1] & 0x03)
        {
                case 0:
                    *fft_mode = MODE_2K;
                    break;
                case 1:
                    *fft_mode = MODE_8K;
                    break;
                default:
                    *fft_mode = 0xff; /* error */
                    break;
           }

        tdata= (UINT16)(data[0]&0x60)>>5;
        switch (tdata)
        {
            case 0:
                * modulation = TPS_CONST_QPSK;
                break;
            case 1:
                * modulation = TPS_CONST_16QAM;
                break;
            case 2:
                * modulation = TPS_CONST_64QAM;
                break;
            default:
                * modulation = 0xff; /* error */
                break;
        }
    return SUCCESS;
}


/************************Zarlink Semiconductor*********************************
*   Name:Tnim_RSUBC
*   Purpose:updates the 1 second RSUBC total. register-pair is reset when RS_UBC_0 is read

*Register RS_UBC_1      RS_UBC_0
*Address  0x14                0x15
*Bits
*             15:8                  7:0

*   Remarks:
*   Inputs:
*   Outputs:
********************************************************************************/
UINT32 Tnim_RSUBC(struct nim_device *dev)
{
    UINT32 RSUBCcount;
    UINT8  value[2];

   f_mt353_read(dev->base_addr,REG353_RSUBC,value,2); //0x14
   RSUBCcount= ((UINT32)value[0]<<8) + (UINT32)value[2];
 //  libc_printf("RSUBCcount=%d\n",RSUBCcount);

   return RSUBCcount;

}
/************************Zarlink Semiconductor*********************************
*   Name:Tnim_PostViterbiBER
*   Purpose:calculates Post-viterbi BER
*   Remarks:formula is RS_BERCNT/(RS_ERRPER*1024*204*8)
*           the result is multiplied by 100E6 to get an integer value, the
*            formula becomes
*            BER = 100e6/(1024*204*8) *RS_BERCNT/RS_ERRPER = 60*RS_BERCNT/RS_ERRPER
*                    RS_ERRPER=77
*   Inputs:
*   Outputs:  BER * 100E6 , or -1 if an error occurs
*******************************************************************************/


INT32 CE6353_PostViterbi_BER_Read(struct nim_device *dev)
{
      UINT8  Interrupt_2;
      UINT8 lock=0;
      UINT8  value[3];
      UINT32  BER=0;
      UINT32  dwCount=0,dwPeriod=1;

      //First is to read FEC Lock !
      if (MT353_CurChannelInfo->lock_status!= 1)
    {
        mt353_ber=40000;
        mt353_per=100;
        return -1;
          }

      //Read Interrupt_2 bit[2] to ensure the end of RS bit error count period
      //INTERUPT_2 (Read Register 0x02, bit[2])
      f_mt353_read(dev->base_addr, 0x02 ,&Interrupt_2,1);

      //RS_ERR_CNT_2_1_0 (Read Register 0x11/0x12/0x13)
      if (Interrupt_2 & 0x04)
      {
        f_mt353_read(dev->base_addr, REG353_RSBERCNT, value, 3);

        f_mt353_read(dev->base_addr,REG353_RSERRPER,&dwPeriod,2);

    if (dwPeriod )
    {
            dwCount = ((UINT32)value[0]<<16) + ((UINT32)value[1]<<8) + (UINT32)value[2];
            //soc_printf("dwCount=%d,v=%d\n",dwCount,dwPeriod);

            BER=dwCount * 240;  /* x60 and x4 for maximum resolution*/
            BER/=dwPeriod;
            BER +=2;
            BER/=4;
            if (BER>Sint32T_MAX)
            {
                BER=Sint32T_MAX;
            }
            mt353_per_tot_cnt += 1;
    }
        mt353_ber=BER;
        mt353_per=BER/200;
        NIM_CE6353_PRINTF("dwCount=%d,    BER=%d,        PER=%d\n",dwCount,mt353_ber,mt353_per);
         return (INT32)BER;
      }
      else
         return -1;
}



/*****************************************************************************
* INT32 f_mt353_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/

static INT32 f_mt353_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data=0;
    OSAL_ER    result;
       UINT32    flgptn;
    static UINT32 dd_time=0;
    UINT8 ber_vld;
    UINT32 m_vbber, m_per;


    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("lock result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            //libc_printf("Get LOCK Fail, Wait ChannelChg Complete!\n");
        *lock=0xff;
    }

    else
    {

        f_mt353_read(dev->base_addr,REG353_ENABLE, &data, 1);
        if ((data &MT353_LOGIC_ENABLE)==MT353_LOGIC_ENABLE)  //353_EN
        {
            f_mt353_read(dev->base_addr,REG353_STATUS_1 , &data, 1);
            if ((data & MT353_FECSTAT_LOCKED)==MT353_FECSTAT_LOCKED)
            {
                *lock = 1;
                  MT353_CurChannelInfo->lock_status=1;
            }
            else
            {
                *lock = 0;
                  MT353_CurChannelInfo->lock_status=0;
            }


        }
        else
        {
            *lock=0;
             MT353_CurChannelInfo->lock_status=0;
        }

        if ( osal_get_tick() - dd_time >100)
        {
                CE6353_PostViterbi_BER_Read(dev);
                dd_time=osal_get_tick() ;
        }

    }



    return SUCCESS;
}

/*****************************************************************************
* INT32 f_mt353_get_freq(struct nim_device *dev, UINT32 *freq)
* Read M3327 frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *freq            :
*
* Return Value: void
*****************************************************************************/
static INT32 f_mt353_get_freq(struct nim_device *dev, UINT32 *freq)
{
    INT32 freq_off=0;
    UINT8  data[3];
    INT32  tdata=0;
        OSAL_ER    result;
        UINT32    flgptn;
#if 0
     result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);//050725 yuchun
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get Frequency Fail, Wait ChannelChg Complete!\n");
        *freq=0;
    }


        *freq=MT353_CurChannelInfo->Frequency;
        NIM_MT353_PRINTF("maintain MT353_CurChannelInfo->Frequency=%d\n",*freq);

            f_mt353_read(dev->base_addr,REG353_FREQOFFSET,data,3);
        tdata = (INT32)(data[2]);
        tdata |=(((INT32)(data[1]))<<8);
        tdata |=(((INT32)(data[0]&0x7F))<<16);

        if (tdata&0x800000)
        {
            tdata|=0xFF000000;
        }

        tdata/=16384;


                NIM_MT353_PRINTF("maintain MT353_CurChannelInfo->ChannelBW=%d\n",MT353_CurChannelInfo->ChannelBW);
            switch(MT353_CurChannelInfo->ChannelBW)
        {
            case 6:
                if(MT353_CurChannelInfo->Mode==0)    //2K mode
                {
                    tdata=(tdata*3348)/1000;    //frequency uint: K
                }
                else                    //8K mode
                {
                    tdata=(tdata*837)/1000;    //frequency uint: K
                }
                break;
            case 7:
                if(MT353_CurChannelInfo->Mode==0)    //2K mode
                {
                    tdata=(tdata*3906)/1000;    //frequency uint: K
                }
                else                    //8K mode
                {
                    tdata=(tdata*976)/1000;    //frequency uint: K
                }
                break;
            case 8:
                if(MT353_CurChannelInfo->Mode==0)    //2K mode
                {
                    tdata=(tdata*4464)/1000;    //frequency uint: K
                }
                else                    //8K mode
                {
                    tdata=(tdata*1100)/1000;    //frequency uint: K
                }
                break;
            default:
                NIM_MT353_PRINTF("Error!! Unknown Bandwidth!\n");
            break;

        }


        MT353_CurChannelInfo->FreqOffset=tdata;
        NIM_MT353_PRINTF("FreqOffset=%d KHz \n",tdata);

#else
        //*freq=MT353_CurChannelInfo->Frequency+MT353_CurChannelInfo->FreqOffset;
        *freq=MT353_CurChannelInfo->Frequency;
        NIM_MT353_PRINTF("freq=%d KHz \n", *freq);
   #endif

    return SUCCESS;

}

/*****************************************************************************
* INT32 f_mt353_get_code_rate(struct nim_device *dev, UINT8* code_rate)
* Description: Read mt353 code rate
*   FEC status (b6-b4)                      code rate                 return value
*    0                    1/2            1
*    1                    2/3            4
*    2                    3/4            8
*    3                    5/6            16
*    5                    7/8            32
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
#if 1
    *code_rate = MT353_CurChannelInfo->FECRates;
    return SUCCESS;

#else
    UINT8    data[2];
    UINT16    tdata;
    OSAL_ER    result;
    UINT32    flgptn;

    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get FEC Fail, Wait ChannelChg Complete!\n");
        *code_rate=0;
    }


        f_mt353_read(dev->base_addr,REG353_TPSRECEIVED,data,2);
        NIM_MT353_PRINTF("valid TPS_RECEIVED =0x%x\n",data[0]&0x80);
              if(0x80==(data[0]&0x80))
            {
                tdata=((data[0]<<8)+data[1]);
                tdata=((tdata&0x0380)>>6);
                NIM_MT353_PRINTF("fec tdata 2 =0x%x\n",tdata);

                switch (tdata)
            {
                case 0:
                    *code_rate =FEC_1_2;
                    NIM_MT353_PRINTF("fec 1_2\n");
                    break;
                case 1:
                    *code_rate =FEC_2_3;
                    NIM_MT353_PRINTF("fec 2_3\n");
                    break;
                case 2:
                    *code_rate =FEC_3_4;
                    NIM_MT353_PRINTF("fec 3_4\n");
                    break;
                case 3:
                    *code_rate =FEC_5_6;
                    NIM_MT353_PRINTF("fec 5_6\n");
                    break;
                case 4:
                    *code_rate =FEC_7_8;
                    NIM_MT353_PRINTF("fec 7_8\n");
                    break;
                default:
                    *code_rate = 0; /* error */
                    NIM_MT353_PRINTF("fec default\n");
                break;
            }
                }
        NIM_MT353_PRINTF(" code_rate =%d\n",* code_rate );
    return SUCCESS;
#endif
}

/*****************************************************************************
* INT32 f_mt353_get_GI(struct nim_device *dev, UINT8 *guard_interval)
* Description: Read mt353 guard interval
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* guard_interval
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
#if 1
    *guard_interval = MT353_CurChannelInfo->Guard;
    return SUCCESS;
#else
    UINT8    data[2];
        OSAL_ER    result;
        UINT32    flgptn;


        result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);


    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get guard_interval Fail, Wait ChannelChg Complete!\n");
//        *guard_interval=0;
        *guard_interval=0xff;//050725 yuchun
    }

        else
        {
            f_mt353_read(dev->base_addr, REG353_TPSRECEIVED  ,data,2);
            if(0x80==(data[0]&0x80))
            {
            //    data[1]=((data[1]&0x06) >> 2);
                data[1]=(data[1]&0x0C) >> 2;
                NIM_MT353_PRINTF("GI data=%d\n",data[1]);
                switch (data[1])
            {
            case 0:
                *guard_interval = guard_1_32;
                break;
            case 1:
                *guard_interval = guard_1_16;
                break;
            case 2:
                *guard_interval = guard_1_8;
                break;
            case 3:
                *guard_interval = guard_1_4;
                NIM_MT353_PRINTF("GI 1_4\n");
                break;
            default:
                *guard_interval = 0xff; /* error */
                NIM_MT353_PRINTF("GI default\n");
                break;
            }

           }
       }

       NIM_MT353_PRINTF(" guard_interval =%d\n", *guard_interval );
    return SUCCESS;
    #endif
}


/*****************************************************************************
* INT32 f_mt353_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
* Description: Read mt353 fft_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* fft_mode
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
#if 1
    *fft_mode = MT353_CurChannelInfo->Mode;
    return SUCCESS;

#else
    UINT8    data[2];
        OSAL_ER    result;
        UINT32    flgptn;


        result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get fft_mode Fail, Wait ChannelChg Complete!\n");
        *fft_mode=0;
    }

    
        else
        {
            f_mt353_read(dev->base_addr, REG353_TPSRECEIVED ,data,2);
                if(0x80==(data[0]&0x80))
            {
                NIM_MT353_PRINTF("MODE=%d\n",(data[1] & 0x03));
                switch (data[1] & 0x03)
                {
                case 0:
                    *fft_mode = MODE_2K;
                    break;
                case 1:
                    *fft_mode = MODE_8K;
                    NIM_MT353_PRINTF("MODE 1_8\n");
                    break;
                default:
                    *fft_mode = 0xff; /* error */
                    break;
                }
            }
        }
       NIM_MT353_PRINTF(" fft_mode =%d\n", *fft_mode );
    return SUCCESS;
    #endif
}


/*****************************************************************************
* INT32 f_mt353_get_modulation(struct nim_device *dev, UINT8 *modulation)
* Description: Read mt353 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* modulation
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_mt353_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
#if 1
    *modulation = MT353_CurChannelInfo->Modulation;
    return SUCCESS;

#else

    UINT8    data[2];
    UINT32    flgptn;
    OSAL_ER    result;


        result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
            NIM_MT353_PRINTF("Get modulation Fail, Wait ChannelChg Complete!\n");
        *modulation=0;
    }


    else
    {
        f_mt353_read(dev->base_addr, REG353_TPSRECEIVED ,data,2);
            if(0x80==(data[0]&0x80))
            {
                data[0]=(data[0]&0x60)>>5;
                NIM_MT353_PRINTF("CONST data=%d\n",data[0]);
                switch (data[0])
                    {
                case 0:
                    * modulation = TPS_CONST_QPSK;
                    break;
                case 1:
                    * modulation = TPS_CONST_16QAM;
                    NIM_MT353_PRINTF("mod 1_16\n");
                    break;
                case 2:
                    * modulation = TPS_CONST_64QAM;
                    break;
                
                default:
                    * modulation = 0xff; /* error */
                    break;
            }
            NIM_MT353_PRINTF("modulation =%d\n",* modulation );
            }
        }

    return SUCCESS;
#endif
}

/*****************************************************************************
* INT32 f_mt353_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    UINT8    data;
    UINT32    flgptn;
    OSAL_ER    result;

    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
            //NIM_PRINTF("Get Spectrum Inv Fail, Wait ChannelChg Complete!\n");
        *Inv=0;
    }
    else
    {
        f_mt353_read(dev->base_addr, REG353_STATUS_0  ,&data,1);
        *Inv=((data&0x80)>>7);// 1:inv 0:non-inv
        //soc_printf("Spectral_Inv=0x%x\n",    *Inv);
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_mt353_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_AGC(struct nim_device *dev, UINT16 *agc)
{
     UINT8        data[2];
     UINT16        agc_if=0;
     OSAL_ER    result;
     UINT32        flgptn;
      UINT16    agc_min,agc_max;
    struct nim_mt353_private *dev_priv;

    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("AGC result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get AGC Fail, Wait ChannelChg Complete!\n");
        *agc = 0;
    }
    else
    {  // *agc=1300;
         if( MT353_CurChannelInfo->lock_status==0)
         {
             *agc = 0;
             return SUCCESS;
        }

        dev_priv = dev->priv;

        f_mt353_read(dev->base_addr, REG353_AGCGAIN  ,data, 2);
        agc_if = ((data[0]&0x3f)<<8 | data[1]);
        //*agc=agc_if/10;//Sam 20050722
        //soc_printf("    agc_if=%d \n",agc_if);
             //td1611
        agc_max =16000;//(*(dev_priv->Tuner_Control.config_data.ptMT353 + 4)<<8) | *(dev_priv->Tuner_Control.config_data.ptMT353 + 5);

        agc_min = 4700;//(*(dev_priv->Tuner_Control.config_data.ptMT353 + 6)<<8) | *(dev_priv->Tuner_Control.config_data.ptMT353 + 7);

//        *agc = (agc_if-agc_min)*2000/(agc_max-agc_min)+1000;
        if (agc_max >= agc_min)//060126 for agc inverse
            {
            if (agc_if > agc_max)//weak
            {
                agc_if = agc_max;
                *agc =10;
                return SUCCESS;
            }
            else if(agc_if < agc_min)//stonge
            {
                agc_if = agc_min;
                *agc =100;
                return SUCCESS;
            }

//            *agc = (agc_if-agc_min)*1900/(agc_max-agc_min)+1100;
            *agc =100-(agc_if-agc_min)*100/(agc_max-agc_min);//now :level inverse agc_if

            }
        else
            {
            if (agc_if < agc_max)
                agc_if = agc_max;
            else if(agc_if > agc_min)
                agc_if = agc_min;


            agc_if = agc_min - agc_if + agc_max;
//            *agc = (agc_if-agc_max)*1900/(agc_min-agc_max)+1100;
            *agc = (agc_if-agc_max)*100/(agc_min-agc_max);
            }
    }

    return SUCCESS;




}

/*****************************************************************************
* INT32 f_mt353_get_SNR(struct nim_device *dev, UINT8 *snr)
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
static INT32 f_mt353_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    UINT8    data;
    OSAL_ER result;
    UINT32     flgptn;


    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("SNR result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get SNR Fail, Wait ChannelChg Complete!\n");
        *snr = 0;
    }
    else
    {    //*snr=160;
        f_mt353_read(dev->base_addr,  REG353_SNR,&data, 1);
        //*snr=data; //dB*8
        if (MT353_CurChannelInfo->Modulation== TPS_CONST_64QAM)
        {
        if (data > 0xc0)
            {
            if (data > 0xcb)
            data = 0xcb;
                *snr = (data-0xc0)*30/(0xcb-0xc0)+70;    //200~140
            }
        else if (data > 0x9a)
                *snr = (data-0x9a)*30/(0xc0-0x9a)+ 40;    //140~80
            else
            {
                if  (data < 0x30)
                    data = 0x30;
                *snr = (data-0x30)*40/(0x9a-0x30);    //80~0
            }
        }
        else if (MT353_CurChannelInfo->Modulation== TPS_CONST_16QAM)
        {
            if (data > 0xc0)
            {
                if (data > 0xd5)
                data = 0xd5;
                *snr = (data-0xc0)*30/(0xd5-0xc0)+70;    //200~140
            }
            else if (data > 0x66)
                *snr = (data-0x66)*30/(0xc0-0x66)+ 40;    //140~80
        else
            {
            if  (data < 0x30)
                data = 0x30;
                *snr = (data-0x30)*40/(0x66-0x30);    //80~0
            }
        }
        else //(MT353_CurChannelInfo->Modulation== TPS_CONST_QPSK)
        {
            if (data > 0xc0)
            {
                if (data > 0xd5)
                    data = 0xd5;
                *snr = (data-0xc0)*30/(0xd5-0xc0)+70;    //200~140
            }
            else if (data > 0x38)
                *snr = (data-0x38)*30/(0xc0-0x38)+ 40;    //140~80
            else
            {
                if  (data < 0x18)
                    data = 0x18;
                *snr = (data-0x18)*40/(0x38-0x18);    //80~0
            }
        }
        NIM_PRINTF("SNR=%d\n",*snr);
    }

        return SUCCESS;
}


/*****************************************************************************
* INT32 f_mt353_get_BER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    UINT32 dwPeriod=0,dwCount=0;
    UINT32 temp,temp1;
    UINT32 ber;
    INT32 i;
    UINT8 data[2];
    OSAL_ER result;
    UINT32     flgptn;


    result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("BER result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get SNR Fail, Wait ChannelChg Complete!\n");
        *vbber = 0;
    }
    else
    {

        f_mt353_read(dev->base_addr,REG353_STATUS_1 , data, 1);
        if ((data[0] & MT353_FECSTAT_LOCKED)!=MT353_FECSTAT_LOCKED)
            {
            *vbber = 0;
            return SUCCESS;
            }


        f_mt353_read(dev->base_addr,  REG353_STATUS_0,data, 1);
        if (data[0] & 0x20 != 0x20)
            {
            *vbber = 0;
            return SUCCESS;
            }
        f_mt353_read(dev->base_addr,  0x01,data, 1);
        if (data[0] & 0x80 != 0x80)
            {
            *vbber = 0;
            return SUCCESS;
            }

        for(i = 0; i < 10; i++)
            {
            f_mt353_read(dev->base_addr,  REG353_RSUBC,data, 2);

            if (data[0]==0)
                break;
            NIM_MT353_PRINTF("REG353_RSUBC = %x \n",data[0]);

            }
        if(i ==10)
            {
            *vbber = 0;
            return SUCCESS;
            }



        f_mt353_read(dev->base_addr,  REG353_RSERRPER,data, 2);

        dwPeriod = (data[0] <<8) | data[1];

        f_mt353_read(dev->base_addr,  REG353_RSBERCNT,data, 3);

        dwCount = (data[0] <<16) | (data[1] <<8) | data[2];

        //BER = dwCount/(dwPeriod x 1024 x 1632)

        NIM_MT353_PRINTF("dwPeriod =  %d    dwCount = %d \n",dwPeriod,dwCount);



        ber = dwCount/dwPeriod ;
        ber *= 100000;
        ber /= 1632;
        //ber *= 1000;
        ber/=1024;
        NIM_MT353_PRINTF("ber=%d\n",ber);

        if (ber >= 10000)
            {
            ber = 10000;
//            return SUCCESS;
            }

        temp = 100000 - ber;
        NIM_MT353_PRINTF("te00000=%d\n",temp);

        temp1 =10000000000/ (temp);
        NIM_MT353_PRINTF("te11111=%d\n",temp1);

        temp = (temp1-100000)*(temp1+100000);
        NIM_MT353_PRINTF("te2222=%d\n",temp);
        if (temp == 0)
        {
    //        *snr = 190;
            *vbber = 100;
            return SUCCESS;
        }
        else
            {
            temp1 = 10000000000/temp;
            NIM_MT353_PRINTF("te3333=%d\n",temp1);

            }

        for (i = 16; i>0; i--)
        {
            if (temp1 & (1<<i))
            {
                temp = ((temp1) / (1<<i));
                temp += i;
                temp *= 3;
                break;
            }
        }

        *vbber =  2*( temp);
    }
    NIM_MT353_PRINTF("vbber = %d\n",*vbber);
    return SUCCESS;
}




static INT32 f_mt353_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)    //50622-01Angus
{
    static UINT32 rec_ber_cnt = 0;
    nim_rec_performance_t * p_nim_rec_performance;
    INT32 ret_val = ERR_FAILUE;

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            mt353_autoscan_stop_flag = param;
        break;

        case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
            p_nim_rec_performance = (nim_rec_performance_t *)param;
            f_mt353_get_lock(dev, &(p_nim_rec_performance->lock));

            if (p_nim_rec_performance->lock == 1)
            {
                if (rec_ber_cnt !=mt353_per_tot_cnt)
                {
                    rec_ber_cnt = mt353_per_tot_cnt;
                    p_nim_rec_performance->ber = mt353_ber;
                    p_nim_rec_performance->per = mt353_per;
                    p_nim_rec_performance->valid = TRUE;
                }
                else
                {
                    p_nim_rec_performance->valid = FALSE;
                }
            }

            ret_val = SUCCESS;

            break;    

        default:
        break;
    }
    return SUCCESS;
}

void f_mt353_tracking_task(struct nim_device *dev)
{
    UINT32     flgptn;
    UINT8 data[1];
    TMO     start_time, temp_time;


    while(1)
    {
        osal_flag_wait(&flgptn,nim_353_flag, NIM_353_CHECKING, OSAL_TWF_ANDW,OSAL_WAIT_FOREVER_TIME);
        osal_flag_clear(nim_353_flag,NIM_353_CHECKING);


//        f_mt353_channel_change(dev,  MT353_CurChannelInfo->Frequency, MT353_CurChannelInfo->ChannelBW,
//                    MT353_CurChannelInfo->Guard, MT353_CurChannelInfo->Mode, MT353_CurChannelInfo->Modulation, MT353_CurChannelInfo->FECRates, MODE_CHECKING, 0);

        osal_flag_clear(nim_353_flag,NIM_353_SCAN_END);

        data[0]=0x10 ;
        f_mt353_write(dev->base_addr,REG353_TUNERGO,data,1);

        start_time = osal_get_tick();
        while(1)
        {
            f_mt353_read(dev->base_addr,REG353_STATUS_0 , data, 1);
            if ((data[0] & MT353_OFDM_LOCK_FLAG) == MT353_OFDM_LOCK_FLAG)
            {
                break;
            }
            osal_task_sleep(20);

            temp_time = osal_get_tick();

            if(temp_time >= start_time ?  ( temp_time - start_time > 512) : (start_time - temp_time < 0xFFFFFFFF - 512))
            {
                osal_flag_set(nim_353_flag, NIM_353_CHECKING);
                break;
            }
        }
        osal_flag_set(nim_353_flag,NIM_353_SCAN_END);
        osal_task_sleep(50);
    }
}


static INT32 f_mt353_tracking_task_init(struct nim_device *dev)
{
    ER    ret_val;
    OSAL_T_CTSK    t_ctsk;

#if (SYS_SDRAM_SIZE == 2)
    t_ctsk.stksz    = 0x200;
#else
    t_ctsk.stksz    = 0x400;
#endif
    t_ctsk.quantum= 10;
    t_ctsk.itskpri    = OSAL_PRI_NORMAL;
    t_ctsk.task = (FP)f_mt353_tracking_task;
    t_ctsk.para1 = (UINT32)dev;

    f_mt353_tracking_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == f_mt353_tracking_task_id)
    {
        NIM_PRINTF("create_task nim_mt353_tracking_task_id failed\n");
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************
* INT32 f_mt353_get_freq_offset(struct nim_device *dev,INT32 *freq_offset)
* Description: Read mt353 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: INT32 *freq_offset
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_mt353_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)//051222 yuchun
{
        OSAL_ER    result;
        UINT32    flgptn;


        result = osal_flag_wait(&flgptn,nim_353_flag, NIM_353_SCAN_END, OSAL_TWF_ANDW,0);
       // libc_printf("offset result = %d\r\n",result);

    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get guard_interval Fail, Wait ChannelChg Complete!\n");
//        *guard_interval=0;
        *freq_offset=0xffffffff;//050725 yuchun
    }
        else
        {
            f_mt353_freq_offset(dev, MT353_CurChannelInfo->ChannelBW, MT353_CurChannelInfo->Mode, freq_offset);
       }

//       soc_printf(" offset =%d\n", *freq_offset );
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_mt353_hier_mode(struct nim_device *dev)
* Description: Read mt353 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_hier_mode(struct nim_device *dev)
{
UINT8 data;
UINT8 hier;

 f_mt353_read(dev->base_addr, REG353_TPSRECEIVED ,&data,1);

    data=(data&TPS_HIERMODE)>>2;
    switch(data)
    {
        case 0 : hier=HIER_NONE;
        break;
        case 1 :hier=HIER_1;
        break;
        case 2 : hier=HIER_2;
        break;
        case 3 : hier=HIER_4;
        break;
        default :
                hier=HIER_NONE;
        break; /* error */
    }
        return hier;
}


/*****************************************************************************
* INT32 f_mt353_get_hier_mode(struct nim_device *dev,UINT8*hier)
* Description: Read mt353 hier_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8*hier
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_mt353_get_hier_mode(struct nim_device *dev, UINT8*hier)
{
 *hier= MT353_CurChannelInfo->Hierarchy;
     //soc_printf(" hier a %d\n", *hier);
return SUCCESS;
}

/*****************************************************************************
* INT32 f_mt353_priority(struct nim_device *dev,UINT8*priority)
* Description: Read mt353 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: UINT8*priority
*
* Return Value: INT32
*****************************************************************************/

INT8 f_mt353_priority(struct nim_device *dev, UINT8*priority)
{

*priority= MT353_CurChannelInfo->Priority;
return SUCCESS;

}


/*****************************************************************************
* void f_mt353_set_gpio(struct nim_device *dev, UINT8 pin_num, BOOL polar)

* Description: Support mt353 gpio confi
*  Arguments:
*  Parameter1: struct nim_device *dev,UINT8 pin_num, BOOL polar
  Parameter2: UINT8*priority
*
* Return Value: INT32
*****************************************************************************/
void f_mt353_set_gpio(struct nim_device *dev, UINT8 pin_num, BOOL polar)
{
 UINT8 data[2];

     switch(pin_num)
    {
        case 35 : data[0]=0x10|polar;
        break;
        case 36 : data[0]=0x20|(polar<<1);
        break;
        case 41 : data[0]=0x40|(polar<<2);
        break;
        case 43 : data[0]=0x80|(polar<<3);
        break;
        default :
                data[0]=0x80|(polar<<3);   //GPP_3
        break; /* error */
    }

    f_mt353_write(dev->base_addr,0x63 ,data,1);


}
#endif
