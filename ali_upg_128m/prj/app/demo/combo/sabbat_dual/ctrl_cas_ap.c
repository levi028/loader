/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_cas_ap.c
 *
 *    Description: This source file is application control CAS relate process.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
#include <hld/sto/sto.h>
#include <api/libsi/sie_monitor.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
#include <bus/usb2/usb.h>
#include <hld/dsc/dsc.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/lib_epg.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#if ((defined CAS9_VSC) || (defined _C0700A_VSC_ENABLE_))
#include <vsc/vsc.h>
#include <api/libchunk/chunk.h>
#endif
#include "copper_common/com_api.h"
#include "control.h"
#include "key.h"
#include "menus_root.h"
#include "win_com_popup.h"
#ifdef MULTI_CAS
#include <api/libcas/mcas.h>
#endif

#ifdef MULTI_CAS
#define CONAX_CA_SYSTEM_ID      0x0B00
#define CRYPTO_CA_SYSTEM_ID     0x0D00
#define C1800A_CA_SYSTEM_ID1    0x1700
#define C1800A_CA_SYSTEM_ID2    0x0600
#define SECA_CA_SYSTEM_ID       0x0100
#define VIACCESS_CA_SYSTEM_ID   0x0500
#endif

#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
static UINT8    last_flg __MAYBE_UNUSED__= 0;
#endif

#ifdef SUPPORT_CAS9
static UINT32   mem_check_end_addr = 0;
void mem_protect_check(void)
{
    UINT32  mem_check_start_addr = 0;

    mem_check_start_addr = 0xa0000200;

    asm volatile(";\
            la $4, _code_text_end_addr;\
            sw $4, mem_check_end_addr;\
            "::);
    mem_check_end_addr |= 0xa0000000;

    //libc_printf("mem_check_start_addr=0x%x, mem_check_end_addr=0x%x\n", mem_check_start_addr, mem_check_end_addr);
#ifndef _RD_DEBUG_
    trig_ram_mon(mem_check_start_addr, mem_check_end_addr, 5, SHA_SHA_256, TRUE);
#endif

    //MEMSET(mem_check_end_addr-0x100, 0xff, 0x100);//If ram mon really works, it should detected code changed.
    //libc_printf("Run time monitor test fail!\n");//If ram mon really works, we shall not see this print info.
}
#endif
#ifdef MULTI_CAS
static void win_mcas_display(UINT8 *string)
{
    UINT8   back_saved = 0;

#ifdef _INVW_JUICE
    if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
    {
#endif
        win_msg_popup_close();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(GET_MID_L(300), GET_MID_T(120), 300, 120);
        win_compopup_set_msg((char *)string, NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
    }
#endif
}
PRESULT ap_mcas_message_proc(UINT32 __MAYBE_UNUSED__ msg_type, 
    UINT32 msg_code)
{
    UINT8               result = PROC_PASS;
    UINT8               slot = (UINT8) (msg_code & 1);
    UINT16              ca_system_id = (UINT16) (msg_code & 0xFFFE);
    mcas_disp_type_t    disp_type = (mcas_disp_type_t) (msg_code >> 16);
    UINT8               content[200] = {0};
    BOOL show_flag =1;

    switch (disp_type)
    {
    case MCAS_DISP_CARD_IN:
        snprintf((char *)content, 200,"Slot %01x card in", slot);
        break;

    case MCAS_DISP_CARD_OUT:
        snprintf((char *)content, 200,"Slot %01x card out", slot);
        break;

    case MCAS_DISP_CARD_INITED:
        if (CONAX_CA_SYSTEM_ID == ca_system_id)
        {
            snprintf((char *)content, 200,"Slot %01x Conax Card", slot);
        }
        else if (CRYPTO_CA_SYSTEM_ID == ca_system_id)
        {
            snprintf((char *)content, 200,"Slot %01x Crypto Card", slot);
        }
        else if ((C1800A_CA_SYSTEM_ID1 == ca_system_id) || (C1800A_CA_SYSTEM_ID2 == ca_system_id))
        {
            snprintf((char *)content, 200, "Slot %01x C1800A Card", slot);
        }
        else if (SECA_CA_SYSTEM_ID == ca_system_id)
        {
            snprintf((char *)content, 200,"Slot %01x Seca Card", slot);
        }
        else if (VIACCESS_CA_SYSTEM_ID == ca_system_id)
        {
            snprintf((char *)content, 200,"Slot %01x Viaccess Card", slot);
        }
        else
        {
            snprintf((char *)content, 200,"Slot %01x Unknow Card", slot);
        }
        break;

    case MCAS_DISP_CARD_NONE:
        snprintf((char *)content, 200,"Unmatched Card");
        break;

    default:
        show_flag = 0;
        break;
    }

    if(show_flag)
        win_mcas_display(content);
    return result;
}

#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7) || defined(SUPPORT_C0200A))
void ap_mcas_display(mcas_disp_type_t type, UINT32 param)
{
    UINT32  msg_code;
    msg_code = (UINT32) (param | (type << 16));
    if (type == MCAS_DISP_CARD_IN)
    {
        osal_task_dispatch_off();
        last_flg = upgrade_detect_flag;
        upgrade_detect_flag = 0;
        osal_task_dispatch_on();
    }
    else if (type == MCAS_DISP_CARD_OUT)
    {
        osal_task_dispatch_off();
        upgrade_detect_flag = last_flg;
        osal_task_dispatch_on();
    }

    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, msg_code, FALSE);
    return;
}
#endif

#ifdef CAS9_VSC
#define VSC_API_PRINTF(...) do{} while(0)
//#define VSC_API_PRINTF libc_printf
#define VSC_STO_CONFIG_OFFSET 0x20000
#define VSC_STO_CONFIG_ELEMENTSIZE 0x200
#define VSC_STO_CONFIG_ELEMENTS 128
#define VSC_STO_CONFIG_TOTALSIZE (VSC_STO_CONFIG_ELEMENTSIZE*VSC_STO_CONFIG_ELEMENTS)

#define C_SECTOR_SIZE       0x10000

static UINT8 vsc_hash[VSC_STO_NUM][LENGTH_SHA_256]={0};
static UINT8 vsc_key[VSC_STO_NUM][VSC_STO_KEY_SIZE]={0};
static UINT8 vsc_store_buffer[VSC_STO_NUM][__MM_VSC_DATA_LEN]={0};
static UINT8 vsc_config_index=0xff;
static UINT8 vsc_latest_index=0;
static UINT8 vsc_idle_index=0;
static OSAL_ID g_vsc_tsk_id = OSAL_INVALID_ID;
static OSAL_ID g_store_mutex = OSAL_INVALID_ID;
static BOOL g_write_flash[VSC_STO_NUM]={FALSE};
static BOOL g_get_first_index=FALSE;

BOOL write_vsc_done(void)        //for mp tool personalizer, wait write flash done to enable osal_dispatch_off
{
    return (g_write_flash[0] || g_write_flash[1]);
}

static RET_CODE vsc_store_set_config (const VSC_STORE_CONFIG config)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   UINT8 *flash_buffer = NULL;
    INT32 ret = 0;
    UINT32 tmp_param[2];
    UINT32 param=0;

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   // step 1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1);
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN] Can't find key store chunk id 0x%08x.\n", \
                      KEY_STORE_ID);
      return RET_FAILURE;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;

   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X\n", \
                   __FUNCTION__, chunk_id, chunk_addr);


    /*jasonfan 20140319: write next blank area, if reach the end, erase configuration sector*/
    if((vsc_config_index<VSC_STO_CONFIG_ELEMENTS)&&(VSC_STO_CONFIG_ELEMENTS==(vsc_config_index+1)))   /*jasonfan: reach the end of config sector*/
    {
        tmp_param[0] = chunk_addr+VSC_STO_CONFIG_OFFSET;
        tmp_param[1] = (VSC_STO_CONFIG_TOTALSIZE>>10);
        param= (UINT32)tmp_param;
        if ( sto_io_control( flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, param )!= SUCCESS )
        {
            VSC_API_PRINTF("%s: erase error!\n",__FUNCTION__);
            return RET_FAILURE;
        }
        VSC_API_PRINTF("%s: erase OK!!\n",__FUNCTION__);
        vsc_config_index=0xff;
    }
    if(vsc_config_index!=0xff)
        vsc_config_index=vsc_config_index+1;    /*found valid configuration and write to next element*/
    else
        vsc_config_index=0;         /*configuration is empty, write the first element*/

        /*step 3. Write the data back to NVRAM*/
   flash_buffer = (UINT8 *)MALLOC(__MM_VSC_DATA_LEN);
    if (NULL == flash_buffer)
    {
        ASSERT(0);
        return RET_FAILURE;
    }

   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, \
                         (UINT32)flash_buffer);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return RET_FAILURE;
   }

    VSC_API_PRINTF("%s: vsc_config_index= %d\n", __FUNCTION__, vsc_config_index);
    ret = sto_put_data (flash_dev, chunk_addr + VSC_STO_CONFIG_OFFSET+(vsc_config_index*VSC_STO_CONFIG_ELEMENTSIZE), \
                       (UINT8 *)&config, sizeof (VSC_STORE_CONFIG));

        /*jasonfan20140319: if put data fail, try erase and then write again*/
   if (sizeof (VSC_STORE_CONFIG) != ret)
   {
       tmp_param[0] = chunk_addr+VSC_STO_CONFIG_OFFSET;
       tmp_param[1] = (VSC_STO_CONFIG_TOTALSIZE>>10);
       param= (UINT32)tmp_param;
       
       VSC_API_PRINTF ("[MAIN-%s] sto_put_data FAIL\n", __FUNCTION__);
       VSC_API_PRINTF ("[MAIN-%s] erase and put data again\n", __FUNCTION__);
       if ( sto_io_control( flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, param )!= SUCCESS )
       {
           VSC_API_PRINTF("%s: erase error!\n",__FUNCTION__);
           sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
           sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, 0);
           FREE(flash_buffer);
           return RET_FAILURE;
       }
       VSC_API_PRINTF("%s: erase ok!!\n",__FUNCTION__);
       vsc_config_index=0;
       ret = sto_put_data (flash_dev, chunk_addr + VSC_STO_CONFIG_OFFSET+(vsc_config_index*VSC_STO_CONFIG_ELEMENTSIZE), \
                       (UINT8 *)&config, sizeof (VSC_STORE_CONFIG));
       if (sizeof (VSC_STORE_CONFIG) != ret)
       {
          sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
          sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, 0);
          FREE(flash_buffer);
          return RET_FAILURE;
       }
   }

   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return RET_FAILURE;
   }

   if (NULL != flash_buffer)
   {
      FREE (flash_buffer);
   }

   return RET_SUCCESS;
}

static RET_CODE vsc_store_get_config (VSC_STORE_CONFIG *config)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   UINT32 ret = 0;
    UINT8 i=0;
    VSC_STORE_CONFIG config_local;

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   // step1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1);
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN-%s] Can't find key store chunk id 0x%08x.\n", \
                      __FUNCTION__, KEY_STORE_ID);
      return RET_FAILURE;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;

   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X\n", \
                   __FUNCTION__, chunk_id, chunk_addr);

    /*jasonfan 20140319: modify to fit new mechanism*/
    /* step2.
Search the latest key config and Copy  current used key store*/
    for(i=1; i<(VSC_STO_CONFIG_ELEMENTS+1);i++)
    {
   ret = sto_get_data (flash_dev, (UINT8 *)(config), \
                       (chunk_addr + VSC_STO_CONFIG_OFFSET+(VSC_STO_CONFIG_TOTALSIZE-(i*VSC_STO_CONFIG_ELEMENTSIZE))), \
                       sizeof (VSC_STORE_CONFIG));
   if (sizeof (VSC_STORE_CONFIG) != ret)
   {
      VSC_API_PRINTF ("[MAIN-%s] sto_get_data FAIL\n", __FUNCTION__);
      return RET_FAILURE;
   }
        if((0==config->index)||(1==config->index))
        {
            if (VSC_TAG == config->tag)
            {
                vsc_config_index=(VSC_STO_CONFIG_ELEMENTS-i);   /*vsc_config_index 0~127*/
                VSC_API_PRINTF("%s: Found valid config index. config->index=%d, vsc_config_index= %d, i=%d\n", __FUNCTION__ ,config->index, vsc_config_index, i);
                ret=RET_SUCCESS;
                break;
            }
        }
        ret=RET_FAILURE;
    }

    if(RET_FAILURE==ret)
    {
         VSC_API_PRINTF ("[%s] can't find any valid config. ready to get backup\n", __FUNCTION__);
         vsc_config_index=0xff;
        /*Get back-up VSC config from system data*/
        sys_data_get_back_up_vsc_config (config);
        if (VSC_TAG != config->tag)
        {
            VSC_API_PRINTF ("[%s]Backup data also check fail\n", __FUNCTION__);
            return RET_FAILURE;
        }
        else
        {
            //write back to vsc store config
            MEMCPY (&config_local, config, sizeof (VSC_STORE_CONFIG));
            ret=vsc_store_set_config(config_local);
            if(RET_FAILURE==ret)
            {
                return RET_FAILURE;
            }
        }
    }
/*jasonfan:end*/

   return RET_SUCCESS;
}

static void vsc_write_flash (void)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   UINT8 index = 0;
   UINT8 *flash_buffer = NULL;
   RET_CODE ret = 0;
   VSC_STORE_CONFIG config;

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   // step 1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1);
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN] Can't find key store chunk id 0x%08x.\n", \
                      KEY_STORE_ID);
        return;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;

   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X\n", \
                   __FUNCTION__, chunk_id, chunk_addr);

    // step 2. Get VSC config
   MEMSET (&config, 0, sizeof (VSC_STORE_CONFIG));
   vsc_store_get_config (&config);

    // step 3. update index to latest
    index=vsc_latest_index;
	if(g_write_flash[index])	// Should be this case
	{
		vsc_idle_index=(index + 1) % 2;
	}
	else
	{
		index=(index + 1) % 2;
		if(g_write_flash[index])
		{
			vsc_idle_index=(index + 1) % 2;
		}
		else
		{
			return ;//RET_FAILURE;
		}
	}

    // step 4. Write the data back to NVRAM
   flash_buffer = (UINT8 *)MALLOC(__MM_VSC_DATA_LEN);
    if (NULL == flash_buffer)
    {
        ASSERT(0);
        return ;//RET_FAILURE;
    }

   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, \
                         (UINT32)flash_buffer);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return ;//RET_FAILURE;
   }
   VSC_API_PRINTF ("[MAIN-%s] DRAM address used to store is 0x%X\n", \
                   __FUNCTION__, vsc_store_buffer[index]);
   ret = sto_put_data (flash_dev, chunk_addr + (index * __MM_VSC_DATA_LEN), \
                       vsc_store_buffer[index], __MM_VSC_DATA_LEN);
   if (__MM_VSC_DATA_LEN != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] sto_put_data FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }
   VSC_API_PRINTF ("[MAIN-%s] Write chunk index is 0x%X, Return %d\n", \
                   __FUNCTION__, index, ret);

   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return ;//RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }

   if (NULL != flash_buffer)
   {
      FREE (flash_buffer);
   }
    // step 5. Update the index and hash value to VSC config

   config.index = index;
    MEMCPY (&(config.random_key[index][0]), vsc_key[index], VSC_STO_KEY_SIZE);
    MEMCPY (&(config.hash[index][0]), vsc_hash[index], LENGTH_SHA_256);
   config.tag=VSC_TAG;

    // Backup to system data
    sys_data_set_back_up_vsc_config (config);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE, 0,FALSE);
   vsc_store_set_config(config);
    g_write_flash[index]=FALSE;
}

static RET_CODE nvram_write_flash (UINT8 *mem_addr_main, VSC_STORE_CONFIG config)
{
    //UINT32 tick1, tick2;

    //osal_mutex_lock(g_store_mutex, OSAL_WAIT_FOREVER_TIME);
    //tick1=osal_get_tick();
    //libc_printf("enter nvram tick1=%d\n",tick1);
    if(g_get_first_index)
    {
		vsc_latest_index=vsc_idle_index;
		MEMCPY (vsc_hash[vsc_idle_index], &(config.hash[0][0]), LENGTH_SHA_256);
		MEMCPY (vsc_key[vsc_idle_index], &(config.random_key[0][0]), VSC_STO_KEY_SIZE);
		MEMCPY (vsc_store_buffer[vsc_idle_index], mem_addr_main, __MM_VSC_DATA_LEN);
		g_write_flash[vsc_idle_index]=TRUE;
    }
	else
	{
        vsc_latest_index=config.index;
		MEMCPY (vsc_hash[config.index], &(config.hash[0][0]), LENGTH_SHA_256);
		MEMCPY (vsc_key[config.index], &(config.random_key[0][0]), VSC_STO_KEY_SIZE);
		MEMCPY (vsc_store_buffer[config.index], mem_addr_main, __MM_VSC_DATA_LEN);
		g_write_flash[config.index]=TRUE;
		g_get_first_index=TRUE;
	}
    //tick2=osal_get_tick();
    //libc_printf("leave nvram diff=%d\n", tick2-tick1);
    //osal_mutex_unlock(g_store_mutex);

   return RET_SUCCESS;
}
static RET_CODE nvram_data_fetch (UINT8 * mem_addr_main, VSC_STORE_CONFIG *config, VSC_STORE_CONFIG *backup_config)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   UINT32 ret = 0;

   if (NULL == mem_addr_main)
   {
      VSC_API_PRINTF ("[MAIN-%s-%d] illegal address\n",
                      __FUNCTION__, __LINE__);
      ASSERT (0);
   }

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   MEMSET (mem_addr_main, 0x00, __MM_VSC_DATA_LEN);

   // step1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1) ;
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN-%s] Can't find key store chunk id 0x%08x.\n", \
                      __FUNCTION__, KEY_STORE_ID);
      return RET_FAILURE;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;
    
   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X\n", \
                   __FUNCTION__, chunk_id, chunk_addr);

   // step2. Copy  current used key store
    MEMSET (config, 0, sizeof (VSC_STORE_CONFIG));
    vsc_store_get_config(config);
    sys_data_get_back_up_vsc_config (backup_config);

    ret = sto_get_data (flash_dev, mem_addr_main, chunk_addr,  __MM_VSC_DATA_LEN*2);
    if ((__MM_VSC_DATA_LEN*2) != ret)
      {
         VSC_API_PRINTF ("[MAIN-%s] sto_get_data FAIL\n", __FUNCTION__);
         return RET_FAILURE;
      }

   return RET_SUCCESS;
}

static void cas_vsc_task(void)
{
    while(1)
    {
        if(TRUE == (g_write_flash[0] || g_write_flash[1]))
        {
            osal_mutex_lock(g_store_mutex, OSAL_WAIT_FOREVER_TIME);
#ifdef FLASH_SOFTWARE_PROTECT
            ap_set_flash_lock_len(OTA_LOADER_END_ADDR);
#endif
            vsc_write_flash();
#ifdef FLASH_SOFTWARE_PROTECT
            ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
#endif
            osal_mutex_unlock(g_store_mutex);
        }
        else
        {
            osal_task_sleep(10);
        }
    }
}

BOOL vsc_task_init(void)
{
    T_CTSK t_ctsk;

    MEMSET(&t_ctsk, 0, sizeof(T_CTSK));
    t_ctsk.task = (FP)cas_vsc_task;
    t_ctsk.stksz = 0x6000;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.name[0] = 'V';
    t_ctsk.name[1] = 'S';
    t_ctsk.name[2] = 'C';

    g_vsc_tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == g_vsc_tsk_id)
    {
        ASSERT(0);
        while(1);
    }

    g_store_mutex = osal_mutex_create();
    if(OSAL_INVALID_ID==g_store_mutex)
    {
        ASSERT(0);
        while(1);
    }
    return TRUE;
}

RET_CODE api_mcas_vsc_init (void)
{
    UINT32 vsc_lib_addr = __MM_VSC_CODE_START_ADDR + 0x200;
    UINT32 vsc_lib_len = 0;
    UINT32 block_addr = 0;
    UINT32 chunk_id = 0x0CF30101;
    RET_CODE ret = RET_FAILURE;

    vsc_task_init();

    vsc_vdev_init ((RET_CODE*)nvram_data_fetch, (RET_CODE*)nvram_write_flash);

    // Fetch NVRAM & Transfer to SEE
    vsc_nvram_read_flash ();
    sto_get_chunk_len (chunk_id, &block_addr, &vsc_lib_len);
    ret = vsc_lib_init(&vsc_lib_addr, &vsc_lib_len);

    return ret;
}
#endif

#ifdef _C0700A_VSC_ENABLE_
#include <api/libcas/abv/abv_ca_types.h>
//#define VSC_API_PRINTF(...) do{} while(0)
#define VSC_API_PRINTF libc_printf

#define C_SECTOR_SIZE       0x10000

static UINT8 vsc_store_buffer[__MM_VSC_DATA_LEN]={0};
static OSAL_ID g_vsc_tsk_id = OSAL_INVALID_ID;
static OSAL_ID g_store_mutex = OSAL_INVALID_ID;
static BOOL g_write_flash = FALSE;
static UINT32 g_write_offset = 0;
static UINT32 g_write_len = 0;

BOOL write_vsc_done(void)        //for mp tool personalizer, wait write flash done to enable osal_dispatch_off
{
    return g_write_flash;
}

static void vsc_write_flash (void)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   //UINT8 index = 0;
   UINT8 *flash_buffer = NULL;
   RET_CODE ret = 0;
   //VSC_STORE_CONFIG config;

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   // step 1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1);
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN] Can't find key store chunk id 0x%08x.\n", \
                      KEY_STORE_ID);
        return;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;

   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X\n", \
                   __FUNCTION__, chunk_id, chunk_addr);

    // step 2. Write the data back to NVRAM
   flash_buffer = (UINT8 *)MALLOC(__MM_VSC_DATA_LEN);
    if (NULL == flash_buffer)
    {
        ASSERT(0);
        return ;//RET_FAILURE;
    }

   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, \
                         (UINT32)flash_buffer);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return ;//RET_FAILURE;
   }
   VSC_API_PRINTF ("[MAIN-%s] DRAM address used to store is 0x%X\n", \
                   __FUNCTION__, vsc_store_buffer);
   
   ret = sto_put_data (flash_dev, chunk_addr + g_write_offset, \
                       vsc_store_buffer+g_write_offset, g_write_len);

   if ((ret<0) || (g_write_len != (UINT32)ret))
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] sto_put_data FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }
   VSC_API_PRINTF ("[MAIN-%s] Write chunk offset is 0x%X, len is 0x%x, Return %d\n", \
                   __FUNCTION__, g_write_offset, g_write_len, ret);

   ret = sto_io_control (flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SECTOR_BUFFER FAIL\n", \
                      __FUNCTION__);
      return ;//RET_FAILURE;
   }
   ret = sto_io_control (flash_dev, STO_DRIVER_SET_FLAG, 0);
   if (RET_SUCCESS != ret)
   {
      FREE(flash_buffer);
      VSC_API_PRINTF ("[MAIN-%s] STO_DRIVER_SET_FLAG FAIL\n", __FUNCTION__);
      return ;//RET_FAILURE;
   }

   if (NULL != flash_buffer)
   {
      FREE (flash_buffer);
   }

   g_write_flash=FALSE;
}

static RET_CODE nvram_write_flash (UINT8 *mem_addr_main, VSC_STORE_CONFIG config, UINT32 offset, UINT32 len)
{
    //UINT32 tick1, tick2;

    osal_mutex_lock(g_store_mutex, OSAL_WAIT_FOREVER_TIME);
    MEMCPY (vsc_store_buffer, mem_addr_main, __MM_VSC_DATA_LEN);
    g_write_offset = offset;
    g_write_len = len;
    g_write_flash=TRUE;
    osal_mutex_unlock(g_store_mutex);

   return RET_SUCCESS;
}
static RET_CODE nvram_data_fetch (UINT8 * mem_addr_main, VSC_STORE_CONFIG *config, VSC_STORE_CONFIG *backup_config)
{
   struct sto_device *flash_dev = NULL;
   UINT32 chunk_id = 0;
   UINT32 chunk_addr = 0xffffffff;
   UINT32 ret = 0;

   if (NULL == mem_addr_main)
   {
      VSC_API_PRINTF ("[MAIN-%s-%d] illegal address\n",
                      __FUNCTION__, __LINE__);
      ASSERT (0);
   }

   flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

   MEMSET (mem_addr_main, 0x00, __MM_VSC_DATA_LEN);

   // step1. Read NVRAM data
   chunk_id = KEY_STORE_ID;
    chunk_addr = sto_chunk_goto(&chunk_id, 0xFFFF0000, 1) ;
   if (0 == chunk_addr)
   {
      VSC_API_PRINTF ("[MAIN-%s] Can't find key store chunk id 0x%08x.\n", \
                      __FUNCTION__, KEY_STORE_ID);
      return RET_FAILURE;
   }
    chunk_addr=chunk_addr+CHUNK_HEADER_SIZE;
    
   VSC_API_PRINTF ("[MAIN_%s] chunk 0x%X at Flash offset 0x%X, len 0x%x\n", \
                   __FUNCTION__, chunk_id, chunk_addr, __MM_VSC_DATA_LEN);

    ret = sto_get_data (flash_dev, mem_addr_main, chunk_addr,  __MM_VSC_DATA_LEN);
    if ((__MM_VSC_DATA_LEN) != ret)
      {
         VSC_API_PRINTF ("[MAIN-%s] sto_get_data FAIL\n", __FUNCTION__);
         return RET_FAILURE;
      }

   return RET_SUCCESS;
}

static void cas_vsc_task(void)
{
    while(1)
    {
        if(TRUE == g_write_flash)
        {
            osal_mutex_lock(g_store_mutex, OSAL_WAIT_FOREVER_TIME);
            vsc_write_flash();
            osal_mutex_unlock(g_store_mutex);
        }
        else
        {
            osal_task_sleep(10);
        }
    }
}

BOOL vsc_task_init(void)
{
    T_CTSK t_ctsk;

    MEMSET(&t_ctsk, 0, sizeof(T_CTSK));
    t_ctsk.task = (FP)cas_vsc_task;
    t_ctsk.stksz = 0x6000;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.name[0] = 'V';
    t_ctsk.name[1] = 'S';
    t_ctsk.name[2] = 'C';

    g_vsc_tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == g_vsc_tsk_id)
    {
        ASSERT(0);
        while(1);
    }

    g_store_mutex = osal_mutex_create();
    if(OSAL_INVALID_ID==g_store_mutex)
    {
        ASSERT(0);
        while(1);
    }
    return TRUE;
}

RET_CODE api_mcas_vsc_init (void)
{
    UINT32 vsc_lib_addr = __MM_VSC_CODE_START_ADDR + 0x200;
    UINT32 vsc_lib_len = 0;
    UINT32 block_addr = 0;
    UINT32 chunk_id = 0x0CF30101;
    RET_CODE ret = RET_FAILURE;

    ret = vsc_task_init();
    libc_printf("vsc_task_init 0x%x\n", ret);

    ret = vsc_vdev_init ((RET_CODE*)nvram_data_fetch, (RET_CODE*)nvram_write_flash);
    libc_printf("vsc_vdev_init 0x%x\n", ret);

    // Fetch NVRAM & Transfer to SEE
    ret = vsc_nvram_read_flash ();
    if(ret!=RET_SUCCESS)
        libc_printf("vsc_nvram_read_flash 0x%x\n", ret);
    
    sto_get_chunk_len (chunk_id, &block_addr, &vsc_lib_len);
    
    libc_printf("vsc_lib_addr 0x%x, vsc_lib_len 0x%x\n", vsc_lib_addr, vsc_lib_len);

    ret = vsc_lib_init(&vsc_lib_addr, &vsc_lib_len);
    if(ret!=RET_SUCCESS)
        libc_printf("vsc_lib_init ret %d\n", ret);
    return ret;
}

RET_CODE api_vsc_reset()
{
    UINT32 vsc_lib_addr = __MM_VSC_CODE_START_ADDR + 0x200;
    UINT32 vsc_lib_len = 0;
    UINT32 block_addr = 0;
    UINT32 chunk_id = 0x0CF30101;
    RET_CODE ret = RET_FAILURE;

    // Fetch NVRAM & Transfer to SEE
    ret = vsc_nvram_read_flash ();
    libc_printf("vsc_nvram_read_flash 0x%x\n", ret);
    sto_get_chunk_len (chunk_id, &block_addr, &vsc_lib_len);
    ret = vsc_lib_init(&vsc_lib_addr, &vsc_lib_len);
    libc_printf("api_vsc_reset 0x%x\n", ret);
    return ret;
}

ABV_Boolean ABV_STBCA_Send_MSG_CA2OtherSTB(ABV_UInt8 *pBuffer,ABV_UInt8 Length,ABV_UInt32 TimeOut)FUNC_TYPE
{
	return ABV_TRUE;
}
#endif
#endif

#if (defined(CAS9_VSC) && defined(CAS9_SC_UPG))
void key_store_erase(void)
{
    struct sto_device *flash_dev=NULL;
    UINT32 tmp_param[2];
    UINT32 param=0;
    UINT32 ret=0;
    UINT32 id=0;
    CHUNK_HEADER chunk_hdr;
    UINT32 key_flash_addr=0;
    UINT32 key_flash_size=0;
    VSC_STORE_CONFIG config;

    MEMSET (&config, 0, sizeof (VSC_STORE_CONFIG));
    ret=vsc_store_get_config(&config);
    if(RET_FAILURE==ret)
    {
        VSC_API_PRINTF("get vsc config failed\n");
        return;
    }
    VSC_API_PRINTF("vsc config.tag=0x%x, config.index=%d\n",config.tag,config.index);
    if((0xFF==config.index) || (VSC_TAG != config.tag))
    {
        VSC_API_PRINTF("Not VSC store\n");
        return;
    }

    id = KEY_STORE_ID;
    ret = sto_get_chunk_header( id, &chunk_hdr );

    if (0==ret)
    {
        VSC_API_PRINTF("init_key_in_memory_basis: sto_get_chunk_header fail\n");
        return RET_FAILURE;
    }

    key_flash_addr = (UINT32)sto_chunk_goto( &id, 0xFFFFFFFF, 1 )+ CHUNK_HEADER_SIZE;

    flash_dev =( struct sto_device *)dev_get_by_id( HLD_DEV_TYPE_STO, 0 );
    if ( flash_dev == NULL )
    {
        VSC_API_PRINTF("%s: can not find flash!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    key_flash_size=chunk_hdr.offset-CHUNK_HEADER_SIZE;
    VSC_API_PRINTF("key_flash_addr=0x%x, key_flash_size=0x%x\n",key_flash_addr,key_flash_size);
    tmp_param[0] = key_flash_addr;
    tmp_param[1] = ( key_flash_size >>10);
    param= (UINT32)tmp_param;

    key_store_write(1);
    if ( sto_io_control( flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, param )!= SUCCESS )
    {
        key_store_write(0);
        VSC_API_PRINTF("%s: erase error!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    key_store_write(0);
    //clear backup vsc config in system data
    MEMSET(&config, 0, sizeof(config));
    sys_data_set_back_up_vsc_config(config);
    sys_data_save (1);
}
#endif

#if 0//def _C0700A_VSC_ENABLE_
INT32 api_mcas_register(mcas_disp_func_t mcas_disp_func)
{
}

INT32 api_mcas_stop_service(UINT32 mon_id)
{
}

INT32 api_mcas_start_service(UINT32 channel_index, UINT32 mon_id)
{
}

INT32 api_mcas_start_transponder(void)
{
}

INT32 api_mcas_stop_transponder(void)
{
}
#endif
