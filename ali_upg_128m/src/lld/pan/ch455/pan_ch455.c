/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2008 Copyright (C)
*
*    File:    pan_ch455.c
*
*    Description:    This file contains all functions definition
*                     of Front Panel driver.
*
*    Note:
*    This driver is used to support CH455K front panel controller.
*
*    History:
*    Date            Athor        Version        Reason
*    ============    =======================================
*1.    2008.07.14    Mao Feng        Ver 0.1        Create file.
*2.    2008.11.12                            Use Task instand of Timer to scan keyboard.
*
*****************************************************************************/

#include <sys_config.h>

#include <retcode.h>
#include <types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <bus/i2c/i2c.h>

#include "pan_ch455.h"
#include "../irc/irc.h"

#define NUM_0    0
#define NUM_1    1
#define NUM_2    2
#define NUM_3    3
#define NUM_4    4

#define PAN_CH455_CHAR_LIST_NUM sizeof(bitmap_table)/sizeof(struct led_bitmap)

/* Name for the panel, the last character must be Number for index */
static char  pan_ch455_0_name[HLD_MAX_NAME_SIZE] = "PAN_CH455_0";

static struct led_bitmap bitmap_table[] =
{
    {'.', 0x80}, /* Let's put the dot bitmap into the table */
    {'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f},
    {'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07},
    {'8', 0x7f}, {'9', 0x6f}, {'a', 0x77}, {'A', 0x77},
    {'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39},
    {'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79},
    {'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d},
    {'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30},
    {'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38},
    {'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f},
    {'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67},
    {'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d},
    {'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e},
    {'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b},
    {':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
    {0x01, 0x01}, {0x02, 0x02}, {0x03, 0x04}, {0x04, 0x08},
    {0x05, 0x10}, {0x06, 0x20}, {0x07, 0x40},
};



static UINT32 read_key(struct pan_ch455_private *tp,  UINT8 *data)
{
    return i2c_read(tp->i2c_id, KEY_ADDR, data, 1);
}


static UINT32 write_led(struct pan_ch455_private *tp, UINT8 dig_addr, UINT8 data)
{

    return i2c_write(tp->i2c_id, dig_addr, &data, 1);
}


static INT32  pan_ch455_mode_set(struct pan_device *dev)
{
    struct pan_ch455_private *tp = (struct pan_ch455_private *)dev->priv;
    UINT32 re = 0;

    re=write_led(tp, SETING_ADDR, tp->mode);
    if(re)
    {
        PAN_CH455_PRINTF("Mode Set failed! re = %d cnt = %d\n",__FUNCTION__, re);
        return re;
    }

    return SUCCESS;
}


static void  panel_ch455_task(UINT32 param)
{
    struct pan_device *dev = (struct pan_device *)param;
    struct pan_ch455_private *tp = (struct pan_ch455_private *)dev->priv;
    //struct pan_hw_info *hp = tp->hw_info;
    //UINT8 row = 0;
    //UINT8 column = 0;
    UINT8 status = 0;
    UINT8 data = 0xff;
    UINT32 keypress = PAN_KEY_INVALID;
    UINT32 re = 0;
    struct pan_key key;


    while(1)
    {

        re = read_key(tp,  &data);
        if(re)
        {
            PAN_CH455_PRINTF("%s()=>Scan keyboard failed!re = %d.\n", __FUNCTION__, re);
            goto ch455_sleep;
        }
        else
        {    //bit 7 should always be 0, bit 2 should always be 1.
            if(((data & 0x80) != 0) || (0 == (data & 0x04)))
            {
                PAN_CH455_PRINTF("%s()=>Read bad key code!data = 0x%2x.\n", __FUNCTION__, data);
                goto ch455_sleep;
            }
            else
            {
                //column = data & tp->mask_column;    //(0~1)
                //row = (data & tp->mask_row) >> 3;    //(0~6)
                status = (data & tp->mask_status) >> 6;
                
               /* libc_printf("%s()=>data: 0x%2x, column: %d, row: %d, status: %d.\n", __FUNCTION__, \
                                   data, column, row, status);
               */                    
                 
            }
        }

/*
        //(column,row)=>(0,0), (0,1), (0,2), (0,3), (0,4), (0,5), (0,6), (1,0)
        if(column == 0)
            key_temp = 1 << row;
        else
            key_temp = 0x80;
*/
        keypress = 0xffff0000 | data;

        if (CH455_STATUS_UP == tp->bak_status)
        {
            if (CH455_STATUS_UP == status)
            {
                goto ch455_sleep;
            }

            //status == CH455_STATUS_DOWN
            key.type = PAN_KEY_TYPE_PANEL;
            key.state = PAN_KEY_PRESSED;
            key.count = tp->key_cnt;
            key.code = keypress;
            pan_buff_queue_tail(&key);
            tp->key_cnt++;
            tp->bak_status = status;
            tp->keypress_cnt = 0;
        }
        else    //tp->bak_status == CH455_STATUS_DOWN
        {
            if (CH455_STATUS_UP == status)
            {
                tp->key_cnt = 1;

                //if (tp->hw_info->intv_release & 1)    //If intv_release is odd, pan key repeat enable
                if(1)
                {
                    key.type = PAN_KEY_TYPE_PANEL;
                    key.state = PAN_KEY_RELEASE;
                    key.count = 1;
                    key.code = keypress;
                    pan_buff_queue_tail(&key);
                }
                tp->bak_status = status;
            }
            else
            {
                //status == CH455_STATUS_DOWN
                tp->keypress_cnt++;
                if (tp->keypress_cnt == tp->keypress_intv)
                {
                    key.type = PAN_KEY_TYPE_PANEL;
                    key.state = PAN_KEY_PRESSED;
                    key.count = tp->key_cnt;
                    key.code = keypress;
                    pan_buff_queue_tail(&key);
                    tp->key_cnt++;
                    tp->keypress_cnt = 0;
                }
            }

        }

ch455_sleep:
        osal_task_sleep(CH455_KEY_INTERVAL);
        //osal_task_sleep(1000);

    }//while(1)

}


//This Function should be called before pan_ch455_open().
void pan_ch455_id_set(struct pan_device *dev, UINT32 id)
{
    struct pan_ch455_private *tp = NULL;

    if (NULL == dev)
    {
        return;
    }
    tp = (struct pan_ch455_private *)dev->priv;
    //Set i2c type id.
    tp->i2c_id = id;
}


static INT32  pan_ch455_open(struct pan_device *dev)
{
    struct pan_ch455_private *tp = NULL;
    UINT32 re = 0;
    UINT8 i = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct pan_ch455_private *)dev->priv;
    tp->task_id = OSAL_INVALID_ID;
    //Set i2c type id.
    //tp->i2c_id = I2C_TYPE_SCB1;

    //Config CH455 working mode.
    tp->mode = CH455_MODE;

    //Set bit mask for keyboard scan.
    tp->mask_status = CH455_KEY_STATUS_MASK;
    tp->mask_row = CH455_KEY_ROW_MASK;
    tp->mask_column = CH455_KEY_COLUMN_MASK;

    tp->key_cnt = 1;
    tp->keypress_cnt = 0;

    //Set repeat key interval to 300 ms.
    tp->keypress_intv = 3;

    tp->keypress_bak = PAN_KEY_INVALID;

    //Set back status to up.
     tp->bak_status = CH455_STATUS_UP;

    re = pan_ch455_mode_set(dev);

    if (1 == tp->is_local)
    {
    	  #ifndef _BUILD_USB_LOADER_RET2GAC
        irc_m6303irc_init(tp->hw_info);
        #endif
    }

    if (re)
    {
        return ERR_FAILUE;
    }

    // set the default display string.
    for(i = 0; i < 8; i++)
    {
        tp->lock[i] = 0;
        tp->disp_buff[i] = 0;
    }

#ifdef _DEBUG_VERSION_
    OSAL_T_CTSK task_param;

    //creat ch455 task for keyboard scan.
    task_param.task   =  (FP)panel_ch455_task;
    task_param.stksz  =  0x1000;
    task_param.quantum = 10;
    task_param.itskpri = OSAL_PRI_NORMAL;
    task_param.para1= (UINT32)dev;
    task_param.para2 = 0;

    tp->task_id = osal_task_create(&task_param);
    if(OSAL_INVALID_ID == tp->task_id)
    {
        PAN_CH455_PRINTF("%s()=>Create panel_ch455_task FAILED!\n", __FUNCTION__);
        return ERR_FAILUE;
    }
#endif

    return SUCCESS;
}


static INT32  pan_ch455_close(struct pan_device *dev)
{
    struct pan_ch455_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct pan_ch455_private *)dev->priv;
#ifdef _DEBUG_VERSION_
    /* Un-register an 1mS cycle interrupt ISR */
    if (tp->task_id != OSAL_INVALID_ID)
    {
        osal_task_delete(tp->task_id);
        tp->task_id = OSAL_INVALID_ID;
    }
#endif
#if(SYS_IRP_MOUDLE != SYS_DEFINE_NULL)
    if(1 == tp->is_local)
    {
    	 #ifndef _BUILD_USB_LOADER_RET2GAC
       irc_m6303irc_close();
       #endif
    }
#endif
    return SUCCESS;
}


static UINT8 pan_ch455_bitmap(struct pan_device *dev, UINT8 c)
{
    struct pan_ch455_private *tp = (struct pan_ch455_private *)dev->priv;
    int len = tp->bitmap_len;
    struct led_bitmap node;
    UINT8 bitmap = 0;
    UINT16 i = 0;

    for (i=0; i<len; i++)
    {
        node = tp->bitmap_list[i];
        if (node.character == c)
        {
            bitmap = (UINT8)node.bitmap;
            break;
        }
    }

    if(i == len)
    {
        PAN_CH455_PRINTF("%s()=>Character not found.\n", __FUNCTION__);
        return 0;
    }

    return bitmap;

}


static void pan_ch455_display(struct pan_device *dev, char *data, UINT32 len)
{
    struct pan_ch455_private *tp = NULL;
    UINT32 pdata = 0;
    UINT8 addr = 0;
    UINT8 bitmap = 0;
    UINT8 temp = 0;
    UINT32 re = 0;
    UINT32 i = 0;
    char esc_command_27 = 27;
    //UINT8 position = (data[pdata + 3])>>4;
    UINT8 led_num=0;
    if (NULL == dev)
    {
        return;
    }
    tp = (struct pan_ch455_private *)dev->priv;
    if (esc_command_27 == data[pdata])            /* ESC command */
    {
        switch(data[pdata + 1])
        {
            case PAN_ESC_CMD_LBD:

                led_num=data[pdata + 2];
                if(led_num<NUM_3+1){
                    if(PAN_ESC_CMD_LBD_ON == (data[pdata + 3]&0x0f))
                    {
                        tp->lock[led_num]=1;
                    }
                    else
                    {
                        tp->lock[led_num]=0;
                    }
                    // if lock status update, must update to 455 at once.
                    pan_ch455_display(dev,(char *)tp->disp_buff, 5/*4*/);//change on 2011-11-02 for ISDVT program num show
                }
                break;

            default:
                break;
        }
    }
    else
    {
        i = 0;
        while ((pdata < len) && (i <= NUM_4))
        {
            temp = data[pdata];
            tp->disp_buff[i] = temp;
            //show ":" for demoboard CH455
            if ((':' == temp) && (i > 1)&& (pdata >= 1))
            {
                addr = DIG1_ADDR;
                bitmap = pan_ch455_bitmap(dev, data[pdata-1]);
                bitmap |= 0x80;
                i--;
            }
			else if (('.' == temp) && (pdata >= 1))
            {
                bitmap = pan_ch455_bitmap(dev, data[pdata-1]);
                bitmap |= 0x80;
               	if ((i > 0) &&( data[pdata-1] != '.'))
					i--;
            }
            else
            {
                bitmap = pan_ch455_bitmap(dev, temp);
                if(i<NUM_3+1)
                {
                    if(tp->lock[i])
                    {
                        bitmap |= 0x80;
                    }
                    else
                    {
                        bitmap &= ~0x80;
                    }
                }
            }
			if (NUM_0 == i)
            {
                addr = DIG0_ADDR;
            }
            else if (NUM_1 == i)
            {
                addr = DIG1_ADDR;
            }
            else if (NUM_2 == i)
            {
                addr = DIG2_ADDR;
            }
            else
            {
                addr = DIG3_ADDR;
            }
            if (i<NUM_4)
            {
               re = tp->write8(tp, addr, bitmap);
            }

            if (re)
            {
                PAN_CH455_PRINTF("%s()=>Display LED failed. re = %d.\n", __FUNCTION__, re);
                return;
            }

            pdata++;
            i++;
        }
    }

    return;
}


static INT32 pan_ch455_ioctl(struct pan_device *dev, INT32 cmd, UINT32 param)
{
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    switch (cmd)
    {
        case PAN_DRIVER_ATTACH:
            break;
        case PAN_DRIVER_SUSPEND:
            break;
        case PAN_DRIVER_RESUME:
            break;
        case PAN_DRIVER_DETACH:
            break;
        case PAN_DRIVER_READ_LNB_POWER:
            break;
        default:
            break;
    }

    return SUCCESS;
}


__ATTRIBUTE_REUSE_
INT32 pan_ch455_attach(struct pan_configuration *config)
{
    struct pan_device *dev = NULL;
    struct pan_ch455_private *tp = NULL;
    int i = 0;

    if ((NULL == config) || (NULL == config->hw_info))
    {
        PAN_CH455_PRINTF("Error: No Config Info.\n");
        return ERR_FAILUE;
    }

    dev = (struct pan_device *)dev_alloc(pan_ch455_0_name, HLD_DEV_TYPE_PAN, sizeof(struct pan_device));
    if (NULL == dev)
    {
        PAN_CH455_PRINTF("Error: Alloc front panel device error!\n");
        return ERR_NO_MEM;
    }

    if (NULL == (tp = (struct pan_ch455_private *)MALLOC(sizeof(struct pan_ch455_private))))
    {
        dev_free(dev);
        PAN_CH455_PRINTF("Error: Alloc front panel device priv memory error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(tp, 0, sizeof(struct pan_ch455_private));

    //if (config->bitmap_list == NULL)
    if (1)
    {
        tp->bitmap_len = PAN_CH455_CHAR_LIST_NUM;
        tp->bitmap_list = &(bitmap_table[0]);
    }
    else
    {
        tp->bitmap_len = config->bitmap_len;
        tp->bitmap_list = config->bitmap_list;
    }
    tp->hw_info = config->hw_info;
    dev->priv = tp;
    dev->led_num = tp->hw_info->num_com;
    dev->key_num = 0;

    /* Function point init */
    dev->init = pan_ch455_attach;
    dev->open = pan_ch455_open;
    dev->stop = pan_ch455_close;
    dev->do_ioctl = pan_ch455_ioctl;
    dev->display = pan_ch455_display;
    dev->send_data = NULL;
    dev->receive_data = NULL;
    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        PAN_CH455_PRINTF("Error: Register panel device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }

    /* Set default bitmap in buffer. At same time get the dot bitmap. */
    for (i = 0; i < tp->bitmap_len; i++)
    {
        if (' ' == tp->bitmap_list[i].character)
        {
            tp->blankmap = tp->bitmap_list[i].bitmap;
        }
        else if ('.' == tp->bitmap_list[i].character)
        {
            tp->dotmap = tp->bitmap_list[i].bitmap;
        }
        else if (':' == tp->bitmap_list[i].character)
        {
            tp->colonmap = tp->bitmap_list[i].bitmap;
        }
    }

    /* Set register operation function handle */
    //init IR in open op func.
    tp->is_local = 1;

    tp->read8 = read_key;
    tp->write8 = write_led;

    return SUCCESS;
}


