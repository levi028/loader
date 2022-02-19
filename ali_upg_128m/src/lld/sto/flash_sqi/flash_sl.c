/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: flash_sl.c
*
*    Description: Provide local serial flash driver for sto type device.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2006.4.24   Justin Wu   0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include <asm/chip.h>
#include "sto_flash.h"
#include "flash_data.h"

#define STRAP_PIN_REG_ADDR    (0xb8000074)

static char sto_sflash_local_name[HLD_MAX_NAME_SIZE] = "STO_SFLASH_0";

INT32 sto_local_sflash_attach(struct sto_flash_info *param)
{
    struct sto_device *dev = NULL;
    struct flash_private *tp = NULL;
    unsigned int ret = 0;
    UINT32 num_x26bf = 0x26bf;
#ifdef FLASH_SOFTWARE_PROTECT
    struct SWP *swp = NULL;
#endif

    dev = (struct sto_device *)dev_alloc(sto_sflash_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
    if (NULL == dev)
    {
        PRINTF("Error: Alloc storage device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    if (NULL == (tp = (struct flash_private *)MALLOC(sizeof(struct flash_private))))
    {
        dev_free(dev);
        PRINTF("Error: Alloc front panel device priv memory error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(tp, 0, sizeof(struct flash_private));
    dev->priv = tp;

#ifdef FLASH_SOFTWARE_PROTECT
    /* Alloc structure space of SWP */
	if ((swp = MALLOC(sizeof(struct SWP))) == NULL) {
		dev_free(dev);
        dev_free(tp);
		PRINTF("Error: Alloc front panel device priv memory error!\n");
		return ERR_NO_MEM;
	}
	MEMSET(swp, 0, sizeof(struct SWP));
    tp->swp = swp;
#endif

    tp->get_id = sflash_get_id;

    tp->erase_chip = sflash_erase_chip;

    tp->erase_sector = sflash_erase_sector;
    tp->write = sflash_copy;
    tp->read = sflash_read;
    tp->verify = sflash_verify;
    tp->set_io = sflash_set_io;
    tp->open  = NULL;
    tp->close = NULL;
    tp->io_ctrl = sflash_ioctl;

    if ((NULL == param) || (0 == param->flash_deviceid_num))
    {
        tp->flash_deviceid = &sflash_deviceid[0];
        tp->flash_id_p         = &sflash_id[0];
        tp->flash_io_p    = &sflash_io[0];
        tp->flash_deviceid_num = sflash_deviceid_num;
    }
    else
    {
        tp->flash_deviceid = param->flash_deviceid;
        tp->flash_id_p         = param->flash_id;
        tp->flash_io_p    = param->flash_io;
        tp->flash_deviceid_num = param->flash_deviceid_num;
    }

    /* Current operate address */
    MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));

    dev->base_addr = SYS_FLASH_BASE_ADDR;   /* Flash base address */

    //MUTEX_ENTER();
    ret = sto_sflash_identify(dev, 0);
    //MUTEX_LEAVE();

    if (0 == ret)
    {
        PRINTF("Error: Unknow Flash type.\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if(sflash_devtp == num_x26bf)
    {
        tp->erase_sector = sst26_sf_erase_sector;
        tp->write = sst26_sf_copy;
        tp->read = sst26_sf_read;
        tp->verify = sst26_sf_verify;
    }
    /* Function point init */
    sto_fp_init(dev, sto_local_sflash_attach);

    dev->totol_size = tp->flash_size;

#ifdef FLASH_SOFTWARE_PROTECT
    /* Initialize swp */    
	tp->swp->flash_size = dev->totol_size;
//change bu yuj 20170824
	if(tp->swp->swp_init)
	{
		if(tp->swp->swp_init(tp->swp))
		{
			libc_printf("software protect init failed...\n");
			libc_printf("DEAD LOCK\n");
			//while(1){}        
		}
	}
	else
	{
		libc_printf("can't find swp_init...Your flash doesn't support software protection\n");
		libc_printf("DEAD LOCK\n");
		//while(1){}       
	}
#endif

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        PRINTF("Error: Register Flash storage device error!\n");
        FREE(tp);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    return SUCCESS;
}
static int set_strap_pin(void)
{
    unsigned long data = 0;

    if(sys_ic_get_chip_id() == ALI_C3701)
    {
        data = *((volatile unsigned long *)STRAP_PIN_REG_ADDR);
        data |= (1 << 30);
        data &= ~(1 << 18);
        *((volatile unsigned long *)STRAP_PIN_REG_ADDR) = data;
        return 0;
    }

    return -1;
}

#ifdef FLASH_SOFTWARE_PROTECT
/*
 * Adding flash id, just add id_table, and add compare id in match_swp_init().
 * Like:
 *
 * static UINT32 xxx_id_table[xxx_NUM] = {xxx_id, ...};
 *
 * for(i = 0; i < xxx_NUM; i++){
 *  if(id[0] == xxx_id_table[i]){
 *    swp->swp_init = xxx_swp_init;
 *    break;
 *  }
 * }
 */
#define WB_ID_TABLE_NUM 1
static UINT32 wb_id_table[WB_ID_TABLE_NUM] = {
    0x1740ef    //w25q64
};
#define GD_ID_TABLE_NUM 3
static UINT32 gd_id_table[GD_ID_TABLE_NUM] = {
    0x1640c8,    //gd25q32
    0x1740c8,    //gd25q64
    0x1840c8     //gd25q128
};

#define MX_ID_TABLE_NUM 3
static UINT32 mx_id_table[MX_ID_TABLE_NUM] = {
    0x1620c2,    //mx25l32
    0x1720c2,    //mx25l64
    0x1820c2     //mx25l128
};

//Ben 171025#1
#define ESMT_ID_TABLE_NUM 1
static UINT32 esmt_support_id_table[ESMT_ID_TABLE_NUM] = {
    0x17701c    //EN25QH64
};
//
//Yuj 181018#1
#define XTX_ID_TABLE_NUM 1
static UINT32 xtx_support_id_table[XTX_ID_TABLE_NUM] = {
    0x17400b    
};
#define SP_ID_TABLE_NUM 1
static UINT32 sp_id_table[SP_ID_TABLE_NUM] = {
    0x186001,    //S25FL256S
};

#define MX_DEFAULT_LOCK_SIZE 0x10000

static void match_swp_init(struct SWP *swp, UINT32 *id)
{
    int i = 0;
    swp->swp_lock = NULL;
    swp->swp_unlock = NULL;
    swp->swp_is_lock = NULL;
	swp->swp_get_lock_range = NULL;
    swp->swp_init = NULL;

	libc_printf("match_swp_init--[%8x][%8x][%8x]\n",swp->flash_id, id[0],swp->flash_size);
	for(i = 0; i < WB_ID_TABLE_NUM; i++)
	{
		/*Compare Winbond only & 0xff*/
		if((id[0] & 0xff) == (wb_id_table[i] & 0xff))
		{
			swp->swp_init = wb_swp_init;
			swp->default_lock_addr = 0x00;
			swp->default_lock_len = swp->flash_size/64;
			swp->flash_id = id[0];

			libc_printf("1-default lock_len:0x%x\n", swp->default_lock_len);
			break;
		}
	}

	for(i = 0; i < GD_ID_TABLE_NUM; i++)
	{
		/*Compare GigaDevice only & 0xff*/
		if((id[0] & 0xff) == (gd_id_table[i] & 0xff))
		{
			swp->swp_init = gd_swp_init;
			swp->default_lock_addr = 0x00;
			swp->default_lock_len = swp->flash_size/64;
			swp->flash_id = id[0];

			libc_printf("2-default lock_len:0x%x\n", swp->default_lock_len);
			break;
		}
	}

	//Ben 171025#1
	for(i = 0; i < ESMT_ID_TABLE_NUM; i++)
	{
		/*Compare ESMT only & 0xff*/
		if((id[0] & 0xff) == (esmt_support_id_table[i] & 0xff))
		{
			swp->swp_init = esmt_swp_init;
			swp->default_lock_addr = 0x00;
			swp->default_lock_len = swp->flash_size/64;
			swp->flash_id = id[0];

			libc_printf("3-default lock_len:0x%x\n", swp->default_lock_len);
			break;
		}
	}
  	//Yuj 181018#1
	for(i = 0; i < XTX_ID_TABLE_NUM; i++)
	{
		/*Compare ESMT only & 0xff*/
		if((id[0] & 0xff) == (xtx_support_id_table[i] & 0xff))
		{
			swp->swp_init = xtx_swp_init;
			swp->default_lock_addr = 0x00;
			swp->default_lock_len = swp->flash_size/64;
			swp->flash_id = id[0];

			libc_printf("4-default lock_len:0x%x\n", swp->default_lock_len);
			break;
		}
	}  

	//
 #if 0  //change by yuj 20170823
    for(i = 0; i < MX_ID_TABLE_NUM; i++){
        /*Compare MXIC only & 0xff*/
        if((id[0] & 0xff) == (mx_id_table[i] & 0xff)){
            swp->swp_init = mx_swp_init;
            swp->default_lock_addr = 0x00;
            swp->default_lock_len = MX_DEFAULT_LOCK_SIZE;
            swp->flash_id = id[0];

			libc_printf("5-default lock_len:0x%x\n", swp->default_lock_len);
            break;
        }
    }
#endif

	for(i = 0; i < SP_ID_TABLE_NUM; i++){
        /*Compare MXIC only & 0xff*/
        if((id[0] & 0xff) == (sp_id_table[i] & 0xff)){
            swp->swp_init = sp_swp_init;
            swp->default_lock_addr = 0x00;
            swp->default_lock_len = 0;
            swp->flash_id = id[0];

            break;
        }
    } 
    
    /* Add comparing new flash id here...
     *
     * for(i = 0; i < xxx_NUM; i++){
     *  if(id[0] == xxx_id_table[i]){
     *    swp->swp_init = xxx_swp_init;
     *    break;
     *  }
     * }
     *
     */
}
#endif

/**************************************************************
 * Function:
 *     sto_flash_identify()
 * Description:
 *     Identify the parameters of flash.
 * Inpute:
 *    dev --- Device control block.
 *    mode
 *       0 --- Ideentify the flash attached to dev
 *       1 --- Identify the local flash and pass the paramters
 *         to those of dev
 * Return Values:
 *    0 ---  Fail to identify flash
 *    1 ---  Idenfify successfully.
 ***************************************************************/
unsigned int sto_sflash_identify(struct sto_device *dev, UINT32 mode)
{
    unsigned short s = 0;
    unsigned short i = 0;
    unsigned short j = 0;
    unsigned long id = 0;
    unsigned long id_buf[3] = {0};
    unsigned char flash_did = 0;
    UINT32 num_x17 = 0x17;
    UINT32 num_x16 = 0x16;
    UINT32 num_x26 = 0x26;
    UINT32 num_x26bf = 0x26bf;
    UINT32 size_2m = 0x200000;
    struct flash_private *tp = (struct flash_private *)dev->priv;

    set_strap_pin();
    if( 0 == mode )
    {
        tp->get_id(id_buf, tflash_cmdaddr[0]);
    }
    else
    {
        /*
        In the mode, one Master STB will upgrade many Slave STBs
        in one time. So we identify local flash and assume that
        all Slave STBs have the same type of flashes. Also, we
        should inform remote_flash driver of the correct flash
        command address.
        */
        sflash_get_id(id_buf, tflash_cmdaddr[0]);
        /* *below operation only applying to remote_flash* */
        if(tp->io_ctrl!=NULL)
        {
            tp->io_ctrl(FLASH_SET_CMDADDR,sflash_devid);
        }
    }
    libc_printf("\nNOR Flash id_buf[0]: 0x%08X, id_buf[1]: 0x%08X, id_buf[2]: 0x%08X\n",
        id_buf[0], id_buf[1], id_buf[2]);

    for (i = 0; i < (tp->flash_deviceid_num)*2; i += 2)
    {
        s = (tp->flash_deviceid)[i + 1];
        id = id_buf[s >> 5];
        s &= 0x1F;
        flash_did = (tp->flash_deviceid)[i];
        if (((id >> s) & 0xFF) == flash_did)
        {
              if (flash_did==num_x17)
             {
                if(0x4d182001 == (id_buf[0]&0xffffffff))
                {
                    continue;
                }
             }
            if(flash_did==num_x16)
            {
                if(0xc2169ec2 == (id_buf[0]&0xffffffff))
                {
                    continue;//for MX25L3255D
                }

            }
            if(flash_did==num_x26)
            {
                if(0xc21826c2 == (id_buf[0]&0xffffffff))
                {
                    continue;//for MX25L12855E
                }
            }
            tp->flash_id = (tp->flash_id_p)[i >> 1];
            tp->flash_io = (tp->flash_io_p)[i >> 1];
            /* special treatment for SST39VF088 */
            if ((FLASHTYPE_39080 == tp->flash_id) && (0 == j))
            {
                tp->flash_id = FLASHTYPE_39088;
            }
            break;
        }
    }
    if (i < (tp->flash_deviceid_num)*2 )
    {
        tp->flash_sectors = (unsigned int) \
            (tflash_sectors[tp->flash_id]);
        tp->flash_size=sto_flash_sector_start(dev,tp->flash_sectors);

    if(sys_ic_get_chip_id() >= ALI_S3602F)
        {
           *((volatile UINT32 *)0xb802e098)&=~0xc0000000;
              if(tp->flash_size <= size_2m)
              {
                *((volatile UINT32 *)(0xb802e098)) |= 0;
              }
           else if((tp->flash_size>0x200000) && (tp->flash_size<=0x400000))
           {
                *((volatile UINT32 *)(0xb802e098)) |= 0x40000000;
           }
             else if((tp->flash_size>0x400000) && (tp->flash_size<=0x800000))
             {
                *((volatile UINT32 *)(0xb802e098)) |= 0x80000000;
             }
           else
           {
                *((volatile UINT32 *)(0xb802e098)) |= 0xc0000000;
           }
        }
        if(0x40EF == ((id_buf[0])&(0xffff)))//W25Q
        {
            tp->flash_io = 1;
        }
        else if(0x30EF == (id_buf[0]&0xffff))//W25X
        {
            tp->flash_io = 1;    // w25x32 can't support 2 line mode
        }
        else if(0x24c2 == (id_buf[0]&0xffff))//MX25L1635D/MX25L3235D
        {
            tp->flash_io = 4;
        }
        else if(0x20c2 == (id_buf[0]&0xffff) && tp->flash_size >= 0x200000) //MX25L1605D/MX25L3205D/MX25L6405D
        {
            tp->flash_io = 1;
        }
        else if(((0x3037 == ((id_buf[0]>>8)&0xffff)) \
                 || (0x2037 == ((id_buf[0]>>8)&0xffff))) \
                 && (tp->flash_size >= 0x200000))//A25L016,A25L032
        {
            tp->flash_io = 2;
        }
        if(((2==tp->flash_io) || (4 ==tp->flash_io)) \
            && sys_ic_get_chip_id()==ALI_S3811 \
            && 0 == (*((volatile unsigned long*)0xb8000480)&(1<<13)))//for S3811
        {
            tp->flash_io = 1;
        }
        if((0x9ec2 == ((id_buf[0])&(0xffff))) && (ALI_S3602F==sys_ic_get_chip_id()))
        {
            *((volatile UINT32 *)(0xb802e000+0x98)) |= 1<<28;//for MX25L3255D
        }

        if(sys_ic_get_chip_id()==ALI_S3811) //for C3811
        {
            tp->flash_io = 1;
        }
        if(sys_ic_get_chip_id()==ALI_S3281) //for C3281
        {
            tp->flash_io = 1;
        }

        if((sys_ic_get_chip_id() == ALI_S3821) || (sys_ic_get_chip_id() == ALI_C3505) \
            || (sys_ic_get_chip_id() == ALI_C3503D) || (sys_ic_get_chip_id() == ALI_C3711C)) //for C3821
        {
            tp->flash_io = 1;
            *((volatile UINT32 *)0xb802e098) &= 0xf0ffffff;
            *((volatile UINT32 *)0xb802e098) |= 0x01000000;		
        }

        if(tp->set_io)
        {
            tp->set_io(tp->flash_io, 0); // 2th args don't used, only for compatibility
        }
    }
    if(sflash_devtp== num_x26bf)
    {
        sst26_sf_get_id(id_buf, tflash_cmdaddr[0]);
    }
    
#ifdef FLASH_SOFTWARE_PROTECT
    tp->swp->flash_size = tp->flash_size;
    match_swp_init(tp->swp, id_buf);
#endif
    
    return (i < (tp->flash_deviceid_num)*2);
}
