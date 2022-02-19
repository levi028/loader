/*
 * Copyright (C) ALi Shanghai Corp. 2006
 *
 * Description
 *    Temp information save and load utility.
 *
 * History
 * 1. 20050118    Zhengdao Li    Change dev_get_by_name to dev_get_by_id.
 */
 #include <sys_config.h>

 #ifdef SUPPORT_FRANCE_HD
#include <types.h>
#include <retcode.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>

#include "nit_info.h"

#define SAVE_NIT_INFO_FAILED        -1
#define SAVE_NIT_INFO_OUTRANGE    -2
#define LOAD_NIT_INFO_FAILED        -3
#define ERASE_NIT_INFO_FAILED        -4

#define NIT_DATA_START_ID        0x12345678
#define NIT_DATA_END_ID        0x87654321


#define TMP_PRINTF PRINTF


static INT32 nit_f_addr = -1;
static UINT32 g_nit_data_base_addr = 0;
static UINT32 g_nit_data_len = 0;


// NIT_DATA_START_ID (UINT16 num) (UINT8 verison_number) (8 byte*num) NIT_DATA_END_ID
static INT32 _read_nit_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    INT32 ret;
    struct sto_device *sto_flash_dev;

    if(addr + buff_len > g_nit_data_len)
    {
        TMP_PRINTF("_read_nit_data(): invalid addr!\n");
        return LOAD_NIT_INFO_FAILED;
    }

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_get_data(sto_flash_dev, buff, addr+g_nit_data_base_addr, (INT32)buff_len) == (INT32)buff_len)
        return SUCCESS;
    else
    {
        TMP_PRINTF("_read_nit_data(): sto_read error at addr %x\n",addr);
        return LOAD_NIT_INFO_FAILED;
    }
}


static INT32 _get_nit_free_addr(UINT16 tmp_len)
{

    if(nit_f_addr == -1)
    {
        INT32 i;
        UINT8 data;
        for(i = g_nit_data_len -sizeof(data); i>=0; i-=sizeof(data))
        {
            if (_read_nit_data(i, (UINT8 *)&data, sizeof(data)) != SUCCESS)
            {
                return LOAD_NIT_INFO_FAILED;
            }
            if (data != 0xFF)
                break;
        }
        nit_f_addr = i+sizeof(data);
    }
    TMP_PRINTF("nit_f_addr=%08x\n", nit_f_addr);
    return nit_f_addr;
}




/**
 * backward_locate
 *
 * locating ident value from start_addr backward till to margin (include).
 * @start_addr    : the starting locating address.
 * @margin    : the address to end search.
 * @ident    : the identifier
 * return    : -1: identifier not found, other: the identifier address.
 */

static INT32 backward_locate(INT32 start_addr, INT32 margin, UINT32 ident)
{
    UINT8 data;
    UINT8 i = 0;;
    INT32 t_start_addr;

    do{
        if (_read_nit_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
            return -1;
        start_addr -= sizeof(UINT8);

        if(data == ((0xff000000&ident)>>24))
        {
            t_start_addr = start_addr;
            for(i=1; i<sizeof(UINT32); i++)
            {
                if (_read_nit_data(t_start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
                    return -1;
                t_start_addr -= sizeof(UINT8);
                if(data != ((ident & (0xFF000000>>(8*i)))>>(24-8*i)))
                    break;
            }
            if(i==sizeof(UINT32))
                return t_start_addr+sizeof(UINT8);
        }
    }while(start_addr > 0);
    return -1;
}


#if 0
static INT32 backward_locate(INT32 start_addr, INT32 margin, UINT32 ident)
{
    UINT32 data;
    if (_read_nit_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
        return -1;
    while (data != ident)
    {
        start_addr -= sizeof(UINT32);
        if (start_addr<margin)
        {
            break;
        }
        else if(_read_nit_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
        {
            return -1;
        }
     }
    return (start_addr<margin)? -1: start_addr;
}
#endif

static INT32 _write_nit_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    INT32 ret;
    struct sto_device *sto_flash_dev;

    if(addr + buff_len > g_nit_data_len)
        return SAVE_NIT_INFO_OUTRANGE;

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_put_data(sto_flash_dev, addr+g_nit_data_base_addr, buff, (INT32)buff_len) == (INT32)buff_len)
    {
        if(nit_f_addr != -1)
                nit_f_addr = addr + buff_len;
        return SUCCESS;
    }
    else
    {
        TMP_PRINTF("_read_nit_data(): sto_read error at addr %x\n",addr);
        return SAVE_NIT_INFO_FAILED;
    }

}



INT32 erase_nit_sector()
{
    UINT32 param;
    struct sto_device *sto_flash_dev;

     sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    UINT32 flash_cmd;
    if(sto_flash_dev->totol_size <= 0x400000)
    {
        param = (g_nit_data_base_addr<<10) + ( g_nit_data_len>>10);
        flash_cmd = STO_DRIVER_SECTOR_ERASE;
    }
    else
    {
        UINT32 tmp_param[2];
        tmp_param[0] = g_nit_data_base_addr;
        tmp_param[1] = ( g_nit_data_len>>10);
        param = (UINT32)tmp_param;
        flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
    }
    if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
    {
        TMP_PRINTF("erase_nit_sector(): Erase info table flash memory failed!\n");
        return ERASE_NIT_INFO_FAILED;
    }
    nit_f_addr = -1;
    return SUCCESS;
}


INT32 save_nit_data(UINT8 *buff,UINT16 buff_len)
{
    UINT32 f_addr;
    UINT32 start_id = NIT_DATA_START_ID,end_id = NIT_DATA_END_ID;

    f_addr = _get_nit_free_addr(buff_len);
    if(f_addr + (UINT32)buff_len + 8 > g_nit_data_len)/*include Start&End flag length */
    {
        erase_nit_sector();
        f_addr = _get_nit_free_addr(buff_len);
    }

    /* if tmp data sector is blank,the free addr is 0 */
    _write_nit_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
    _write_nit_data(f_addr+4,buff,buff_len);
    _write_nit_data(f_addr+buff_len+4,(UINT8 *)&end_id,sizeof(UINT32));
    return SUCCESS;
}


INT32 load_nit_data(UINT8 *buff,UINT16 buff_len)
{
    INT32 t_addr, h_addr;
    UINT16 num = 0;

    if (g_nit_data_len < (UINT32)(h_addr = _get_nit_free_addr(buff_len)))
    {
        TMP_PRINTF("_load_nit_data(): free address not enough or invalid.\n");
        return LOAD_NIT_INFO_FAILED;
    }
    TMP_PRINTF("tmp info free addr = %08x\n", h_addr);
    /*
     * h_addr is not always the next position for ending ID.
     * since reported from YST.
     */
    do {
        /*
         * looking for ending ID from tail to the position
         * where only 1 valid entry could write.
         */
        //t_addr = backward_locate(h_addr-4, buff_len+4, NIT_DATA_END_ID);
        t_addr = backward_locate(h_addr-1, buff_len+4, NIT_DATA_END_ID);

        TMP_PRINTF("tail address=%08x\n",t_addr);
        if (t_addr<0)
        {
            TMP_PRINTF("valid tail not found!\n");
            return LOAD_NIT_INFO_FAILED;
        }
        /*
         * looking for starting ID from tail-4
         */
        h_addr = backward_locate(t_addr-1, 0, NIT_DATA_START_ID);
        TMP_PRINTF("head address=%08x\n", h_addr);
        if (h_addr<0)
        {
            TMP_PRINTF("valid head not found!\n");
            return LOAD_NIT_INFO_FAILED;
        }

        _read_nit_data(h_addr+4, (UINT8 *)&num, sizeof(UINT16));
    }while (t_addr-h_addr != (INT32)(sizeof(struct NIT_VER_INFO)+sizeof(struct NIT_SER_INFO)*(num-1)+4)); /* at the correct position.*/

    return _read_nit_data(h_addr+4,buff,buff_len);

}


INT32 init_nit_info(UINT32 base_addr, UINT32 len)
{
    if(g_nit_data_base_addr != 0 && g_nit_data_len != 0)
        return SUCCESS;

    g_nit_data_base_addr = base_addr;
    g_nit_data_len = len;

    return SUCCESS;
}
#endif

