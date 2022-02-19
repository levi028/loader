/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: swap_info.c
*
*    Description: Temp information save and load utility
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
 #include <sys_config.h>

#ifdef TEMP_INFO_HEALTH
#include <types.h>
#include <retcode.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libc/fast_crc.h>

#ifdef HMAC_CHECK_TEMP_INFO
#include <api/librsa/rsa_verify.h>
#include <api/libdb/tmp_info.h>
#else
#define crc_size    4
#endif

#define SAVE_SWAP_INFO_FAILED        -1
#define SAVE_SWAP_INFO_OUTRANGE    -2
#define LOAD_SWAP_INFO_FAILED        -3
#define ERASE_SWAP_INFO_FAILED        -4
#define LOAD_SWAP_INFO_CRC_FAILED    -5

#define SWAP_DATA_START_ID        0x12345678
#define SWAP_DATA_END_ID        0x87654321


#define TMP_PRINTF PRINTF


static INT32 swap_f_addr = -1;
static UINT32 g_swap_data_base_addr = 0;
static UINT32 g_swap_data_len = 0;

static INT32 _read_swap_data(UINT32 addr,UINT8 *buff,UINT32 buff_len)
{
    struct sto_device *sto_flash_dev;

    if(addr + buff_len > g_swap_data_len)
    {
        TMP_PRINTF("_read_swap_data(): invalid addr!\n");
        return LOAD_SWAP_INFO_FAILED;
    }

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_get_data(sto_flash_dev, buff, addr+g_swap_data_base_addr, (INT32)buff_len) == (INT32)buff_len)
        return SUCCESS;
    else
    {
        TMP_PRINTF("_read_swap_data(): sto_read error at addr %x\n",addr);
        return LOAD_SWAP_INFO_FAILED;
    }
}


static INT32 _get_swap_free_addr(UINT16 tmp_len)
{

    if(-1 == swap_f_addr)
    {
        INT32 i = 0;
        UINT8 data = 0;
        for(i = g_swap_data_len -sizeof(data); i>=0; i-=sizeof(data))
        {
            if (_read_swap_data(i, (UINT8 *)&data, sizeof(data)) != SUCCESS)
            {
                return LOAD_SWAP_INFO_FAILED;
            }
            if (data != 0xFF)
                break;
        }
        swap_f_addr = i+sizeof(data);
    }
    TMP_PRINTF("swap_f_addr=%08x\n", swap_f_addr);
    return swap_f_addr;
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
    UINT8 data = 0;
    UINT8 i = 0;;
    INT32 t_start_addr = 0;
    const UINT8 right_shift_24 = 24;

    do
    {
        if (_read_swap_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
        {
            return -1;
        }
        start_addr -= sizeof(UINT8);

        if(data == ((0xff000000 & ident)>>right_shift_24))
        {
            t_start_addr = start_addr;
            for(i=1; i<sizeof(UINT32); i++)
            {
                if (_read_swap_data(t_start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
                {
                    return -1;
                }
                t_start_addr -= sizeof(UINT8);
                if(data != ((ident & (0xFF000000>>(8*i)))>>(24-8*i)))
                {
                    break;
                }
            }
            if(i==sizeof(UINT32))
            {
                return t_start_addr+sizeof(UINT8);
            }
        }
    }while(start_addr > 0);
    return -1;
}

static INT32 _write_swap_data(UINT32 addr,UINT8 *buff,UINT32 buff_len)
{
    struct sto_device *sto_flash_dev = NULL;

    if(addr + buff_len > g_swap_data_len)
    {
        return SAVE_SWAP_INFO_OUTRANGE;}

    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(sto_put_data(sto_flash_dev, addr+g_swap_data_base_addr, buff, (INT32)buff_len) == (INT32)buff_len)
    {
        if(swap_f_addr != -1)
        {
          swap_f_addr = addr + buff_len;
        }
        return SUCCESS;
    }
    else
    {
        TMP_PRINTF("_read_swap_data(): sto_read error at addr %x\n",addr);
        return SAVE_SWAP_INFO_FAILED;
    }

}

INT32 erase_swap_sector()
{
    UINT32 param = 0;
    struct sto_device *sto_flash_dev = NULL;

     sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    UINT32 flash_cmd = 0;
    UINT32 tmp_param[2] = {0};
    tmp_param[0] = g_swap_data_base_addr;
    tmp_param[1] = ( g_swap_data_len>>10);
    param = (UINT32)tmp_param;
    flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
    if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
    {
        TMP_PRINTF("erase_swap_sector(): Erase info table flash memory failed!\n");
        return ERASE_SWAP_INFO_FAILED;
    }
    swap_f_addr = -1;
    return SUCCESS;
}


INT32 save_swap_data(UINT8 *buff,UINT16 buff_len)
{
    UINT32 f_addr;
    UINT32 start_id = SWAP_DATA_START_ID,end_id = SWAP_DATA_END_ID;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32;
#endif

    f_addr = _get_swap_free_addr(buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    if(f_addr + (UINT32)buff_len + 8 + HMAC_OUT_LENGTH > g_swap_data_len)/*include Start&End flag length */
#else
    if(f_addr + (UINT32)buff_len + 8 + crc_size > g_swap_data_len)/*include Start&End flag length */
#endif
    {
        erase_swap_sector();
        f_addr = _get_swap_free_addr(buff_len);
    }
#ifdef HMAC_CHECK_TEMP_INFO
	//calculate_hmac(buff, buff_len, hmac_out, g_tmp_data_key);
    api_gen_hmac_ext(buff, buff_len, hmac_out, g_tmp_data_key);
#else
    crc_32 = mg_table_driven_crc(0xFFFFFFFF,buff,buff_len);
#endif
    /* if tmp data sector is blank,the free addr is 0 */
    _write_swap_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
    _write_swap_data(f_addr+4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    _write_swap_data(f_addr+ 4 + buff_len,hmac_out,HMAC_OUT_LENGTH);
    _write_swap_data(f_addr+buff_len + 4 + HMAC_OUT_LENGTH,(UINT8 *)&end_id,sizeof(UINT32));
#else
    _write_swap_data(f_addr+ 4 + buff_len,(UINT8 *)&crc_32,crc_size);
    _write_swap_data(f_addr+buff_len + 4 + crc_size,(UINT8 *)&end_id,sizeof(UINT32));
#endif
    return SUCCESS;
}


INT32 load_swap_data(UINT8 *buff,UINT16 buff_len)
{
    INT32 t_addr =0;
    INT32 h_addr = 0;
    INT32  ret = 0;
#ifdef HMAC_CHECK_TEMP_INFO
	UINT8 hmac_out[HMAC_OUT_LENGTH]={0};
    UINT8 hmac[HMAC_OUT_LENGTH]={0};
#else
    UINT32 crc_32 = 0;
#endif
    if (g_swap_data_len < (UINT32)(h_addr = _get_swap_free_addr(buff_len)))
    {
        TMP_PRINTF("_load_swap_data(): free address not enough or invalid.\n");
        return LOAD_SWAP_INFO_FAILED;
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
        //t_addr = backward_locate(h_addr-4, buff_len+4, SWAP_DATA_END_ID);
        t_addr = backward_locate(h_addr-1, buff_len+4, SWAP_DATA_END_ID);

        TMP_PRINTF("tail address=%08x\n",t_addr);
        if (t_addr<0)
        {
            TMP_PRINTF("valid tail not found!\n");
            return LOAD_SWAP_INFO_FAILED;
        }
        /*
         * looking for starting ID from tail-4
         */
        h_addr = backward_locate(t_addr-1, 0, SWAP_DATA_START_ID);
        TMP_PRINTF("head address=%08x\n", h_addr);
        if (h_addr<0)
        {
            TMP_PRINTF("valid head not found!\n");
            return LOAD_SWAP_INFO_FAILED;
        }
    }
#ifdef HMAC_CHECK_TEMP_INFO
	while (t_addr-h_addr != buff_len + 4 + HMAC_OUT_LENGTH); /* at the correct position.*/
#else
	while (t_addr-h_addr != buff_len + 4 + crc_size); /* at the correct position.*/
#endif

    ret = _read_swap_data(h_addr + 4,buff,buff_len);
#ifdef HMAC_CHECK_TEMP_INFO
    ret = _read_swap_data(h_addr + 4 + buff_len,hmac,HMAC_OUT_LENGTH);
#else
    ret = _read_swap_data(h_addr + 4 + buff_len,(UINT8 *)&crc_32,crc_size);
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
            ret = LOAD_SWAP_INFO_CRC_FAILED;
        }
    }
    return ret;

}

INT32 check_swap_info(INT32 *address,INT32 *len)
{
    INT32  t_addr = -1, h_addr = -1;
    UINT32 ident = 0;
    INT32  ret = SUCCESS;
    //UINT32 crc_32 = 0;
    UINT8  *buffer = NULL;
    INT32  i = 0;

    buffer = (UINT8 *)MALLOC(g_swap_data_len);

    if(buffer != NULL)
    {
        ret = _read_swap_data(0,buffer,g_swap_data_len);
        if(ret == SUCCESS)
        {
            for(i = g_swap_data_len - 1; i >= 3;i--)
            {
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
                ident = ((buffer[i] << 24)|(buffer[i - 1] << 16)|(buffer[i - 2] << 8)|buffer[i-3]);
#else
                ident = ((buffer[i - 3] << 24)|(buffer[i - 2] << 16)|(buffer[i - 1] << 8)|buffer[i]);
#endif
                if(SWAP_DATA_END_ID == ident)
                {
                    t_addr = i - 3;
                }
                if(SWAP_DATA_START_ID == ident)
                {
                    h_addr = i - 3;
                }
                if(t_addr != -1 && h_addr != -1)
                    break;
            }
            if(h_addr != -1)
            {
                *address = h_addr + 4;
			#ifdef HMAC_CHECK_TEMP_INFO
    	        *len = t_addr - h_addr - 4 - HMAC_OUT_LENGTH; //end ident not count
			#else
                *len = t_addr - h_addr - 4 - crc_size; //end ident not count
            #endif
            }
            else
                ret = LOAD_SWAP_INFO_FAILED;
        }
        FREE(buffer);
        buffer = NULL;
    }
    else
        ret =  LOAD_SWAP_INFO_FAILED;

    return ret;
}

INT32 init_swap_info(UINT32 base_addr, UINT32 len)
{
    if(g_swap_data_base_addr != 0 && g_swap_data_len != 0)
        return SUCCESS;

    g_swap_data_base_addr = base_addr;
    g_swap_data_len = len;
    //erase_swap_sector();
#ifndef HMAC_CHECK_TEMP_INFO
    mg_setup_crc_table();//setup crc table.
#endif
    return SUCCESS;
}
#endif

