/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scart_mx9671.c
*
* Description:
*     scart chip mx9671 driver API implementation for DVBapplication.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <retcode.h>
#include <types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/scart/scart.h>
#include <bus/i2c/i2c.h>
#include "scart_mx9671.h"

/*******************************************************
* macro define
********************************************************/

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
static char scart_mx9671_name[HLD_MAX_NAME_SIZE]= "SCART_MX9671_0";
static UINT16 g_mx_volume = 0;

/*******************************************************
* local function declare
********************************************************/
static INT32 scart_mx9671_detach(struct scart_device *dev);

static void scart_mx9671_tv_mode(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    reg_offset = 7;
    if(TV_MODE_RGB == param)
    {
        reg_tgt_val = 3<<3;
    }
    else if(TV_MODE_CVBS == param)
    {
        reg_tgt_val = 0<<3;
    }
    else
    {
        reg_tgt_val = 2<<3;
    }
    tp->reg_val[reg_offset] &= (~(0x03<<3));
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;
}

static void scart_mx9671_tv_aspect(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    reg_offset = 7;
    if(ASPECT_4_3 == param)
    {
        reg_tgt_val = 3;
    }
    else if(ASPECT_16_9 == param)
    {
        reg_tgt_val = 1;
    }
    else
    {
        reg_tgt_val = 0;
    }
    tp->reg_val[reg_offset] &= (~0x03);
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;
}

static void scart_mx9671_vcr_aspect(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    reg_offset = 9;
    if(ASPECT_4_3 == param)
    {
        reg_tgt_val = 3;
    }
    else if(ASPECT_16_9 == param)
    {
        reg_tgt_val = 1;
    }
    else
    {
        reg_tgt_val = 0;
    }
    tp->reg_val[reg_offset] &= (~0x03);
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;
}

static void scart_mx9671_tv_source(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;
    UINT8 tmp_buf[4] = {0};

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    if(SOURCE_STB_IN == param)
    {
        reg_offset = 1;//Audio
        reg_tgt_val = 0;
        tp->reg_val[reg_offset] &= (~0x3);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 6;//video
        reg_tgt_val = 0;
        tp->reg_val[reg_offset] &= (~0x1f);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
    else if(SOURCE_VCR_IN == param)
    {
        reg_offset = 1;//audio
        reg_tgt_val = 0x01;
        tp->reg_val[reg_offset] &= (~0x03);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 6;//video
        reg_tgt_val = 0x02|0x08;
        tp->reg_val[reg_offset] &= (~0x1f);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        tmp_buf[0] = 0x0e;
        i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf, 1, 1);

        tmp_buf[0] = (tmp_buf[0] & 0x0C) >> 2;

        reg_offset = 7;//Set TV FS
        //reg_tgt_val = 0x12;/////| tmp_buf[0];
        reg_tgt_val = 0x10 | tmp_buf[0];
        tp->reg_val[reg_offset] &= ~0x1B;////(~(0x3<<3));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

    }
    else
    {
        reg_offset = 1;//Audio
        reg_tgt_val = 2;
        tp->reg_val[reg_offset] &= (~0x3);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 6;//video
        reg_tgt_val = 0x03|0x10;
        tp->reg_val[reg_offset] &= (~0x1f);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static void scart_mx9671_vcr_source(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    if(SOURCE_STB_IN == param)
    {
        reg_offset = 1;//Audio
        reg_tgt_val = 0<<2;
        tp->reg_val[reg_offset] &= (~(0x3<<2));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 8;//vcr video only cvbs.RGB only use input function
        reg_tgt_val = 0;
        tp->reg_val[reg_offset] &= (~0x7);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

    }
    else if(SOURCE_TV_IN == param)
    {
        reg_offset = 1;//Audio
        reg_tgt_val = 2<<2;
        tp->reg_val[reg_offset] &= (~(0x3<<2));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 8;//video
        reg_tgt_val = 3;
        tp->reg_val[reg_offset] &= (~(0x7));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
    else
    {
        reg_offset = 1;//Audio
        reg_tgt_val = 1<<2;
        tp->reg_val[reg_offset] &= (~(0x3<<2));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 8;//video
        reg_tgt_val = 2;
        tp->reg_val[reg_offset] &= (~(0x7));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static void scart_mx9671_audio_mute(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    reg_offset = 2;
    tp->reg_val[reg_offset] &= ~0x01;
    if(param)
    {
        reg_tgt_val = 0;
    }
    else
    {
        reg_tgt_val = 1;
    }
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;
}

static void scart_mx9671_enter_standby(struct scart_device *dev, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if(NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    reg_offset = 16;
    if(0 == param)
    {
        reg_tgt_val = 1<<6;
    }
    else
    {
        reg_tgt_val = 3<<6;
    }
    tp->reg_val[reg_offset] &= (~(0x3<<6));
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;
}

static INT32 scart_mx9671_io_control(struct scart_device *dev, INT32 cmd, UINT32 param)
{
    struct scart_mx9671_private *tp = NULL;
    RET_CODE rlt = RET_FAILURE;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;
    UINT8 tmp_buf[4] = {0};
    UINT8 write = 0;

    if((NULL == dev)|| (UINT_MAX == param))
    {
        ASSERT(0);
        return rlt;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    if (NULL == tp)
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    MEMSET(tp->reg_dirty_list, 0, sizeof(tp->reg_dirty_list));
    switch(cmd)
    {
    case SCART_TV_MODE:
        scart_mx9671_tv_mode(dev, param);
        write = 1;
        break;
    case SCART_TV_ASPECT:
        scart_mx9671_tv_aspect(dev, param);
        write = 1;
        break;
    case SCART_VCR_ASPECT:
        scart_mx9671_vcr_aspect(dev, param);
        write = 1;
        break;
    case SCART_TV_SOURCE:
        scart_mx9671_tv_source(dev, param);
        write = 1;
        break;
    case SCART_VCR_SOURCE:
        scart_mx9671_vcr_source(dev, param);
        write = 1;
        break;
    case SCART_CHK_STATE:
        write = 0;
        if(param)
        {
            *((UINT32 *)param) = 0;
            tmp_buf[0] = 0x0e;
            i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf, 1, 1);

            tmp_buf[1] = 0x0f;
            i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf+1, 1, 1);

            tmp_buf[2] = 0x07;
            i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf+2, 1, 1);

            if(tmp_buf[0]&0x04)
            {
                *((UINT32 *)param) |= SCART_STATE_VCR_IN;
                if((tmp_buf[2] & 0x03) != ((tmp_buf[0] & 0x0C)>>2))
                {
                    reg_offset = 7;
                    reg_tgt_val = ((tmp_buf[0] & 0x0C)>>2);
                    tp->reg_val[reg_offset] &= ~0x3;
                    tp->reg_val[reg_offset] |= reg_tgt_val;
                    (tp->reg_dirty_list)[reg_offset] = 1;
                    write = 1;
                }
            }
            if(tmp_buf[0]&0x01)
            {
                *((UINT32 *)param) |= SCART_STATE_TV_IN;
            }
        }
        break;
    case SCART_AUDIO_MUTE:
        scart_mx9671_audio_mute(dev, param);
        write = 1;
        break;
    case SCART_ENTRY_STADNBY:
        scart_mx9671_enter_standby(dev, param);
        write = 1;
        break;
    case SCART_DETACH_DEV:
        write = 0;
        rlt = scart_mx9671_detach(dev);
        break;
    default:
        rlt = ERR_FAILUE;
        break;
    }
    if(RET_SUCCESS == rlt)
    {
        if(write)
        {
            for(reg_offset = 0; reg_offset<MX_MAX_REG_LEN; reg_offset++)
            {
                if((tp->reg_dirty_list)[reg_offset])
                {
                    tmp_buf[0] = reg_offset;
                    tmp_buf[1] = tp->reg_val[reg_offset];
                    i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);
                }
            }
        }
    }
    return rlt;
}

static INT32 scart_mx9671_close(struct scart_device *dev)
{
    UINT8 tmp_buf[MX_MAX_REG_LEN] = {0};

    if(NULL == dev)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    tmp_buf[0] = 16;
    tmp_buf[1] = 0x40;

    i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);
    //scart_mx9671_dump_reg(dev);
    return RET_SUCCESS;
}

static INT32 scart_mx9671_open(struct scart_device *dev)
{
    struct scart_mx9671_private *tp = NULL;
    UINT8 tmp_buf[16] = {0, };
    UINT16 lsr_id = 0;
    INT32  ret = RET_SUCCESS;

    if(NULL == dev)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    tp = (struct scart_mx9671_private *)dev->priv;
    tmp_buf[0] = 0x10;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 16

    tmp_buf[0] = 0x00;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 0

    tmp_buf[0] = 0x01;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 1

    tmp_buf[0] = 0x06;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 6

    tmp_buf[0] = 0x07;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 7

    tmp_buf[0] = 0x08;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 8

    tmp_buf[0] = 0x09;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 9

    tmp_buf[0] = 0x0D;
    tmp_buf[1] = tp->reg_val[tmp_buf[0]];
    ret = i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 13

    if(dev->vcr_callback != (UINT32)NULL)
    {/*If register callback,using interrupt way to detect VCR plug in-out*/
        HAL_GPIO_BIT_DIR_SET(dev->vcr_plug_pos,HAL_GPIO_I_DIR);
        HAL_GPIO_INT_SET(dev->vcr_plug_pos, 1);
        //HAL_GPIO_INT_REDG_SET(dev->vcr_plug_pos, 1);
        HAL_GPIO_INT_FEDG_SET(dev->vcr_plug_pos, 1);
        HAL_GPIO_INT_CLEAR(dev->vcr_plug_pos);
#if(SYS_CHIP_MODULE == ALI_S3602)
        lsr_id = 8;
        osal_interrupt_register_lsr(lsr_id, (ISR_PROC)dev->vcr_callback, (UINT32)dev);
#endif

    }

#if 0
    MEMSET(tmp_buf, 0x5a, 10);
    tmp_buf[0] = 0;
    i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf, 1, 10);

    for(i=0; i<10; i++)
        if(tmp_buf[i]!=tp->reg_val[i])
            return ERR_FAILUE;
#endif
    return ret;
}


INT32 scart_mx9671_attach(struct scart_init_param *param)
{
    struct scart_device *dev = NULL;
    struct scart_mx9671_private *tp = NULL;
    const UINT16 scart_max_vol = 7;

    if(!param)
    {
        ASSERT(0);
        return ERR_FAILUE;
    }
    dev = (struct scart_device *)dev_alloc(scart_mx9671_name, HLD_DEV_TYPE_SCART, sizeof(struct scart_device));
    if (NULL == dev)
    {
        PRINTF("Error: Alloc SCART switch device ERROR!\n");
        return ERR_NO_MEM;
    }

    dev->base_addr =    param->reserved3;//SCART_AK4707_BASE_ADDR;
    dev->i2c_type_id = param->i2c_type_id;
    dev->vcr_plug_pos = param->vcr_plug_pos;
    dev->vcr_callback = param->vcr_callback;

    if(param->scart_volume <= scart_max_vol)
    {
        g_mx_volume = param->scart_volume;
    }

    dev->priv    =    MALLOC(sizeof(struct scart_mx9671_private));
    if (NULL == dev->priv)
    {
        dev_free(dev);
        return ERR_NO_MEM;
    }
    MEMSET(dev->priv, 0, sizeof(struct scart_mx9671_private));
    tp = (struct scart_mx9671_private*)dev->priv;
    tp->priv_addr = dev->priv;
    tp->reg_val[0] = 0x40|g_mx_volume<<1|0;
    tp->reg_val[1] = 0x10;
    tp->reg_val[6] = 0x00;
    tp->reg_val[7] = 0x01;
    tp->reg_val[8] = 0x98;
    tp->reg_val[9] = 0x06;
    tp->reg_val[13] = 0xFE;
    tp->reg_val[16] = 0xC0;

    /* Function point init */
    dev->attach    =    scart_mx9671_attach;
    dev->open    =    scart_mx9671_open;
    dev->close    =    scart_mx9671_close;
    dev->ioctl    =    scart_mx9671_io_control;
    /* Add this device to queue */
    if(dev_register(dev) != RET_SUCCESS)
    {
        PRINTF("Error: Register RF modulator device ERROR!\n");
        return ERR_DEV_ERROR;
    }
    dev->flags = 0;
    scart_mx9671_name[STRLEN(scart_mx9671_name) - 1]++;
    return RET_SUCCESS;
}

static INT32 scart_mx9671_detach(struct scart_device *dev)
{
    struct scart_mx9671_private *tp = NULL;
    UINT16 lsr_id = 0;
    INT32 ret = !RET_SUCCESS;

    if(dev != NULL)
    {

        if(dev->vcr_callback != (UINT32)NULL)
        {
#if(SYS_CHIP_MODULE == ALI_S3602)
            lsr_id = 8;
            osal_interrupt_unregister_lsr(lsr_id, (ISR_PROC)dev->vcr_callback);
#endif
        }
        tp = (struct scart_mx9671_private*)dev->priv;
        FREE(tp);
        tp = NULL;
        dev_free(dev);
        dev = NULL;
        ret = RET_SUCCESS;
    }
    else
    {

        ret = ERR_DEV_ERROR;
    }

    return ret;
}
