/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scart_ak4707.c
*
* Description:
*     scart chip ak4707 driver API implementation for DVBapplication.
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
#include <asm/chip.h>

#include <bus/i2c/i2c.h>
#include "scart_ak4707.h"

/*******************************************************
* macro define
********************************************************/
#define GPIO1_MAX_COUNT 32
#define GPIO2_MAX_COUNT 64

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
static char scart_ak4707_name[HLD_MAX_NAME_SIZE]= "SCART_AK4707_0";
static UINT16 g_ak_volume = 3;

/*******************************************************
* local function declare
********************************************************/

static void scart_ak4707_dump_reg(struct scart_device *dev, UINT8 reg_val[10])
{
    UINT32 i = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    MEMSET(reg_val, 0x5a, 10);
    reg_val[0] = 0;
    i2c_write_read(dev->i2c_type_id, dev->base_addr, reg_val, 1, 10);
    SCART_AK_PRINTF("dump ak4707 reg:\n");
    for(i=0; i<10; i++)
    {
        SCART_AK_PRINTF("%02x ", reg_val[i]);
    }
    SCART_AK_PRINTF("\n");
}

static void scart_ak4707_tv_mode(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;
    BOOL is_3101 = FALSE;
    UINT32 chip_id = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;;
    is_3101 = sys_ic_is_m3101();
    chip_id = sys_ic_get_chip_id();

    reg_offset = 7;
    if(TV_MODE_RGB == param)
    {
        reg_tgt_val = 1<<0;
    }
    else if(TV_MODE_CVBS == param)
    {
        reg_tgt_val = 0<<0;
    }
    else
    {
        reg_tgt_val = 2<<0;
    }
    tp->reg_val[reg_offset] &= (~0x03);
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;

    if ((ALI_S3602 == chip_id) && (!is_3101)&& (4 != (tp->reg_val[4] & 0x07)))
    {
        reg_offset = 4;
        if (TV_MODE_CVBS == param)
        {
            reg_tgt_val = 2;    // to avoid that CVBS's quality is effected by RGB (HD DAC).
        }
        else
        {
            reg_tgt_val = 1;
        }
        tp->reg_val[reg_offset] &= (~0x7);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }

}

static void scart_ak4707_tv_aspect(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    reg_offset = 7;
    if(ASPECT_4_3 == param)
    {
        reg_tgt_val = 3<<2;
    }
    else if(ASPECT_16_9 == param)
    {
        reg_tgt_val = 1<<2;
    }
    else
    {
        reg_tgt_val = 0<<2;
    }
    tp->reg_val[reg_offset] &= (~(0x03<<2));
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;


}

static void scart_ak4707_vcr_aspect(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    reg_offset = 7;
    if(ASPECT_4_3 == param)
    {
        reg_tgt_val = 3<<4;
    }
    else if(ASPECT_16_9 == param)
    {
        reg_tgt_val = 1<<4;
    }
    else
    {
        reg_tgt_val = 0<<4;
    }
    tp->reg_val[reg_offset] &= (~(0x03<<4));
    tp->reg_val[reg_offset] |= reg_tgt_val;
    (tp->reg_dirty_list)[reg_offset] = 1;


}

static void scart_ak4707_tv_source(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;
    BOOL is_3101 = FALSE;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    is_3101 = sys_ic_is_m3101();
    if(SOURCE_STB_IN == param)
    {
        reg_offset = 1;
        reg_tgt_val = 0;
        tp->reg_val[reg_offset] &= (~0x3);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 4;
        if ((ALI_S3602 == sys_ic_get_chip_id()) && (!is_3101)&& (0 == (tp->reg_val[7] & 0x03)))
        {
            reg_tgt_val = 2;    // to avoid that CVBS's quality is effected by RGB (HD DAC).
        }
        else
        {
            reg_tgt_val = 1;
        }
        tp->reg_val[reg_offset] &= (~0x7);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 6;
        reg_tgt_val = 0<<5;
        tp->reg_val[reg_offset] &= (~(0x3<<5));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 7;
        reg_tgt_val = 0<<6;
        tp->reg_val[reg_offset] &= (~(0x1<<6));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
    else if(SOURCE_VCR_IN == param)
    {
        reg_offset = 1;
        reg_tgt_val = 1;
        tp->reg_val[reg_offset] &= (~0x3);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 4;
        reg_tgt_val = 4;
        tp->reg_val[reg_offset] &= (~0x7);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 6;
        reg_tgt_val = 2<<5;//VCR clamp control : biased
        tp->reg_val[reg_offset] &= (~(0x3<<5));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 7;
        reg_tgt_val = 3<<6;
        tp->reg_val[reg_offset] &= (~(0x3<<6));
        tp->reg_val[reg_offset] |= reg_tgt_val;

        // set the same VCR FB as input
        reg_tgt_val = 0x02;
        tp->reg_val[reg_offset] &= (~0x02);
        tp->reg_val[reg_offset] |= reg_tgt_val;

        (tp->reg_dirty_list)[reg_offset] = 1;
    }
    else
    {
        reg_offset = 4;
        reg_tgt_val = 5;
        tp->reg_val[reg_offset] &= (~0x7);
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 7;
        reg_tgt_val = 2<<6;
        tp->reg_val[reg_offset] &= (~(0x3<<6));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static void scart_ak4707_vcr_source(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    if(SOURCE_STB_IN == param)
    {
        reg_offset = 1;
        reg_tgt_val = 0<<4;
        tp->reg_val[reg_offset] &= (~(0x3<<4));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 4;
        reg_tgt_val = 1<<3;
        tp->reg_val[reg_offset] &= (~(0x7<<3));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
    else if(SOURCE_TV_IN == param)
    {
        reg_offset = 1;
        reg_tgt_val = 1<<4;
        tp->reg_val[reg_offset] &= (~(0x3<<4));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 4;
        reg_tgt_val = 3<<3;
        tp->reg_val[reg_offset] &= (~(0x7<<3));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

    }
    else
    {
        reg_offset = 4;
        reg_tgt_val = 4<<3;
        tp->reg_val[reg_offset] &= (~(0x7<<3));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;

        reg_offset = 7;
        reg_tgt_val = 1<<7;
        tp->reg_val[reg_offset] &= (~(0x1<<7));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static void scart_ak4707_audio_mute(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    if (tp)
    {
        reg_offset = 2;
        tp->reg_val[reg_offset] &= (~0x38);
        if(param)
        {
            tp->reg_val[reg_offset] |= (g_ak_volume<<3);
        }
        else
        {
            tp->reg_val[reg_offset] |= (0<<3);
        }
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static void scart_ak4707_sb_out(struct scart_device *dev, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 reg_offset = 0;
    UINT8 reg_tgt_val = 0;

    if ((NULL == dev) || (UINT_MAX == param))
    {
        ASSERT(0);
        return;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    if (tp)
    {
        reg_offset = 7;
        if(param)
        {
            reg_tgt_val = 0;
        }
        else
        {
            reg_tgt_val = 1<<7;
        }
        tp->reg_val[reg_offset] &= (~(0x1<<7));
        tp->reg_val[reg_offset] |= reg_tgt_val;
        (tp->reg_dirty_list)[reg_offset] = 1;
    }
}

static INT32 scart_ak4707_io_control(struct scart_device *dev, INT32 cmd, UINT32 param)
{
    struct scart_ak4707_private *tp = NULL;
    RET_CODE rlt = RET_FAILURE;
    UINT8 reg_current[10] = {0,};
    UINT8 *p_reg = NULL;
    UINT8 reg_offset = 0;
    UINT8 write = 0;
    UINT8 tmp_buf[4] = {0,};
    UINT8 vcr_sb = 0;//0xff;

    if ((NULL == dev)||(SCART_IO_BASE > cmd)||(UINT_MAX == param))
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    if (NULL == tp)
    {
        return RET_FAILURE;
    }

    MEMSET(tp->reg_dirty_list, 0, sizeof(tp->reg_dirty_list));
    switch(cmd)
    {
    case SCART_TV_MODE:
        scart_ak4707_tv_mode(dev, param);
        write = 1;
        break;
    case SCART_TV_ASPECT:
        scart_ak4707_tv_aspect(dev, param);
        write = 1;
        break;
    case SCART_VCR_ASPECT:
        scart_ak4707_vcr_aspect(dev, param);
        write = 1;
        break;
    case SCART_TV_SOURCE:
        scart_ak4707_tv_source(dev, param);
        write = 1;
        break;
    case SCART_VCR_SOURCE:
        scart_ak4707_vcr_source(dev, param);
        write = 1;
        break;
    case SCART_CHK_STATE:
        if(param)
        {
            *((UINT32 *)param) = 0;
            tmp_buf[0] = 8;
            i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf, 1, 1);
            //if(tmp_buf[0]&0x8)
            {
                if(tp->reg_val[7]&0x80)
                {
                    vcr_sb = tmp_buf[0]&0x3;
                }
                if(0!=vcr_sb)
                {
                    *((UINT32 *)param) |= SCART_STATE_VCR_IN;
                }
            }
            if(tmp_buf[0]&0x10)
            {
                *((UINT32 *)param) |= SCART_STATE_TV_IN;
            }
        }
        write = 0;
        break;
    case SCART_AUDIO_MUTE:
        scart_ak4707_audio_mute(dev, param);
        write = 1;
        break;
    case SCART_VCR_SB_OUT:
        scart_ak4707_sb_out(dev, param);
        write = 1;
        break;
    case SCART_REG_UPDATE:
        MEMSET((tp->reg_dirty_list), 0, AK_MAX_REG_LEN);
        scart_ak4707_dump_reg(dev, reg_current);
        p_reg = (UINT8 *)param;
        for(reg_offset=0;reg_offset < 10;reg_offset++)
        {
            if(reg_current[reg_offset] != *p_reg)
            {
                tp->reg_val[reg_offset] = *p_reg;
                (tp->reg_dirty_list)[reg_offset] = 1;
                write = 1;
                SCART_AK_PRINTF("SCART_REG_UPDATE:%d -> %x\n", reg_offset, *p_reg);
            }
            p_reg++;
        }
        break;
    default:
        rlt = ERR_FAILUE;
        break;
    }
    if(SUCCESS == rlt)
    {
        if(write)
        {
            for(reg_offset = 0; reg_offset<AK_MAX_REG_LEN; reg_offset++)
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

static INT32 scart_ak4707_close(struct scart_device *dev)
{
    UINT8 tmp_buf[16] = {0};

    if (NULL == dev)
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    tmp_buf[0] = 0;
    tmp_buf[1] = 1;

    i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);
    //scart_ak4707_dump_reg(dev);
    return RET_SUCCESS;
}

static INT32 scart_ak4707_open(struct scart_device *dev)
{
    struct scart_ak4707_private *tp = NULL;
    UINT8 tmp_buf[16] = {0,};
    UINT16 lsr_id = 0;
    UINT32 scart_state = 0;

    if (NULL == dev)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    tp = (struct scart_ak4707_private *)dev->priv;
    scart_ak4707_io_control(dev, SCART_CHK_STATE, (UINT32)(&scart_state));
    if(scart_state&SCART_STATE_VCR_IN)
    {
        scart_ak4707_io_control(dev, SCART_TV_SOURCE, SOURCE_VCR_IN);
    }
    tmp_buf[0] = 1;
    MEMCPY(&tmp_buf[1], &(tp->reg_val[1]), 9);
    i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 10);//write reg 1~9

    tmp_buf[0] = 0;
    tmp_buf[1] = tp->reg_val[0];
    i2c_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 0

    if(dev->vcr_callback != (UINT32)NULL)
    {/*If register callback,using interrupt way to detect VCR plug in-out*/
        HAL_GPIO_BIT_DIR_SET(dev->vcr_plug_pos,HAL_GPIO_I_DIR);
        HAL_GPIO_INT_SET(dev->vcr_plug_pos, 1);
        //HAL_GPIO_INT_REDG_SET(dev->vcr_plug_pos, 1);
        HAL_GPIO_INT_FEDG_SET(dev->vcr_plug_pos, 1);
        HAL_GPIO_INT_CLEAR(dev->vcr_plug_pos);

        if(dev->vcr_plug_pos < GPIO1_MAX_COUNT)
        {
            lsr_id = 1 + 8;
        }
        else if(dev->vcr_plug_pos < GPIO2_MAX_COUNT)
        {
            lsr_id = 16 + 8;
        }
        else
        {
            lsr_id = 23 + 8;
        }
        osal_interrupt_register_lsr(lsr_id, (ISR_PROC)dev->vcr_callback, (UINT32)dev);
    }

#if 0
    MEMSET(tmp_buf, 0x5a, 10);
    tmp_buf[0] = 0;
    i2c_write_read(dev->i2c_type_id, dev->base_addr, tmp_buf, 1, 10);

    for(i=0; i<10; i++)
        if(tmp_buf[i]!=tp->reg_val[i])
            return ERR_FAILUE;
#endif
    return SUCCESS;
}


INT32 scart_ak4707_attach(struct scart_init_param *param)
{
    struct scart_device *dev = NULL;
    struct scart_ak4707_private *tp = NULL;
    const UINT16 scart_max_vol = 7;

    if(!param)
    {
        ASSERT(0);
        return ERR_FAILUE;
    }
    dev = (struct scart_device *)dev_alloc(scart_ak4707_name, HLD_DEV_TYPE_SCART, sizeof(struct scart_device));
    if (NULL == dev)
    {
        PRINTF("Error: Alloc SCART switch device ERROR!\n");
        return ERR_NO_MEM;
    }

    dev->base_addr =    SCART_AK4707_BASE_ADDR;
    dev->i2c_type_id = param->i2c_type_id;
    dev->vcr_plug_pos = param->vcr_plug_pos;
    dev->vcr_callback = param->vcr_callback;

    if(param->scart_volume <= scart_max_vol)
    {
        g_ak_volume = param->scart_volume;
    }

    dev->priv    =    MALLOC(sizeof(struct scart_ak4707_private));
    if (NULL == dev->priv)
    {
        dev_free(dev);
        return ERR_NO_MEM;
    }
    MEMSET(dev->priv, 0, sizeof(struct scart_ak4707_private));
    tp = (struct scart_ak4707_private*)dev->priv;
    tp->priv_addr = dev->priv;
    tp->reg_val[0] = 0x00;
    tp->reg_val[1] = 0x04;
    tp->reg_val[2] = 0x7|(g_ak_volume<<3);
    tp->reg_val[3] = 0x00;
    tp->reg_val[4] = 0x09;
    tp->reg_val[5] = 0x5f;
    tp->reg_val[6] = 0x00;
    tp->reg_val[7] = 0xbc;
    tp->reg_val[8] = 0x00;
    tp->reg_val[9] = 0x80;
    /* Function point init */
    dev->attach    =    scart_ak4707_attach;
    dev->open    =    scart_ak4707_open;
    dev->close    =    scart_ak4707_close;
    dev->ioctl    =    scart_ak4707_io_control;

    /* Add this device to queue */
    if(dev_register(dev) != SUCCESS)
    {
        PRINTF("Error: Register RF modulator device ERROR!\n");
    }
    dev->flags = 0;
    scart_ak4707_name[STRLEN(scart_ak4707_name) - 1]++;
    return SUCCESS;
}

