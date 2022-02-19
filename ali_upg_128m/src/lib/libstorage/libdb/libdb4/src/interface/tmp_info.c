/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: tmp_info.c
*
*    Description: implement temp information save and load utility .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libdb/tmp_info.h>
#ifdef HMAC_CHECK_TEMP_INFO
#include <api/librsa/rsa_verify.h>
#else
#include <api/libc/fast_crc.h>
#endif


#define TMP_PRINTF PRINTF
#define CRC_SIZE    4

#ifdef SUPPORT_TWO_TMP_INFO
#define SECTOR_SIZE (64*1024)
static INT32 tmp_f_addr_ahead = -1;
static UINT32 g_tmp_data_ahead_base_addr = 0;
static UINT32 g_tmp_data_ahead_len = 0;
static UINT8 *g_tmp_buf_ahead = NULL;
#endif

static INT32 tmp_f_addr = -1;
static UINT32 g_tmp_data_base_addr = 0;
static UINT32 g_tmp_data_len = 0;
static UINT8 *g_tmp_buf = NULL;

#ifdef HMAC_CHECK_TEMP_INFO
UINT8 g_tmp_data_key[16]=
    {
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
		0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,
	};
#endif

extern void mg_setup_crc_table(void);
static INT32 _read_tmp_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    //INT32 ret = 0;
    struct sto_device *sto_flash_dev = NULL;

    if((addr + buff_len > g_tmp_data_len) || (NULL == buff))
    {
        TMP_PRINTF("_read_tmp_data(): invalid addr!\n");
        return LOAD_TMP_INFO_FAILED;
    }

    /*read data from tmp memory buf*/
    if(g_tmp_buf!=NULL)
    {
        MEMCPY(buff, &g_tmp_buf[addr], buff_len);
        return SUCCESS;
    }
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_get_data(sto_flash_dev, buff, addr+g_tmp_data_base_addr, (INT32)buff_len) == (INT32)buff_len)
    {
        return SUCCESS;
    }
    else
    {
        TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
        return LOAD_TMP_INFO_FAILED;
    }
}


static INT32 _get_tmp_free_addr(void)
{
    INT32 i = 0;
    UINT8 data = 0;
    struct sto_device *sto_flash_dev = NULL;

    //tmp_len = 0;//for compiler
    if(-1 == tmp_f_addr)
    {
        /*load tmp data sector all to memory, then search free addr*/
        if(NULL == g_tmp_buf)
        {
            g_tmp_buf = (UINT8 *)MALLOC(g_tmp_data_len);
            if(NULL == g_tmp_buf)
            {
                TMP_PRINTF("%s(): malloc fail!\n",__FUNCTION__);
                return LOAD_TMP_INFO_FAILED;
            }
            sto_flash_dev=(struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO,0);
            if(sto_get_data(sto_flash_dev, g_tmp_buf, g_tmp_data_base_addr,
                (INT32)g_tmp_data_len) != (INT32)g_tmp_data_len)
            {
                return LOAD_TMP_INFO_FAILED;
            }
        }

        for(i = g_tmp_data_len -sizeof(data); i>=0; i-=sizeof(data))
        {
            /*if (_read_tmp_data(i, (UINT8 *)&data, sizeof(data)) != SUCCESS)
            {
                return LOAD_TMP_INFO_FAILED;
            }
            if (data != 0xFF)
                break;
            */
            /*load tmp data sector all to memory, then search free addr*/
            if(g_tmp_buf[i]!=0xFF)
            {
                break;
            }
        }
        tmp_f_addr = i+sizeof(data);
    }
    TMP_PRINTF("tmp_f_addr=%08x\n", tmp_f_addr);
    return tmp_f_addr;
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
    UINT32 data = 0;

	if((-1==start_addr)||( -1 ==margin)||(UINT_MAX ==ident))
	{
		return -1;
	}
    if (_read_tmp_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
    {
        return -1;
    }
    while (data != ident)
    {
        start_addr -= sizeof(UINT32);
        if (start_addr<margin)
        {
            break;
        }
        else if(_read_tmp_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
        {
            return -1;
        }
    }
    return (start_addr<margin)? -1: start_addr;
}

static INT32 _write_tmp_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    //INT32 ret = 0;
    struct sto_device *sto_flash_dev = NULL;

    if((addr + buff_len > g_tmp_data_len)||(NULL == buff))
    {
        return SAVE_TMP_INFO_OUTRANGE;
    }

    /*write data to tmp data buf at same time*/
    if(g_tmp_buf!=NULL)
    {
        MEMCPY(&g_tmp_buf[addr], buff, buff_len);
    }

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_put_data(sto_flash_dev, addr+g_tmp_data_base_addr, buff, (INT32)buff_len) == (INT32)buff_len)
    {
        if(tmp_f_addr != -1)
        {
            tmp_f_addr = addr + buff_len;
        }
        return SUCCESS;
    }
    else
    {
        TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
        return SAVE_TMP_INFO_FAILED;
    }

}


INT32 erase_tmp_sector(void)
{
    UINT32 param = 0;
    struct sto_device *sto_flash_dev = NULL;
    UINT32 flash_cmd = 0;
    const UINT32 flash_total_size = 0x400000;
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    UINT32 tmp_param[2] = {0};

    if(NULL == sto_flash_dev)
    {
        ASSERT(0);
        return !SUCCESS;
    }

    if((UINT32)sto_flash_dev->totol_size <= flash_total_size)
    {
        param = (g_tmp_data_base_addr<<10) + ( g_tmp_data_len>>10);
        flash_cmd = STO_DRIVER_SECTOR_ERASE;
    }
    else
    {
        tmp_param[0] = g_tmp_data_base_addr;
        tmp_param[1] = ( g_tmp_data_len>>10);
        param = (UINT32)tmp_param;
        flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
    }
    if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
    {
        TMP_PRINTF("%s():Erase info table flash memory failed!\n",__FUNCTION__);
        return ERASE_TMP_INFO_FAILED;
    }
    tmp_f_addr = -1;
    return SUCCESS;
}


INT32 save_tmp_data(UINT8 *buff,UINT16 buff_len)
{
    UINT32 f_addr = 0;
    UINT32 start_id = TMP_DATA_START_ID;
    UINT32 end_id = TMP_DATA_END_ID;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32 = 0;
#endif
    UINT32 ret = 0;
    const UINT8 tmp = 8;

    if(NULL == buff)//||(SYS_FLASH_SIZE<buff_len))
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return SAVE_TMP_INFO_FAILED;
    }

    f_addr = _get_tmp_free_addr();
    /*include Start&End flag length */
#ifdef HMAC_CHECK_TEMP_INFO
    if(f_addr + (UINT32)buff_len + tmp + HMAC_OUT_LENGTH > g_tmp_data_len)
#else
    if(f_addr + (UINT32)buff_len + tmp + CRC_SIZE > g_tmp_data_len)
#endif
    {
        /*free tmp data buf befor erase sector*/
        if(g_tmp_buf!=NULL)
        {
            FREE(g_tmp_buf);
            g_tmp_buf = NULL;
        }

        ret = erase_tmp_sector();
        if (SUCCESS != ret)
        {
            TMP_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        f_addr = _get_tmp_free_addr();
    }
#ifdef HMAC_CHECK_TEMP_INFO
	//ret = calculate_hmac(buff, buff_len, hmac_out, g_tmp_data_key);
    ret = api_gen_hmac_ext(buff, buff_len, hmac_out, g_tmp_data_key);
#else
    crc_32 = mg_table_driven_crc(0xFFFFFFFF,buff,buff_len);
#endif

    /* if tmp data sector is blank,the free addr is 0 */
    ret = _write_tmp_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
    ret = _write_tmp_data(f_addr+4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    ret = _write_tmp_data(f_addr+ 4 + buff_len,hmac_out,HMAC_OUT_LENGTH);
    ret = _write_tmp_data(f_addr+buff_len + 4 + HMAC_OUT_LENGTH,(UINT8 *)&end_id,\
                            sizeof(end_id));
#else
    ret = _write_tmp_data(f_addr+ 4 + buff_len,(UINT8 *)&crc_32,CRC_SIZE);
    ret = _write_tmp_data(f_addr+buff_len + 4 + CRC_SIZE,(UINT8 *)&end_id,\
                            sizeof(end_id));
#endif

    return SUCCESS;
}


INT32 load_tmp_data(UINT8 *buff,UINT16 buff_len)
{
    INT32 t_addr = 0;
    INT32 h_addr = 0;
    INT32 ret = 0;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
    UINT8 hmac[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32 = 0;
#endif
    const UINT8 tmp = 4;

    if(NULL == buff)
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return LOAD_TMP_INFO_FAILED;
    }

    h_addr = _get_tmp_free_addr();
    if (g_tmp_data_len < (UINT32)h_addr)
    {
        TMP_PRINTF("_load_tmp_data(): free address not enough or invalid.\n");
        return LOAD_TMP_INFO_FAILED;
    }
    TMP_PRINTF("tmp info free addr = %08x\n", h_addr);
    /*
     * h_addr is not always the next position for ending ID.
     * since reported from YST.
     */
    do
    {
        /*
         * looking for ending ID from tail to the position
         * where only 1 valid entry could write.
         */
        t_addr = backward_locate(h_addr-4, buff_len+4, TMP_DATA_END_ID);
        TMP_PRINTF("tail address=%08x\n",t_addr);
        if (t_addr<0)
        {
            TMP_PRINTF("valid tail not found!\n");
            return LOAD_TMP_INFO_FAILED;
        }
        /*
         * looking for starting ID from tail-4
         */
        h_addr = backward_locate(t_addr-4, 0, TMP_DATA_START_ID);
        TMP_PRINTF("head address=%08x\n", h_addr);
        if (h_addr<0)
        {
            TMP_PRINTF("valid head not found!\n");
            return LOAD_TMP_INFO_FAILED;
        }
        /* at the correct position.*/
    }
#ifdef HMAC_CHECK_TEMP_INFO
    while (t_addr-h_addr != buff_len + tmp + HMAC_OUT_LENGTH);
#else
	while (t_addr-h_addr != buff_len + tmp + CRC_SIZE);
#endif
    ret = _read_tmp_data(h_addr + 4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    ret = _read_tmp_data(h_addr + 4 + buff_len,hmac,HMAC_OUT_LENGTH);
#else
    ret = _read_tmp_data(h_addr + 4 + buff_len,(UINT8 *)&crc_32,CRC_SIZE);
#endif
	
    if(SUCCESS == ret)
    {
	#ifdef HMAC_CHECK_TEMP_INFO
	    //ret = calculate_hmac(buff, buff_len, hmac_out, g_tmp_data_key);
        ret = api_gen_hmac_ext(buff, buff_len, hmac_out, g_tmp_data_key);
		if(0 != MEMCMP(hmac,hmac_out,HMAC_OUT_LENGTH))
	#else
        if(crc_32 != mg_table_driven_crc(0xFFFFFFFF,buff,buff_len))
	#endif
        {
            TMP_PRINTF("CRC error!\n");
            ret = LOAD_TMP_INFO_CRC_FAILED;
        }
    }
    return ret;

}

#ifdef SUPPORT_TWO_TMP_INFO
static INT32 _read_tmp_data_ahead(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    INT32 ret;
    struct sto_device *sto_flash_dev;

    if(NULL == buff)
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return LOAD_TMP_INFO_FAILED;
    }

    if(addr + buff_len > g_tmp_data_ahead_len)
    {
        TMP_PRINTF("_read_tmp_data(): invalid addr!\n");
        return LOAD_TMP_INFO_FAILED;
    }

    /*read data from tmp memory buf*/
    if(g_tmp_buf_ahead!=NULL)
    {
        MEMCPY(buff, &g_tmp_buf_ahead[addr], buff_len);
        return SUCCESS;
    }
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_get_data(sto_flash_dev, buff, addr+g_tmp_data_ahead_base_addr, \
                                (INT32)buff_len) == (INT32)buff_len)
        return SUCCESS;
    else
    {
        TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
        return LOAD_TMP_INFO_FAILED;
    }
}

static INT32 _get_tmp_free_addr_ahead(void)
{
    INT32 i    = 0;
    UINT8 data = 0;

    if(-1 == tmp_f_addr_ahead)
    {
        /*load tmp data sector all to memory, then search free addr*/
        struct sto_device *sto_flash_dev = NULL;
        if(g_tmp_buf_ahead==NULL)
        {
            g_tmp_buf_ahead = (UINT8 *)MALLOC(g_tmp_data_ahead_len);
            if(g_tmp_buf_ahead==NULL)
            {
                TMP_PRINTF("%s(): malloc fail!\n",__FUNCTION__);
                return LOAD_TMP_INFO_FAILED;
            }
            sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
            if(sto_get_data(sto_flash_dev, g_tmp_buf_ahead, g_tmp_data_ahead_base_addr,
                    (INT32)g_tmp_data_ahead_len) != (INT32)g_tmp_data_ahead_len)
            {
                return LOAD_TMP_INFO_FAILED;
            }
        }

        for(i = g_tmp_data_ahead_len-sizeof(data); i>=0; i-=sizeof(data))
        {
            /*if (_read_tmp_data(i, (UINT8 *)&data, sizeof(data)) != SUCCESS)
            {
                return LOAD_TMP_INFO_FAILED;
            }
            if (data != 0xFF)
                break;
            */
            /*load tmp data sector all to memory, then search free addr*/
            if(g_tmp_buf_ahead[i]!=0xFF)
            {
                break;
            }
        }
        tmp_f_addr_ahead= i+sizeof(data);
    }
    TMP_PRINTF("tmp_f_addr=%08x\n", tmp_f_addr_ahead);
    return tmp_f_addr_ahead;
}

static INT32 backward_locate_ahead(INT32 start_addr, INT32 margin, UINT32 ident)
{
    UINT32 data = 0;

    if (_read_tmp_data_ahead(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
    {
        return -1;
    }
    while (data != ident)
    {
        start_addr -= sizeof(UINT32);
        if (start_addr<margin)
        {
            break;
        }
        else if(_read_tmp_data_ahead(start_addr, (UINT8 *)&data,sizeof(data))!= SUCCESS)
        {
            return -1;
        }
     }
    return (start_addr<margin)? -1: start_addr;
}

static INT32 _write_tmp_data_ahead(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
    INT32 ret;
    struct sto_device *sto_flash_dev;

    if(NULL == buff)
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return SAVE_TMP_INFO_FAILED;
    }

    if(addr + buff_len > g_tmp_data_ahead_len)
    {
        return SAVE_TMP_INFO_OUTRANGE;
    }

    /*write data to tmp data buf at same time*/
    if(g_tmp_buf_ahead!=NULL)
    {
        MEMCPY(&g_tmp_buf_ahead[addr], buff, buff_len);
    }

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_put_data(sto_flash_dev, addr+g_tmp_data_ahead_base_addr, buff,(INT32)buff_len) == (INT32)buff_len)
    {
        if(tmp_f_addr_ahead!= -1)
        {
            tmp_f_addr_ahead= addr + buff_len;
        }
        return SUCCESS;
    }
    else
    {
        TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
        return SAVE_TMP_INFO_FAILED;
    }

}



INT32 erase_tmp_sector_ahead(void)
{
    UINT32 param = 0;
    struct sto_device *sto_flash_dev = NULL;
    UINT32 flash_cmd = 0;
    const UINT32 flash_total_size = 0x400000;

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(sto_flash_dev->totol_size <= flash_total_size)
    {
        param = (g_tmp_data_ahead_base_addr<<10) + ( g_tmp_data_ahead_len>>10);
        flash_cmd = STO_DRIVER_SECTOR_ERASE;
    }
    else
    {
        UINT32 tmp_param[2];
        tmp_param[0] = g_tmp_data_ahead_base_addr;
        tmp_param[1] = ( g_tmp_data_ahead_len>>10);
        param = (UINT32)tmp_param;
        flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
    }
    if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
    {
        TMP_PRINTF("%s: Erase info table flash memory failed!\n",__FUNCTION__);
        return ERASE_TMP_INFO_FAILED;
    }
    tmp_f_addr_ahead= -1;
    return SUCCESS;
}


INT32 save_tmp_data_ahead(UINT8 *buff,UINT16 buff_len)
{
    UINT32 f_addr;
    UINT32 start_id = TMP_DATA_START_ID,end_id = TMP_DATA_END_ID;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32;
#endif
    const UINT8 tmp = 8;

    if(NULL == buff)
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return SAVE_TMP_INFO_FAILED;
    }

    f_addr = _get_tmp_free_addr_ahead();
    /*include Start&End flag length */
#ifdef HMAC_CHECK_TEMP_INFO
    if(f_addr + (UINT32)buff_len + tmp + HMAC_OUT_LENGTH> g_tmp_data_ahead_len)
#else
    if(f_addr + (UINT32)buff_len + tmp + CRC_SIZE> g_tmp_data_ahead_len)
#endif
    {
        /*free tmp data buf befor erase sector*/
        if(g_tmp_buf_ahead!=NULL)
        {
            FREE(g_tmp_buf_ahead);
            g_tmp_buf_ahead= NULL;
        }

        erase_tmp_sector_ahead();
        f_addr = _get_tmp_free_addr_ahead();
    }
#ifdef HMAC_CHECK_TEMP_INFO
	//calculate_hmac(buff, buff_len, hmac_out, g_tmp_data_key);
	api_gen_hmac_ext(buff, buff_len, hmac_out, g_tmp_data_key);
#else
    crc_32 = mg_table_driven_crc(0xFFFFFFFF,buff,buff_len);
#endif
    /* if tmp data sector is blank,the free addr is 0 */
    _write_tmp_data_ahead(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
    _write_tmp_data_ahead(f_addr+4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    _write_tmp_data_ahead(f_addr+ 4 + buff_len,hmac_out,HMAC_OUT_LENGTH);
    _write_tmp_data_ahead(f_addr+buff_len + 4 + HMAC_OUT_LENGTH,(UINT8 *)&end_id, sizeof(UINT32));
#else
    _write_tmp_data_ahead(f_addr+ 4 + buff_len,&crc_32,CRC_SIZE);
    _write_tmp_data_ahead(f_addr+buff_len + 4 + CRC_SIZE,(UINT8 *)&end_id, sizeof(UINT32));
#endif
    return SUCCESS;
}

INT32 load_tmp_data_ahead(UINT8 *buff,UINT16 buff_len)
{
    INT32 t_addr = 0;
    INT32 h_addr = 0;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
    UINT8 hmac[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32 = 0;
#endif
    INT32 ret = 0;
    const UINT8 tmp = 4;

    if(NULL == buff)
    {
        TMP_PRINTF("%s():wrong param!\n",__FUNCTION__);
        ASSERT(0);
        return LOAD_TMP_INFO_FAILED;
    }

    if (g_tmp_data_ahead_len < (UINT32)(h_addr = _get_tmp_free_addr_ahead()))
    {
        TMP_PRINTF("_load_tmp_data(): free address not enough or invalid.\n");
        return LOAD_TMP_INFO_FAILED;
    }
    TMP_PRINTF("tmp info free addr = %08x\n", h_addr);
    /*
     * h_addr is not always the next position for ending ID.
     * since reported from YST.
     */
    do
    {
        /*
         * looking for ending ID from tail to the position
         * where only 1 valid entry could write.
         */
        t_addr = backward_locate_ahead(h_addr-4, buff_len+4, TMP_DATA_END_ID);
        TMP_PRINTF("tail address=%08x\n",t_addr);
        if (t_addr<0)
        {
            TMP_PRINTF("valid tail not found!\n");
            return LOAD_TMP_INFO_FAILED;
        }
        /*
         * looking for starting ID from tail-4
         */
        h_addr = backward_locate_ahead(t_addr-4, 0, TMP_DATA_START_ID);
        TMP_PRINTF("head address=%08x\n", h_addr);
        if (h_addr<0)
        {
            TMP_PRINTF("valid head not found!\n");
            return LOAD_TMP_INFO_FAILED;
        }
    }
#ifdef HMAC_CHECK_TEMP_INFO
    while (t_addr-h_addr != buff_len+tmp+HMAC_OUT_LENGTH); /* at the correct position.*/
#else
	while (t_addr-h_addr != buff_len+tmp+CRC_SIZE);  /* at the correct position.*/
#endif
    ret = _read_tmp_data_ahead(h_addr + 4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    ret = _read_tmp_data_ahead(h_addr + 4 + buff_len,hmac,HMAC_OUT_LENGTH);
#else
    ret = _read_tmp_data_ahead(h_addr + 4 + buff_len,&crc_32,CRC_SIZE);
#endif
    if(ret == SUCCESS)
    {
	#ifdef HMAC_CHECK_TEMP_INFO
	    //ret = calculate_hmac(buff, buff_len, hmac_out, g_tmp_data_key);
        ret = api_gen_hmac_ext(buff, buff_len, hmac_out, g_tmp_data_key);
		if(0 != MEMCMP(hmac,hmac_out,HMAC_OUT_LENGTH))
	#else
        if(crc_32 != mg_table_driven_crc(0xFFFFFFFF,buff,buff_len))
	#endif
        {
            TMP_PRINTF("CRC error!\n");
            ret = LOAD_TMP_INFO_CRC_FAILED;
        }
    }
    return ret;

}
#endif

INT32 init_tmp_info(UINT32 base_addr, UINT32 len)
{
	if((UINT_MAX==base_addr) ||(SYS_FLASH_SIZE<len))
	{
		return ERR_FAILURE;
	}
    if((0 != g_tmp_data_base_addr) && (0 != g_tmp_data_len))
    {
        return SUCCESS;
    }
#ifdef SUPPORT_TWO_TMP_INFO
    if(len >= (SECTOR_SIZE + SECTOR_SIZE) )
    {
        g_tmp_data_ahead_base_addr = base_addr;
        g_tmp_data_ahead_len = SECTOR_SIZE;
        g_tmp_data_base_addr = base_addr + SECTOR_SIZE;
        g_tmp_data_len = len - SECTOR_SIZE;
    }
    else
#endif
    {
        g_tmp_data_base_addr = base_addr;
        g_tmp_data_len = len;
    }
    mg_setup_crc_table();//setup crc table.
    return SUCCESS;
}



