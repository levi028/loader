/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    rfm_mcbs373.c
*
*    Description:    This file contains all functions definition
*                     of MCBS373 RF modulator driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Dec.13.2004       Justin Wu      Ver 0.1    Create file.
*    2.
*****************************************************************************/


#include <retcode.h>
#include <types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal_timer.h>
#include <osal/osal_int.h>
#include <osal/osal_mm.h>

#include <hld/hld_dev.h>
#include <hld/rfm/rfm.h>
#include <hld/rfm/rfm_dev.h>
#include <bus/i2c/i2c.h>


#define RFM_MCBS373_DEFAULT_SYSTEM            RFM_SYSTEM_PAL_DK
#define RFM_MCBS373_BASE_CHANNEL            21


static INT32 rfm_mcbs373_open(struct rfm_device *dev);
static INT32 rfm_mcbs373_close(struct rfm_device *dev);
static INT32 rfm_mcbs373_system_set(struct rfm_device *dev, UINT32 system);
static INT32 rfm_mcbs373_channel_set(struct rfm_device *dev, UINT16 channel);

/* Name for RF modulator, the last character must be Number for index */
static char rfm_mcbs373_name[HLD_MAX_NAME_SIZE] = "RFM_MCBS373_0";
static char rfm_373_374_mask = 0;
#ifndef DVBC_INDIA
/*
 *     Name        :   rfm_mcbs373_init()
 *    Description    :   MCBS373 init funciton should be called in system boot up.
 *    Return        :    INT32                : return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 rfm_mcbs373_init()
{
    struct rfm_device *dev;

    dev = (struct rfm_device *)dev_alloc(rfm_mcbs373_name, HLD_DEV_TYPE_RFM, sizeof(struct rfm_device));
    if (dev == NULL)
    {
        PRINTF("Error: Alloc RF modulator device error!\n");
        return ERR_NO_MEM;
    }
    dev->base_addr = SYS_RFM_BASE_ADDR;
    dev->system = RFM_MCBS373_DEFAULT_SYSTEM;
    dev->channel = RFM_MCBS373_BASE_CHANNEL;

    dev->priv = NULL;

    /* Function point init */
    dev->init = rfm_mcbs373_init;
    dev->open = rfm_mcbs373_open;
    dev->stop = rfm_mcbs373_close;
    dev->do_ioctl = NULL;
    dev->system_set = rfm_mcbs373_system_set;
    dev->channel_set = rfm_mcbs373_channel_set;
    dev->get_stats = NULL;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        PRINTF("Error: Register RF modulator device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }

    return SUCCESS;
}

/*
* Name          :   rfm_mcbs373_open()
* Description   :   Open MCBS373 RF modulator
* Parameter     :   struct rfm_device *dev
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_open(struct rfm_device *dev)
{
    UINT8 data[4];
    UINT32 n;
    INT32 result;

    /* Reset RFM to default status and make it power save off */
    switch (dev->system)
    {
    case RFM_SYSTEM_NTSC_M:    /* NTSC_M, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x00;
        dev->step = 6;
        dev->base_ch = 14;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_BG:    /* PAL_BG, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x08;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_I:    /* PAL_I, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x10;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_DK:    /* PAL_DK, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x18;
        dev->step = 8;
        dev->base_ch = 13;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_L:
    default:                /* PAL_BG, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x08;
        break;
    }
    n = (((RFM_MCBS373_BASE_CHANNEL - dev->base_ch) * dev->step + dev->base_freq) << 2)+1;
    data[2] = ((n >> 6) & 0x3f);
    data[3] = ((n << 2) & 0xfc);

    if(dev->lo_low==0)
        data[0] |= 0x10;/*  LOP high */
#ifdef I2C_FOR_RFM
  if(!dev->i2c_type_id_cfg)
      dev->i2c_type_id = I2C_FOR_RFM;
#endif
    //soc_printf("%s: %02x %02x %02x %02x\n", __FUNCTION__, data[0], data[1], data[2], data[3]);
    result = i2c_write(dev->i2c_type_id,dev->base_addr, data, 4) ;
    if(result != SUCCESS)
        return result;
    /* for the difference between 373 and 374, OSC must be changed
        OSC  0: 373
                1:  374
        read OOR to detect the device type.
    */
    result = i2c_read(dev->i2c_type_id,dev->base_addr,data,1);
    if(result != SUCCESS)
        return result;
    if((data[0]&0x01) == 0x01)/* VCO out of range, device type is 374 */
        rfm_373_374_mask = 0x40;
    return SUCCESS;
}

/*
* Name          :   rfm_mcbs373_close()
* Description   :   Close MCBS373 RF modulator
* Parameter     :   struct rfm_device *dev
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_close(struct rfm_device *dev)
{
    UINT8 data[2];

    /* Make RFM power save on */
    data[0] = 0xa0;
    data[1] = 0x60;

    return i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
}


/*
* Name          :   rfm_mcbs373_system_set()
* Description   :   Set output system mode
* Parameter     :   struct rfm_device *dev
*                    UINT32 system            : System mode
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_system_set(struct rfm_device *dev, UINT32 system)
{
    UINT8 data[2];
    INT32 result;
    dev->system = system;
    /* Make RFM power save on */
    switch (system)
    {

    case RFM_SYSTEM_NTSC_M:    /* NTSC_M, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x00|rfm_373_374_mask;
        dev->step = 6;
        dev->base_ch = 14;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_BG:    /* PAL_BG, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x08|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_I:    /* PAL_I, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x10|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_DK:    /* PAL_DK, White clip on, LOP low */
        data[0] = 0x80;
        data[1] = 0x18|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 13;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_L:
    default:                /* PAL_BG, White clip on, LOP low */
        return ERR_FAILUE;
        break;
    }

    if(dev->lo_low==0)
        data[0] |= 0x10;/*  LOP high */
    //soc_printf("%s: %02x %02x\n", __FUNCTION__, data[0], data[1]);
    result = i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
    //libc_printf("rfm_mcbs373_system_set, system: %d, result: %x\n",system,result);
    return result;
}

/*
* Name          :   rfm_mcbs373_channel_set()
* Description   :   Set output channel, this RFM support CCIR Ch2 ~ Ch69
* Parameter     :   struct rfm_device *dev
*                    UINT16 channel            : Channel No.
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_channel_set(struct rfm_device *dev, UINT16 channel)
{
    UINT8 data[2];
    UINT32 n;
    UINT16 tmp = RFM_MCBS373_BASE_CHANNEL;
    INT32 result;

    switch (dev->system)
    {
    case RFM_SYSTEM_L:    /* needn't support currently */
        break;
    case RFM_SYSTEM_NTSC_M:    /* ch 14-83 */
        if(channel > 83)
            tmp = 83;
        else if(channel < 14)
            tmp = 14;
        else
            tmp = channel;
        break;
    case RFM_SYSTEM_PAL_BG:    /* 21-69 */
    case RFM_SYSTEM_PAL_I:
        if(channel > 69)
            tmp = 69;
        else if(channel < 21)
            tmp = 21;
        else
            tmp = channel;
        break;
    case RFM_SYSTEM_PAL_DK:    /* 13-57 */
        if(channel > 57)
            tmp = 57;
        else if(channel < 13)
            tmp = 13;
        else
            tmp = channel;
        break;
    default:
        break;
    }
    n = (((tmp - dev->base_ch) * dev->step + dev->base_freq) << 2)+1;
    PRINTF("base_ch = %d,step = %d,base_freq = %d\nch = %d,Freq = %d\n",dev->base_ch,dev->step,dev->base_freq,tmp,n);
    data[0] = ((n >> 6) & 0x3f);
    data[1] = ((n << 2) & 0xfc);
    //soc_printf("%s: %02x %02x\n", __FUNCTION__, data[0], data[1]);
    result=i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
    //libc_printf("rfm_mcbs373_channel_set, chn: %d,result: %x\n",channel,result);
    return result;
}
#else
/*
 *     Name        :   rfm_mcbs373_init()
 *    Description    :   MCBS373 init funciton should be called in system boot up.
 *    Return        :    INT32                : return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 rfm_mcbs373_init()
{
    struct rfm_device *dev;

    dev = dev_alloc(rfm_mcbs373_name, HLD_DEV_TYPE_RFM, sizeof(struct rfm_device));
    if (dev == NULL)
    {
        PRINTF("Error: Alloc RF modulator device error!\n");
        return ERR_NO_MEM;
    }
    dev->base_addr = 0xCA;
    dev->system = RFM_MCBS373_DEFAULT_SYSTEM;
    dev->channel = RFM_MCBS373_BASE_CHANNEL;

    dev->priv = NULL;

    /* Function point init */
    dev->init = rfm_mcbs373_init;
    dev->open = rfm_mcbs373_open;
    dev->stop = rfm_mcbs373_close;
    dev->do_ioctl = NULL;
    dev->system_set = rfm_mcbs373_system_set;
    dev->channel_set = rfm_mcbs373_channel_set;
    dev->get_stats = NULL;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        PRINTF("Error: Register RF modulator device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }

    return SUCCESS;
}

/*
* Name          :   rfm_mcbs373_open()
* Description   :   Open MCBS373 RF modulator
* Parameter     :   struct rfm_device *dev
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_open(struct rfm_device *dev)
{
    UINT8 data[4];
    UINT32 n;
    INT32 result;

    /* Reset RFM to default status and make it power save off */
    switch (dev->system)
    {
    case RFM_SYSTEM_NTSC_M:    /* NTSC_M, White clip on, LOP low */
        data[0] = 0x88;
        data[1] = 0x00;
        dev->step = 6;
        dev->base_ch = 14;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_BG:    /* PAL_BG, White clip on, LOP low */
        data[0] = 0x88;
        data[1] = 0x08;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_I:    /* PAL_I, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x10;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_DK:    /* PAL_DK, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x18;
        dev->step = 8;
        dev->base_ch = 13;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_L:
    default:                /* PAL_BG, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x08;
        break;
    }
    n = (((RFM_MCBS373_BASE_CHANNEL - dev->base_ch) * dev->step + dev->base_freq) << 2);
    data[2] = ((n >> 6) & 0x3f);
    data[3] = ((n << 2) & 0xfc);

//    if(dev->lo_low==0)
//        data[0] |= 0x10;/*  LOP high */
#ifdef I2C_FOR_RFM
  if(!dev->i2c_type_id_cfg)
      dev->i2c_type_id = I2C_FOR_RFM;
#endif
   result = i2c_write(dev->i2c_type_id,dev->base_addr, data, 4) ;
    if(result != SUCCESS)
        return result;
    /* for the difference between 373 and 374, OSC must be changed
        OSC  0: 373
                1:  374
        read OOR to detect the device type.
    */
    result = i2c_read(dev->i2c_type_id,dev->base_addr,data,1);
    if(result != SUCCESS)
        return result;

    if(data[0]&0x01 == 0x01)/* VCO out of range, device type is 374 */
        rfm_373_374_mask = 0x40;
    //#if(SYS_MAIN_BOARD==BOARD_M3329C_KAON00)
    if(data[0] == 0xf8)
    rfm_373_374_mask = 0x40;
    //#endif
    return SUCCESS;
}

/*
* Name          :   rfm_mcbs373_close()
* Description   :   Close MCBS373 RF modulator
* Parameter     :   struct rfm_device *dev
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_close(struct rfm_device *dev)
{
    UINT8 data[2];

    /* Make RFM power save on */
    data[0] = 0xa0;
    data[1] = 0x20;

    return i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
}


/*
* Name          :   rfm_mcbs373_system_set()
* Description   :   Set output system mode
* Parameter     :   struct rfm_device *dev
*                    UINT32 system            : System mode
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_system_set(struct rfm_device *dev, UINT32 system)
{
    UINT8 data[2];
    INT32 result;
    dev->system = system;
    /* Make RFM power save on */
    switch (system)
    {

    case RFM_SYSTEM_NTSC_M:    /* NTSC_M, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x00|rfm_373_374_mask;
        dev->step = 6;
        dev->base_ch = 14;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_BG:    /* PAL_BG, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x08|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_I:    /* PAL_I, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x10|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 21;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_PAL_DK:    /* PAL_DK, White clip on, LOP low */
        data[0] = 0X88;
        data[1] = 0x18|rfm_373_374_mask;
        dev->step = 8;
        dev->base_ch = 13;
        dev->base_freq = 471;
        break;
    case RFM_SYSTEM_L:
    default:                /* PAL_BG, White clip on, LOP low */
        return ERR_FAILUE;
        break;
    }

//    if(dev->lo_low==0)
//        data[0] |= 0x10;/*  LOP high */

    result = i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
    //libc_printf("rfm_mcbs373_system_set, system: %d, result: %x\n",system,result);
    return result;
}

/*
* Name          :   rfm_mcbs373_channel_set()
* Description   :   Set output channel, this RFM support CCIR Ch2 ~ Ch69
* Parameter     :   struct rfm_device *dev
*                    UINT16 channel            : Channel No.
* Return        :   SUCCESS
*/
static INT32 rfm_mcbs373_channel_set(struct rfm_device *dev, UINT16 channel)
{
    UINT8 data[2];
    UINT32 n;
    UINT16 tmp = RFM_MCBS373_BASE_CHANNEL;
    INT32 result;

    switch (dev->system)
    {
    case RFM_SYSTEM_L:    /* needn't support currently */
        break;
    case RFM_SYSTEM_NTSC_M:    /* ch 14-83 */
        if(channel > 83)
            tmp = 83;
        else if(channel < 14)
            tmp = 14;
        else
            tmp = channel;
        break;
    case RFM_SYSTEM_PAL_BG:    /* 21-69 */
    case RFM_SYSTEM_PAL_I:
        if(channel > 69)
            tmp = 69;
        else if(channel < 21)
            tmp = 21;
        else
            tmp = channel;
        break;
    case RFM_SYSTEM_PAL_DK:    /* 13-57 */
        if(channel > 57)
            tmp = 57;
        else if(channel < 13)
            tmp = 13;
        else
            tmp = channel;
        break;
    default:
        break;
    }
    n = (((tmp - dev->base_ch) * dev->step + dev->base_freq) << 2);
    PRINTF("base_ch = %d,step = %d,base_freq = %d\nch = %d,Freq = %d\n",dev->base_ch,dev->step,dev->base_freq,tmp,n);
    data[0] = ((n >> 6) & 0x3f);
    data[1] = ((n << 2) & 0xfc);

    result=i2c_write(dev->i2c_type_id,dev->base_addr, data, 2);
    //libc_printf("rfm_mcbs373_channel_set, chn: %d,result: %x\n",channel,result);
    return result;
}
#endif

