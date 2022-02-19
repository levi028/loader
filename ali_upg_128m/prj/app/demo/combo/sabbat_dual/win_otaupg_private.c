#if 0//new or change add by yuj
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg_private.c
*
*    Description:   The internal function of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <bus/dog/dog.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <api/libota/lib_ota.h>
#include <hld/decv/decv.h>
#include <api/libosd/osd_lib.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/lib_epg.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <sys_parameters.h>
#include <api/libstbinfo/stb_info_data.h>
#include <hld/crypto/crypto.h>
#include <hld/dsc/dsc.h>
#include <api/librsa/flash_cipher.h>
#include <api/librsa/rsa_verify.h>
#ifdef AUTO_OTA
#include <hld/pan/pan_dev.h>
#endif
#ifdef _BC_CA_ENABLE_
//#include <api/libcas/bc/bc_types.h>
#endif
#ifdef _BC_CA_NEW_
#include <hld/crypto/crypto.h>
#define _SIG_SEE                0x4     /**<Signature verification flag */
#define _DECRYPT_SEE            0x1     /**<Decryption flag */
#define _UZIP_SEE               0x2     /**<Unzip flag */
#define DUS_SEE    ((_DECRYPT_SEE <<28) | (_UZIP_SEE << 24)| \
                     (0xf <<20) | (_SIG_SEE <<16 ))
#define SEE_ENTRY               (__MM_PRIVATE_ADDR + 0x200)
static UINT8 see_rsa_pub_key[516]={0};
#define RSASSA_PKCS1_V1_5       0       
#endif
#include <api/libota/lib_ota.h>
#include <bus/otp/otp.h>
#include <boot/boot_common.h>

#include "control.h"
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_otaupg.h"
#include "win_ota_set.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "ota_ctrl.h"
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
#ifdef _NV_PROJECT_SUPPORT_ 
#include <udi/nv/ca_upg.h>
#include <udi/nv/ca_mmap.h>
#ifdef SSD_DEBUG
#include <udi/nv/ssassdd.h>
#else
#include <udi/nv/ssassd.h>
#endif
#endif
#include "win_otaupg_private.h"
#include "ctrl_util.h"
#include "key.h"
#include "power.h"

#if defined (_CAS9_CA_ENABLE_)
#include <api/libchunk/chunk_hdr.h>
#endif

#ifdef _SMI_ENABLE_
#include "smi_ap/smi_api.h"
extern RET_CODE test_cust_rsa_ram(const UINT32 addr, const UINT32 len, UINT8 *public_key);
#endif

#define OTA_FW_CHIP_ID      0x36060000

static UINT32 old_process = 0;

#ifdef _NV_PROJECT_SUPPORT_ 
static UINT32 ota_upgrade_size = 0;;
#endif
#ifdef _BC_CA_NEW_
extern BOOL loader_set_run_parameter(BOOL set,BOOL type);
extern BOOL loader_check_run_parameter();
extern RET_CODE generate_bc_see_key(struct UKEY_INFO *ukey);
extern RET_CODE get_see_root_pub_key(UINT8 *pk,UINT32 len);
extern RET_CODE test_cust_rsa_ram(const UINT32 addr, const UINT32 len, UINT8 *public_key);
//extern void mg_setup_crc_table(void);
//extern unsigned int mg_table_driven_crc(register unsigned int crc,
//                register unsigned char *bufptr,
//                register int len);
#endif
#ifdef _BC_CA_ENABLE_
extern UINT32 str2uint32(UINT8 *str, UINT8 len);
extern void mg_setup_crc_table(void);
extern unsigned int mg_table_driven_crc(register unsigned int crc,
                register unsigned char *bufptr,
                register int len);
#endif

#ifdef _BUILD_LOADER_COMBO_
extern UINT8 loader_check_osm_parameter();
#endif

#ifdef _SMI_ENABLE_
UINT8 ca_switch_mode = 0;
UINT8 g_cust_public_key[516]={0};
UINT8 g_ecgk_key[32]={0};
#endif

static INT32 init_storage(void)
{
    otaupg_sto_device = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if(NULL == otaupg_sto_device)
    {
        OTA_PRINTF("%s: can not get sto device\n", __FUNCTION__);
        return ERR_NO_DEV;
    }

    INT32 er = 0;

    er = sto_open(otaupg_sto_device);
    if(SUCCESS != er)
    {
        OTA_PRINTF("%s: can not open sto device\n", __FUNCTION__);
    }


    return er;
}

static RET_CODE verify_dynamic_hdr_sig_from_memory()
{
    RET_CODE ret = RET_FAILURE;
    UINT32 chunk_hdr = 0;
    UINT32 chunk_data = 0;
    UINT32 chunk_data_len = 0;
    UINT32 data_base = 0;
    UINT32 stb_chunk_id=STB_CHUNK_STBINFO_HDR_ID;
    UINT32 chunk_cnt=0;
    UINT32 chunk_cnt2=0;   
    UINT32 chunklist_base = 0;
    UINT32 chunklist_len=0;    
    UINT32 header_base = 0;    
    UINT32 header_len=0;
    UINT32 dwi=0;
    UINT8  chunk_match=FALSE;
    UINT32 start_cnt =0;//flag stb info hdr chunk pos
    UINT32 tmp_addr= 0;
	UINT32 boot_total_area_len = 0;
    m_ota_info.uncompressed_len = \
    (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);
    OTA_PRINTF("ota uncompressed_len = 0x%08x\n", m_ota_info.uncompressed_len);
    get_boot_total_area_len(&boot_total_area_len);
    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len,m_ota_info.uncompressed_len-boot_total_area_len);
    chunk_hdr = (UINT32)chunk_goto(&stb_chunk_id, 0xFFFFFFFF, 1);
    if(((UINT32)ERR_PARA == chunk_hdr)||((UINT32)ERR_FAILUE== chunk_hdr))
    {
        return RET_FAILURE;
    }
    chunk_cnt=chunk_count(0, 0);
    if(chunk_cnt==0)
        return RET_FAILURE;
    chunklist_len=chunk_cnt* sizeof(CHUNK_LIST);      
    chunklist_base=(UINT32) MALLOC (chunklist_len);
    if(0==chunklist_base)
    {
        return RET_FAILURE;
    }        
	chunk_cnt2=get_chunk_list((CHUNK_LIST *)chunklist_base);
    if(chunk_cnt2==0)
    {
        FREE((UINT8 *)chunklist_base);  
        return RET_FAILURE;
    }   
    header_len=chunk_cnt2* CHUNK_HEADER_SIZE+0x100;//add sig len
    header_base=(UINT32) MALLOC (header_len);
    if(0==header_base)
    {
        FREE((UINT8 *)chunklist_base);  
        return RET_FAILURE;
    }
    for(dwi=0;dwi<chunk_cnt2;dwi++)
	{
	    tmp_addr = (UINT32)chunk_goto(&(((CHUNK_LIST *)chunklist_base+dwi)->id),0xFFFFFFFF, 1);
        if(((UINT32)ERR_PARA == tmp_addr)||((UINT32)ERR_FAILUE== tmp_addr))
        {                                
			ret = RET_FAILURE;
            goto Exit;
		}
		MEMCPY((UINT8 *)(header_base + dwi*CHUNK_HEADER_SIZE),(UINT8 *)tmp_addr,CHUNK_HEADER_SIZE);
        if(stb_chunk_id==((CHUNK_LIST *)chunklist_base+dwi)->id)
	    {
			start_cnt=dwi;
            chunk_match = TRUE;
		}
    }
    if(chunk_match== FALSE)
    {
        ret = RET_FAILURE;
        goto Exit;//no stb info hdr chunk
    }
    chunk_data = chunk_hdr;
    data_base = header_base+header_len-0x100;
    chunk_data_len = (chunk_cnt2-start_cnt)*CHUNK_HEADER_SIZE+0x100;
    MEMCPY((UINT8 *)data_base,(UINT8 *)(chunk_data+0x80),0x100);
    data_base = header_base + start_cnt*CHUNK_HEADER_SIZE;
    if (0 == test_rsa_ram((UINT32)data_base, chunk_data_len))
    {
		libc_printf("verify_dynamic_hdr_sig success\n");
        ret = RET_SUCCESS;
    }
    else
    {
		libc_printf("verify_dynamic_hdr_sig failed\n");
        ret = RET_FAILURE;
    }
Exit:     
    FREE((UINT8 *)chunklist_base);
	FREE((UINT8 *)header_base); 
    return ret;
}

static BOOL setup_ota_parameters(void)
{
    UINT32 addr=0;
    UINT32 len=0;
    UINT32 boot_total_area_len = 0;
    UINT32 __MAYBE_UNUSED__ ouc_ck_id = 0;
    UINT32 ota_para_chid = CHUNKID_OTA_PARA;
    UINT32 ota_loader_size = 0;
    UINT32 __MAYBE_UNUSED__ ouc_f_addr = 0;
    UINT8 *p = NULL;
    struct sto_device *flash_dev __MAYBE_UNUSED__= NULL;
    RET_CODE __MAYBE_UNUSED__ ret = 0 ;
    
	if(0 == ota_loader_size)
	{
		;
	}
	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    // get unzip size;
    m_ota_info.uncompressed_len = \
    (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);
    OTA_PRINTF("ota uncompressed_len = 0x%08x\n", m_ota_info.uncompressed_len);

    get_boot_total_area_len(&boot_total_area_len);

    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len,m_ota_info.uncompressed_len-boot_total_area_len);
    if(!get_chunk_add_len(0x04FB0100, &addr, &len))
    {
        return FALSE;
    }
    m_ota_info.user_db_addr_offset = addr-m_ota_info.uncompressed_addr;
    m_ota_info.user_db_len = len;
    m_ota_info.user_db_start_sector\
     = m_ota_info.user_db_addr_offset/C_SECTOR_SIZE;
    m_ota_info.user_db_sectors = m_ota_info.user_db_len/C_SECTOR_SIZE;

    if(m_ota_info.user_db_len%C_SECTOR_SIZE)
    {
    //    ASSERT(0);
        m_ota_info.user_db_sectors++;
    }
    // find the firmware position
    m_ota_info.ota_fw_addr = m_ota_info.uncompressed_addr;
    m_ota_info.ota_fw_size = addr + len - m_ota_info.ota_fw_addr;

#ifdef _OUC_LOADER_IN_FLASH_
    // find ota.bin position.  
    ouc_ck_id = OUC_CHUNK_ID;
    p = (UINT8*)chunk_goto(&ouc_ck_id, 0xFFFFFFFF, 1);    
    
    if(p)
    {    
        ota_loader_size = fetch_long(p + CHUNK_OFFSET);
        m_ota_info.ota_bin_addr = (UINT32)(p + 0x80 );//m_ota_info.ota_fw_addr + m_ota_info.ota_fw_size;
        m_ota_info.ota_bin_size = fetch_long(p + CHUNK_OFFSET) -0x80;// - C_SECTOR_SIZE;//exclude the param
        m_ota_info.ota_bin_sectors = m_ota_info.ota_bin_size/C_SECTOR_SIZE;        
        if(m_ota_info.ota_bin_size % C_SECTOR_SIZE)
        {
            m_ota_info.ota_bin_sectors++;
        }

        sto_chunk_init(boot_total_area_len, flash_dev->totol_size);
        ouc_f_addr = sto_chunk_goto(&ouc_ck_id, 0xFFFFFFFF, 1);

        if(ouc_f_addr > 0)
        {
            m_ota_info.ota_upg_addr = ouc_f_addr + CHUNK_HEADER_SIZE;
            m_ota_info.ota_upg_size = sto_fetch_long(ouc_f_addr+CHUNK_OFFSET) - 0x80;// - C_SECTOR_SIZE;//exclude the param
            m_ota_info.ota_upg_sectors = m_ota_info.ota_upg_size/C_SECTOR_SIZE; 
            if(m_ota_info.ota_upg_size % C_SECTOR_SIZE)
            {
                m_ota_info.ota_upg_sectors++;
            }

            //m_ota_info.ouc_para_f_offset = ouc_get_param_f_addr();
             m_ota_info.ouc_para_f_offset = sto_chunk_goto(&ota_para_chid, 0xFFFFFFFF, 1)+ CHUNK_HEADER_SIZE;  //flash protection redefine chunk for ota_para
        }
        else
        {
            m_ota_info.ota_upg_addr = (UINT32)p - m_ota_info.uncompressed_addr + CHUNK_HEADER_SIZE;
            m_ota_info.ota_upg_size = m_ota_info.ota_bin_size;
            m_ota_info.ota_upg_sectors = m_ota_info.ota_bin_sectors ; 
        }

        if(m_ota_info.ota_upg_sectors < m_ota_info.ota_bin_sectors)
        {
            OTA_PRINTF("OUC loader space is smaller than the size in memory\n");
            return FALSE;
        }

        if(m_ota_info.ota_upg_addr % C_SECTOR_SIZE)
        {
            OTA_PRINTF("OUC loader don't align with 64K, please check\n");
            return FALSE;
        }
        
    }
    else
    {
        OTA_PRINTF("Cannot find the OUC loader in upgrade ABS!!!\n");
    }
#else
    // find ota.bin position.
    m_ota_info.ota_bin_addr = m_ota_info.ota_fw_addr + m_ota_info.ota_fw_size;
    m_ota_info.ota_bin_size = \
    m_ota_info.uncompressed_len - m_ota_info.ota_fw_size;
    m_ota_info.ota_bin_sectors = m_ota_info.ota_bin_size/C_SECTOR_SIZE;
    if(m_ota_info.ota_bin_size % C_SECTOR_SIZE)
    {
        m_ota_info.ota_bin_sectors++;
    }
#endif
  

#if defined(_RETAILER_PROJECT_)    
    // calculate upgrade sectors.
    UINT32 bloader_offset=0;
    UINT32 bl_sector=0;
    UINT32 bl_count=0;
    //add by colin: used for count the block of ciplus key
    bloader_offset = ((UINT32)(*((UINT8 *)m_ota_info.ota_fw_addr+8)) << 24)\
        | ((UINT32)(*((UINT8 *)m_ota_info.ota_fw_addr + 9)) << 16) \
        | ((UINT32)(*((UINT8 *)m_ota_info.ota_fw_addr + 10)) << 8)\
    | ((UINT32)(*((UINT8 *)m_ota_info.ota_fw_addr + 11)) << 0);

    bl_sector = bloader_offset / C_SECTOR_SIZE;
    if (NUM_ZERO != bloader_offset % C_SECTOR_SIZE)
    bl_sector ++;   //  Colin>>This sentence has already skipped the HDCP key
    if (ci_flag)
    {
         bl_count=0x20000/C_SECTOR_SIZE;
         if (NUM_ZERO != 0x20000 % C_SECTOR_SIZE)
                    bl_count++;
         bl_sector+=bl_count;
    }
#if defined(_BUILD_OTA_E_) && defined(_OUC_LOADER_IN_FLASH_)    
    bl_sector += m_ota_info.ota_bin_sectors ;
#endif

    //skip bootloader, hdcp, (and ciplus sector.) , loader.
    m_ota_info.ota_upg_addr = m_ota_info.ota_fw_addr \
    + bl_sector * C_SECTOR_SIZE; 
    //skip bootloader and user_db
    m_ota_info.ota_upg_size = m_ota_info.ota_fw_size \
    - len - bl_sector * C_SECTOR_SIZE;    
    m_ota_info.ota_upg_sectors = m_ota_info.ota_upg_size/C_SECTOR_SIZE;
    if(m_ota_info.ota_upg_size % C_SECTOR_SIZE)
        m_ota_info.ota_upg_sectors++;

    m_ota_info.update_secotrs_index = 0;
#ifdef _OUC_LOADER_IN_FLASH_
    m_ota_info.update_total_sectors = m_ota_info.ota_upg_sectors;
#else
#ifdef _BUILD_OTA_E_
    m_ota_info.update_total_sectors = m_ota_info.ota_upg_sectors + \
        m_ota_info.user_db_sectors;
#else
    m_ota_info.update_total_sectors = m_ota_info.ota_upg_sectors + \
        m_ota_info.ota_bin_sectors + m_ota_info.user_db_sectors;
#endif
#endif
#endif

    // find ota_cfg chunk.    
    #ifdef _OUC_LOADER_IN_FLASH_
    //OTA loader is IN the FW.
    #if defined(_SMI_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_RETAILER_PROJECT_)
    //For SMI, and VMX Plus SC1.5, RET
    //param within loader.
    chunk_init(m_ota_info.ota_bin_addr,m_ota_info.uncompressed_len-boot_total_area_len); //flash protection ota_para depend
    #else   
    //For CAS9/GCA.
    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len,m_ota_info.uncompressed_len-boot_total_area_len); //flash protection ota_para depend
    #endif
    #else
    //OTA loader is NOT IN the FW.
    chunk_init(m_ota_info.ota_bin_addr,m_ota_info.ota_bin_size); 
    #endif
    p = (UINT8*)chunk_goto(&ota_para_chid, 0xFFFFFFFF, 1);
    if(p)
    {
        m_ota_info.ota_bin_cfg_addr = (UINT32)p;
        #ifdef _OUC_LOADER_IN_FLASH_
        m_ota_info.ouc_para_m_offset = (UINT32)p - m_ota_info.uncompressed_addr;
        #endif
        ota_save_parameter(p);
    }
#ifdef _OUC_LOADER_IN_FLASH_
    else
    {
        OTA_PRINTF("Cannot find the OUC param in Memory\n");
        return FALSE;        
    }
#endif

#if 0//defined(_CAS9_CA_ENABLE_) && !defined(_BUILD_OTA_E_)
    ret = update_otaloader_hmac(m_ota_info.ota_bin_addr,m_ota_info.ota_bin_size);
    if(ret != RET_SUCCESS)
    {
        OTA_PRINTF("Update OTALODER chunk hmac failed\n");
        return FALSE;
    }
#endif

    return TRUE;
}



#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
#ifdef _CAS9_VSC_ENABLE_
BOOL is_upgrade_vsc (UINT8 *vsc_m_ver, UINT8 *vsc_f_ver)
{
    UINT8 i = 0;
	
    for (i = 0; i < VSC_VERSION_LEN; i++)
	{
		if (vsc_m_ver [i] > vsc_f_ver [i])
		{
			return TRUE;
		}
	}
    return FALSE;
}
#endif
static int win_otaupg_burnflash_pre(struct otaupg_ver_info *ver_info)
{
    INT32 int_stroge_ret=0;
#if 1//defined(_M3383_SABBAT_) || defined(_ENABLE_4M_FLASH_)
    BOOL ota_set_para_ret=FALSE;
#endif
    BOOL ota_check_ret=FALSE;
    win_popup_choice_t popup_choice=WIN_POP_CHOICE_NULL;
    UINT32 upgrade_size = 0;
    UINT8 back_saved=0;

	if(WIN_POP_CHOICE_NULL == popup_choice)
	{
		;
	}
	if(0 == int_stroge_ret)
	{
		;
	}
	int_stroge_ret=init_storage();
#if 1//defined(_M3383_SABBAT_) || defined(_ENABLE_4M_FLASH_)
#ifdef _CAS9_CA_ENABLE_
    if( RET_SUCCESS != verify_dynamic_hdr_sig_from_memory())
    {
       OTA_PRINTF(" Dynamic data (from stream) sig verify Fail!\n");
       win_compopup_init(WIN_POPUP_TYPE_OK);
       win_compopup_set_msg("Dynamic data (from stream) sig verify Fail!", NULL, 0);
       win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
       popup_choice=win_compopup_open_ext(&back_saved);
       osal_flag_set(g_ota_flg_id, PROC_FAILURE);
       return -1;
     }
#endif
    ota_set_para_ret=setup_ota_parameters();
    if(FALSE == ota_set_para_ret)
    {
        OTA_PRINTF("setup_ota_parameters Failed \n");
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Setup OTA/OUC parameter Failed !", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        popup_choice=win_compopup_open_ext(&back_saved);
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
#endif
#ifdef _NOVEL_ENABLE_
    //GCA + NOVEL BL Flow.
    UINT8 key[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,};
    UINT8 iv[32];
    MEMSET(iv,0,sizeof(iv));
    set_key_iv(key,iv);
    ota_check_ret=otaupg_check_fixed_key(ver_info);
#else
    ota_check_ret=otaupg_check(ver_info);
#endif
    if(FALSE == ota_check_ret)
    {
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }

    #ifdef OTA_NO_VERION_CHK //just for no version verify
    ver_info->m_f_ver = 1;
    ver_info->m_m_ver = 2;
    ver_info->o_b_ver = 1;
    ver_info->o_f_ver = 1;
    ver_info->o_m_ver = 2;
    ver_info->s_f_ver = 1;
    ver_info->s_m_ver = 2;
    #endif
    #ifdef _OUC_LOADER_IN_FLASH_
    if(
        (0 == ver_info->o_m_ver) ||(0 == ver_info->os_m_ver) ||\
        (0 == ver_info->m_m_ver ) || (0 == ver_info->s_m_ver) ||\
        (ver_info->m_m_ver != ver_info->s_m_ver)
        #ifdef _MOD_DYNAMIC_LOAD_
        || (0 == ver_info->dyn_m_ver)
        #endif
        )
    #else
    if (
        (0 == ver_info->o_m_ver) ||
        (0 == ver_info->m_m_ver ) ||
        (0 == ver_info->s_m_ver) ||
        (ver_info->m_m_ver != ver_info->s_m_ver))
    #endif
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Invalid OTA data, cannot burn it!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        popup_choice=win_compopup_open_ext(&back_saved);
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }


#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_) &&\
    !defined(_OUC_LOADER_IN_FLASH_)
    if (ver_info->o_m_ver > ver_info->o_f_ver)
        upgrade_size += m_ota_info.ota_loader_size;
#endif

    if (ver_info->m_m_ver > ver_info->m_f_ver)
    {
        upgrade_size += m_ota_info.main_code_size;
    }

    if (ver_info->s_m_ver > ver_info->s_f_ver)
    {
        upgrade_size += m_ota_info.see_code_size;
    }
#ifdef _CAS9_VSC_ENABLE_
    // if(ver_info->vsc_m_ver > ver_info->vsc_f_ver)
    if(TRUE == is_upgrade_vsc (ver_info->vsc_m_ver, ver_info->vsc_f_ver))
        upgrade_size += m_ota_info.vsc_code_size;
#endif

#ifdef _OUC_LOADER_IN_FLASH_
#ifndef _BUILD_OTA_E_
    if(ver_info->o_m_ver > ver_info->o_f_ver || ver_info->os_m_ver > ver_info->os_f_ver)
    {
        upgrade_size += m_ota_info.ota_bin_size;
    }
#endif
#endif
#ifdef _MOD_DYNAMIC_LOAD_
    if(ver_info->dyn_m_ver > ver_info->dyn_f_ver)
        upgrade_size += m_ota_info.dyn_code_size;
#endif

#if defined( _BUILD_OTA_E_) && defined(_CAS9_CA_ENABLE_)
    if (NUM_ZERO==upgrade_size)
    {
        //need to update hmac.
        return 0;
    }
#endif

    // Oncer Yu 20110725:
    // As M3603 Mode, need to check OTALoader/MainCode/SeeCode
    // but M3383 mode, neet to check MainCode/SeeCode firstly
    // if need to do upgrade, then do OTALoader check
    if (NUM_ZERO==upgrade_size)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Nothing need to be upgrade!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        popup_choice=win_compopup_open_ext(&back_saved);
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_) &&\
    !defined(_OUC_LOADER_IN_FLASH_)
    else if (ver_info->o_f_ver > ver_info->o_b_ver)
    {
        upgrade_size += OTA_LOADER_BACKUP_LEN + DECRYPT_KEY_BACKUP_LEN;
    }
#else
#ifdef _OUC_LOADER_IN_FLASH_
    else if(ver_info->o_m_ver == ver_info->o_f_ver \
        && ver_info->os_m_ver == ver_info->os_f_ver)
    {
        #ifndef _BUILD_OTA_E_
        upgrade_size += C_SECTOR_SIZE;// OUC param
        #endif
    }
#else
    else if(ver_info->o_m_ver > ver_info->o_f_ver)
    {
    #ifndef _BUILD_OTA_E_
        upgrade_size += m_ota_info.ota_loader_size;
        upgrade_size += m_ota_info.user_db_len;
    #endif
    }
    else
    {
        // if enter here, that means OTA Loader Invalid!!!
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("OTA Invalid!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        popup_choice=win_compopup_open_ext(&back_saved);
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
#endif
#endif

    m_ota_info.update_total_sectors = upgrade_size / C_SECTOR_SIZE;
    if(upgrade_size%C_SECTOR_SIZE)
    {
        m_ota_info.update_total_sectors++;
    }
    m_ota_info.update_secotrs_index = 0;
    return 0;
}

//CAS9
INT32 win_otaupg_burnflash(void)
{
#ifdef _VERSION_CONTROL_
    UINT8 back_saved=0;
    __MAYBE_UNUSED__ win_popup_choice_t popup_choice=WIN_POP_CHOICE_NULL;
#endif
    struct otaupg_ver_info ver_info;
    UINT32 chunk_id=0;
    UINT32 chunk_addr=0;
    UINT32 chunk_size=0;
    INT32 ret = 0;
    UINT32 flash_pos=0;

    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_BURNFLASH;
    MEMSET(&ver_info,0,sizeof(struct otaupg_ver_info));

    osal_flag_clear(g_ota_flg_id, OTA_FLAG_PTN);
    if(win_otaupg_burnflash_pre(&ver_info) != 0)
    {
        return -1;
    }

    do
    {
#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_) \
    && !defined(_OUC_LOADER_IN_FLASH_)

        if (ver_info.o_f_ver > ver_info.o_b_ver)
        {
            // backup ota loader and universal key
            OTA_PRINTF("backup ota loader and universal key\n");
            struct sto_device *flash_dev = NULL;
            flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
            chunk_id = OTA_LOADER_CHUNK_ID;
            chunk_size = OTA_LOADER_BACKUP_LEN;
            chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
            if ((UINT32)ERR_FAILUE == chunk_addr)
            {
                ret = ERR_FAILED;
                break;
            }
            if ((UINT32)(sto_get_data(flash_dev, (UINT8 *)m_ota_info.cipher_buf_addr, chunk_addr, chunk_size)) == chunk_size)
            {
                flash_pos = OTA_LOADER_BACKUP_ADDR / C_SECTOR_SIZE;
                ret = ota_upg_burn_block((UINT8 *)m_ota_info.cipher_buf_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
                if (SUCCESS != ret)
                    break;
            }

            chunk_id = DECRPT_KEY_ID;
            chunk_size = DECRYPT_KEY_BACKUP_LEN;
            chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
            if ((UINT32)ERR_FAILUE == chunk_addr)
            {
                ret = ERR_FAILED;
                break;
            }
            if ((UINT32)(sto_get_data(flash_dev, (UINT8 *)m_ota_info.cipher_buf_addr, chunk_addr, chunk_size)) == chunk_size)
            {
                flash_pos = DECRYPT_KEY_BACKUP_ADDR / C_SECTOR_SIZE;
                ret = ota_upg_burn_block((UINT8 *)m_ota_info.cipher_buf_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
                if (SUCCESS != ret)
                    break;
            }
        }
#endif

#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_)\
    && !defined(_OUC_LOADER_IN_FLASH_)
        // ota loader chunk addr and size
        chunk_id = OTA_LOADER_CHUNK_ID;
        chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if ((UINT32)ERR_FAILUE == chunk_addr)
        {
            OTA_PRINTF("Cannot find ota loader's chunk!\n");
            ret = ERR_FAILED;
            break;
        }

        chunk_size = m_ota_info.ota_loader_size;
        if (ver_info.o_m_ver > ver_info.o_f_ver)
        {
            OTA_PRINTF("upgrade ota loader\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;
            ret = ota_upg_burn_block((UINT8 *)m_ota_info.ota_loader_addr,flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
                break;
        }
#else

#ifndef _BUILD_OTA_E_
#ifdef _OUC_LOADER_IN_FLASH_
        // TODO: step 1: burn OUC loader
        UINT8* ouc_para_addr = NULL;
        UINT32 ouc_sector_start = 0;
        
				 #ifdef FLASH_SOFTWARE_PROTECT
		         ap_set_flash_lock_len(KEY_AREA_END_ADDR);
         #endif
        if(ver_info.o_m_ver > ver_info.o_f_ver || ver_info.os_m_ver > ver_info.os_f_ver)
        {
            OTA_PRINTF("Burn OUCLOADER!\n");
            ret = ota_upg_burn_block((UINT8*)m_ota_info.ota_bin_addr, m_ota_info.ota_upg_addr/C_SECTOR_SIZE, m_ota_info.ota_bin_sectors);
            if(SUCCESS != ret)
            {
                break;
            }          
        }
       // else
       // {
            ouc_para_addr = (UINT8*)(m_ota_info.uncompressed_addr + m_ota_info.ouc_para_m_offset);//m_ota_info.ota_bin_addr + (m_ota_info.ota_bin_sectors-1) * C_SECTOR_SIZE;
            if(m_ota_info.ouc_para_f_offset)
            {
                ouc_sector_start = m_ota_info.ouc_para_f_offset/C_SECTOR_SIZE;
            }
            else
            {
                ouc_sector_start = m_ota_info.ouc_para_m_offset/C_SECTOR_SIZE;
            }

            if(m_ota_info.ouc_para_m_offset && ouc_sector_start)
            {
                OTA_PRINTF("Burn OUCLOADER Param \n");
                ret = ota_upg_burn_block(ouc_para_addr+CHUNK_HEADER_SIZE, ouc_sector_start, 1);
                #ifdef _CAS9_CA_ENABLE_
                update_otapara_hmac(ouc_para_addr+CHUNK_HEADER_SIZE,0x1008);
                #endif

                if(SUCCESS != ret)
                {
                    break;
                }  
            }
        //}              
#else
        // TODO: step 1: backup UserDB
        OTA_PRINTF("BackupDataBase!\n");
        ret = ota_upg_backup_database();
        if (SUCCESS != ret)
        {
            break;
        }

        // TODO: step 2: burn OTA to UserDB
        OTA_PRINTF("Burn OTALOADER!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.ota_bin_addr, m_ota_info.user_db_start_sector, m_ota_info.ota_bin_sectors);
        if(SUCCESS != ret)
        {
            break;
        }
#endif
#endif

        // TODO: step 3: calculate maincode chunk_addr
        chunk_id = MAINCODE_ID;
        chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if ((UINT32)ERR_FAILUE == chunk_addr)
        {
            OTA_PRINTF("Cannot find maincode's chunk!\n");
#ifdef _BUILD_OTA_E_
            //when cannot find maincode chunk,we use maincode chunk addr in mem.
            if((NUM_ZERO != m_ota_info.main_code_addr ) && (NUM_ZERO != m_ota_info.ota_fw_addr))
                chunk_addr = m_ota_info.main_code_addr - m_ota_info.ota_fw_addr;
            if(chunk_addr <= NUM_ZERO)
            {
                ret = ERR_FAILED;
                break;
            }
#else
            ret = ERR_FAILED;
            break;
#endif
        }
        chunk_size = 0;
#endif

        // burn new maincode.bin
        // main code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.main_code_size;
        if (ver_info.m_m_ver > ver_info.m_f_ver)
        {
            OTA_PRINTF("upgrade main code\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;
       
            ret = ota_upg_burn_block((UINT8 *)m_ota_info.main_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
            {
                break;
            }
        }

        // see code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.see_code_size;
        if (ver_info.s_m_ver > ver_info.s_f_ver)
        {
            OTA_PRINTF("upgrade see code\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;         
            ret = ota_upg_burn_block((UINT8 *)m_ota_info.see_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
            {
                break;
            }
        }
#ifdef _CAS9_VSC_ENABLE_
        // vsc code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.vsc_code_size;
		// bing 20150326 correct version compare for VSC
        if(TRUE == is_upgrade_vsc (ver_info.vsc_m_ver, ver_info.vsc_f_ver))
        {
            OTA_PRINTF("upgrade vsc code\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;                   
            ret = ota_upg_burn_block((UINT8 *)m_ota_info.vsc_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
                break;
        }
#endif

#ifdef _MOD_DYNAMIC_LOAD_
        // dyn code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.dyn_code_size;
        if (ver_info.dyn_m_ver > ver_info.dyn_f_ver)
        {
            OTA_PRINTF("upgrade dyn code\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;                   
            ret = ota_upg_burn_block((UINT8 *)m_ota_info.dyn_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
                break;
        }
#endif

#ifdef _VERSION_CONTROL_
        if(!version_update(ver_info.m_m_ver,SW_VER))
        {
            OTA_PRINTF("update code version fail\n");
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg("Update version fail!", NULL, 0);
            popup_choice=win_compopup_open_ext(&back_saved);
            while(1);
        }
#endif

#if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
		//update_allhdr_hmac(FALSE,0, NULL, 0);
		update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
#endif

#ifndef _OUC_LOADER_IN_FLASH_
#if defined( _M3383_SABBAT_ ) ||defined(_ENABLE_4M_FLASH_)
    #ifndef _BUILD_OTA_E_
        // TODO: burn back UserDB
        OTA_PRINTF("BurnBack UserDB!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.backup_db_addr,\
          m_ota_info.user_db_start_sector, m_ota_info.user_db_sectors);
        if(SUCCESS != ret)
        {
            break;
        }
    #endif
#endif
#endif
    } while(0);
    win_ota_upgrade_process_update_old(OTA_UPDATE_PROGRESS); // make sure to display 100% at last

    #if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
    //update_allhdr_hmac(FALSE,0, NULL, 0);
    update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
    #endif
    osal_flag_set(g_ota_flg_id,(SUCCESS == ret) ? PROC_SUCCESS : PROC_FAILURE);
    return ret;
}

#else /* _CAS9_CA_ENABLE_ */
#ifdef _BC_CA_STD_ENABLE_
int verify_signature_std( UINT8* src, UINT8* dest, CHUNK_HEADER hdr,UINT8 *error_code)
{
    FIXED_PRINTF("Verify chunk (0x%x)!\n",hdr.id);
    int ret=0,val=0;
    *error_code=1;
    UINT8 kid=0;
    if(NO_CRC!=hdr.crc)
    {
        kid=1;    //common key
        FIXED_PRINTF("Use Common Key!\n");
    }
    else
    {
        FIXED_PRINTF("Use AES Key!\n");
    }
    aes_cbc_std(0,kid,src,dest);
    fetch_sys_pub_key(1);
    val=test_rsa_ram((UINT32)dest+CHUNK_HEADER_SIZE,\
        hdr.len-CHUNK_HEADER_SIZE + CHUNK_NAME);
    if(NUM_ZERO!=val)      //case of no valid signature
    {
        if(-2==val) //Special case during MP (No RSA Key)
        {
            FIXED_PRINTF("Skip RSA Check, due to Zero RSA Key!\n");
            ret=1;
            *error_code=0;
        }
        else
        {
            FIXED_PRINTF("No valid signature!\n");
            if(NUM_ONE==kid)
            {
                *error_code = 3; //Need hang up, since check fail with common key
            }
            else
            {
                *error_code = 1;
            }
            return ret;
        }
    }
    //valid signature
    if(NUM_ZERO==kid)
    {
        ret=1;
        *error_code=0;
        FIXED_PRINTF("Success to check chunk.\n");
    }
    else if(NUM_ONE==kid)
    {
        MEMSET(src,0xFF,hdr.offset);
        val=aes_cbc_std(1,0,dest,src);
        if(NUM_ZERO==val)
        {
            *error_code=2;   //case of need to store reEnc code to flash
            FIXED_PRINTF("Need to store data to flash!\n");
        }
        else if(-2==val)    //special case during MP Flow (No AES Key)
        {
            *error_code=0;
            ret=1;
            FIXED_PRINTF("Spcial case during MP !\n");
        }
    }
    return ret;
}

BOOL otaupg_check(struct otaupg_ver_info *ver_info)
{
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0;
    UINT8 *data;
    UINT8 *block_addr;
    int ret;

    UINT8* tmp_buffer;
    UINT32 len, max_len;
    UINT8 error_code;
    CHUNK_HEADER chunk_header;
    UINT8 verify_result=0;
    UINT8 error_type=2;

    // get unzip size
    m_ota_info.uncompressed_len = \
    (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);
    chunk_init(m_ota_info.uncompressed_addr, m_ota_info.uncompressed_len);

    tmp_buffer = (UINT8 *)m_ota_info.tmp_buf_addr;
    mg_setup_crc_table();
    OTA_PRINTF("check ota loader in memory\n");

    chunk_id = OTA_LOADER_ID;
    block_addr = (UINT8 *)m_ota_info.ota_bin_addr;

    if(!get_chunk_header(chunk_id, &chunk_header))
    {
        OTA_PRINTF("cannot find ota loader in memory\n");
        //ret = FALSE;
        //return ret;
        #if 1
            chunk_header.id= fetch_long(block_addr + CHUNK_ID);
            chunk_header.len= fetch_long(block_addr + CHUNK_LENGTH);
            chunk_header.offset=fetch_long(block_addr + CHUNK_OFFSET);
            chunk_header.crc=fetch_long(block_addr + CHUNK_CRC);
        #endif
    }
    //m_ota_info.ota_bin_size = fetch_long(block_addr + CHUNK_OFFSET);
    m_ota_info.ota_bin_size = chunk_header.offset;;
    data = block_addr;
    ret=verify_signature_std(data, tmp_buffer, chunk_header,&error_code);
    if(NUM_ONE==ret)  //app can be started
    {
        OTA_PRINTF("ota loader cannot be started in loader mode\n");
    }
    else    // don't start app
    {
        if(error_type==error_code)    //store app to flash and reboot later
        {
            ver_info->o_m_ver = str2uint32(chunk_header.version, 8);
            OTA_PRINTF("ota loader verify ok and re-encrypt\n");
            verify_result|=SAVE_OTA_LOADER;
        }
        else if(NUM_ONE==error_code)
        {
            OTA_PRINTF("ota code verify failed\n");
        }
    }

    OTA_PRINTF("ota loader: 0x%X, 0x%X\n", \
              m_ota_info.ota_bin_addr, m_ota_info.ota_bin_size);

    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAINCODE_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        block_addr = (UINT8 *)m_ota_info.main_code_addr;
        if(!get_chunk_header(chunk_id, &chunk_header))
        {
            OTA_PRINTF("cannot find main code in memory\n");
            ret = FALSE;
            return ret;
        }

        //m_ota_info.main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        m_ota_info.main_code_size = chunk_header.offset;
        data = block_addr;
        ret=verify_signature_std(data, tmp_buffer, chunk_header,&error_code);
        if(NUM_ONE==ret)  //app can be started
        {
            OTA_PRINTF("main code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(error_type==error_code)    //store app to flash and reboot later
            {
                ver_info->m_m_ver = str2uint32(chunk_header.version, 8);
                OTA_PRINTF("main code verify ok and re-encrypt\n");
                verify_result|=SAVE_MAIN;
            }
            else if(NUM_ONE==error_code)
            {
                OTA_PRINTF("main code verify failed\n");
            }
        }
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n", \
               m_ota_info.main_code_addr, m_ota_info.main_code_size);

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEECODE_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        if(!get_chunk_header(chunk_id, &chunk_header))
        {
            OTA_PRINTF("cannot find main code in memory\n");
            ret = FALSE;
            return ret;
        }
        block_addr = (UINT8 *)m_ota_info.see_code_addr;
        //m_ota_info.see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        m_ota_info.see_code_size = chunk_header.offset;
        data = block_addr;

        ret=verify_signature_std(data, tmp_buffer, chunk_header,&error_code);
        if(NUM_ONE==ret)  //app can be started
        {
            OTA_PRINTF("see code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(error_type==error_code)    //store app to flash and reboot later
            {
                ver_info->s_m_ver = str2uint32(chunk_header.version, 8);
                OTA_PRINTF("see code verify ok and re-encrypt\n");
                verify_result|=SAVE_SEE;
            }
            else if(NUM_ONE==error_code)
            {
                OTA_PRINTF("see code verify failed\n");
            }
        }

    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n", \
               m_ota_info.see_code_addr, m_ota_info.see_code_size);

#ifdef  _BUILD_OTA_E_
    /*because the function of ota_load_parameter
    will call sto_chunk_init to find ota parameter.*/
    //sto_chunk_init again
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        OTA_PRINTF("Can't find FLASH device!\n");
    }
    sto_open(flash_dev);
    sto_chunk_init(0, flash_dev->totol_size);
#endif

    chunk_id=MAINCODE_ID;
    if(sto_get_chunk_header(chunk_id, &chunk_header))
    {
        ver_info->m_f_ver = str2uint32(chunk_header.version, 8);
    }

    chunk_id=SEECODE_ID;
    if(sto_get_chunk_header(chunk_id, &chunk_header))
    {
        ver_info->s_f_ver = str2uint32(chunk_header.version, 8);
    }

    OTA_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver, ver_info->m_f_ver,
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    OTA_PRINTF("verify result=0x%x\n",verify_result);
    if(verify_result==(SAVE_MAIN|SAVE_SEE|SAVE_OTA_LOADER))
    {
        return TRUE;    //save apps to flash
    }
    else
    {
        return FALSE;
    }
}

INT32 win_otaupg_burnflash(void)
{
    BOOL fw_not_match = FALSE;
    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_BURNFLASH;
    win_popup_choice_t choice;
    UINT8 back_saved;
    struct otaupg_ver_info ver_info;
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id, chunk_addr, chunk_size;
    BOOL check_ret;

    osal_flag_clear(g_ota_flg_id, OTA_FLAG_PTN);

    init_storage();

    setup_ota_parameters();

    MEMSET(&ver_info, 0, sizeof(struct otaupg_ver_info));
    check_ret=otaupg_check(&ver_info);
    if((check_ret!=TRUE)||(ver_info.m_m_ver != ver_info.s_m_ver))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Invalid OTA data, cannot burn it!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }

    UINT32 upgrade_size = 0;

    if((ver_info.m_m_ver >= ver_info.m_f_ver)\
       &&(ver_info.s_m_ver >= ver_info.s_f_ver))
    {
        upgrade_size += m_ota_info.main_code_size;
        upgrade_size += m_ota_info.see_code_size;
    }
#ifndef _BUILD_OTA_E_
    else
    {
        //OSM_TRIGGERS* pOSM_Triggers=GetOsmTriggers();
        //if(pOSM_Triggers->OTA_Type==OSM_FORCED)
        if(backup_ota_type==OSM_FORCED)
        {
            upgrade_size += m_ota_info.main_code_size;
            upgrade_size += m_ota_info.see_code_size;
        }
    }
#endif

    if (NUM_ONE==upgrade_size )
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Nothing need to be upgrade!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }

#ifndef _BUILD_OTA_E_
    upgrade_size += m_ota_info.ota_bin_size;
    upgrade_size += m_ota_info.user_db_len;
#else
    upgrade_size += m_ota_info.user_db_len;
#endif

    m_ota_info.update_total_sectors = upgrade_size / C_SECTOR_SIZE;
    m_ota_info.update_secotrs_index = 0;

    INT32 ret = 0;
    UINT32 flash_pos;
    do
    {
    #ifndef _BUILD_OTA_E_
        // TODO: step 1: backup UserDB
        OTA_PRINTF("BackupDataBase!\n");
        ret = ota_upg_backup_database();
        if (SUCCESS != ret)
            break;

        // TODO: step 2: burn OTA to UserDB
        OTA_PRINTF("Burn OTALOADER!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.ota_bin_addr, \
          m_ota_info.user_db_start_sector , m_ota_info.ota_bin_sectors);
        if(SUCCESS != ret)
            break;
    #endif

        // TODO: step 3: calculate maincode chunk_addr
        chunk_id = MAINCODE_ID;
        chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if ((UINT32)ERR_FAILUE == chunk_addr)
        {
            OTA_PRINTF("Cannot find maincode's chunk!\n");
            ret = ERR_FAILED;
            break;
        }
        chunk_size = 0;

        // burn new maincode.bin
        // main code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.main_code_size;
        OTA_PRINTF("upgrade main code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.main_code_addr,\
          flash_pos, chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;

        // see code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.see_code_size;
        OTA_PRINTF("upgrade see code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.see_code_addr, \
          flash_pos, chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;

    #ifndef _BUILD_OTA_E_
        // TODO: burn back UserDB
        OTA_PRINTF("BurnBack UserDB!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.backup_db_addr,\
          m_ota_info.user_db_start_sector, m_ota_info.user_db_sectors);
        if(SUCCESS != ret)
            break;
    #else
        OTA_PRINTF("reset UserDB!\n");
        ret = ota_upg_burn_block((UINT8*)NULL, m_ota_info.user_db_start_sector, \
          m_ota_info.user_db_sectors);
        if(SUCCESS != ret)
            break;
    #endif
    } while(0);

    osal_flag_set(g_ota_flg_id,(SUCCESS == ret) ? PROC_SUCCESS : PROC_FAILURE);
    return ret;
}

#else
#ifdef _BC_CA_ENABLE_
#define NEW_OTA_FLOW
#define NEW_SEE_VERIFY_FLOW
#define MAX_CHUNK_LEN    0x800000

#ifdef _SMI_ENABLE_
BOOL verify_code(UINT8 *addr, UINT32 len, UINT8 *public_key, UINT8 *ecgk) //implement verify upgraded code here
{
	static UINT8 *encrypted_buff = NULL;
	RET_CODE ret=RET_SUCCESS;
	BOOL ver_flag = TRUE;
		
	encrypted_buff=(UINT8*)( (0xFFFFFFF8&(UINT32)MALLOC(len+0xf)) ); 
	if(encrypted_buff ==NULL)
	{
		OTA_PRINTF("verify code malloc failed..\n");
		return FALSE;;
	}	
	ret=decrypt_code(addr, encrypted_buff, len, ecgk); //Decrypt system code
	if(RET_SUCCESS!=ret)
	{
		OTA_PRINTF("OTA Decrypt code failed\n");
		ver_flag = FALSE;
	}
	else
	{
		ret=test_cust_rsa_ram((UINT32)encrypted_buff, len, public_key); //verify sw signature
		if(RET_SUCCESS!=ret)
		{
			OTA_PRINTF("OTA verify code failed,First 4 byte(%08x)\n",*(UINT32*)encrypted_buff);
			ver_flag = FALSE;
		}
	}
	FREE(encrypted_buff);
	return ver_flag; //verify ok
}

BOOL verify_see_code(UINT8 *addr, UINT32 len, UINT8 *public_key) //implement verify upgraded code here
{
	RET_CODE ret=RET_SUCCESS;
	BOOL ver_flag = TRUE;		

	ret = test_cust_rsa_ram((UINT32)addr, len, public_key); //verify sw signature
	if(RET_SUCCESS!=ret)
	{
		OTA_PRINTF("%s():OTA verify SEE code failed\n",__FUNCTION__);
		ver_flag = FALSE;
	}
	return ver_flag; //verify ok
}

//sync from BL
BOOL bootrom_sig_enable(void)
{
	UINT32 temp_buf=0;
	
	otp_init(NULL);
	otp_read(0xDD*4, (UINT8*)&temp_buf,4);
	temp_buf>>=1;
	temp_buf&=0x1;
	if(1 == temp_buf)
	{
		OTA_PRINTF("bootrom signature check is enable\n");
		return TRUE;
	}
	else
	{
		OTA_PRINTF("bootrom signature check is disable\n");
		return FALSE;
	}
}

void config_check_temp_buffer(UINT32 addr, UINT32 size)
{
    m_ota_info.tmp_buf_addr = addr;
    m_ota_info.tmp_buf_size = size;
}

void get_main_version_in_flash(UINT32 ck_addr,UINT32 *pversion)
{
    UINT32 chunk_addr = ck_addr;
    UINT32 len = 0;
    UINT32 data_len = 0;
    UINT32 chk_maxlen = 0;
    struct sto_device *flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    UINT8 *temp_buffer = (UINT8 *)m_ota_info.tmp_buf_addr;
    UINT32 temp_buffer_size = m_ota_info.tmp_buf_size;

    chk_maxlen = sto_fetch_long(chunk_addr + CHUNK_OFFSET);
    data_len = sto_fetch_long(chunk_addr + CHUNK_LENGTH);
    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return ;
    }
    
    if(data_len < temp_buffer_size)
    {
    	len = data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);    
        sto_get_data(flash_dev,temp_buffer,chunk_addr,len+CHUNK_HEADER_SIZE);
    	//verify flash ota main code using upgraded rsa key
    	if(TRUE==verify_code(&temp_buffer[CHUNK_HEADER_SIZE], len, g_cust_public_key, g_ecgk_key))
    	{	
    		*pversion = str2uint32(temp_buffer+CHUNK_VERSION, 8);
    	}
    	else
    	{	
    		*pversion = 0;
    	}
    }
}

void get_see_version_in_flash(UINT32 ck_addr,UINT32 *pversion)
{
    UINT32 chunk_addr = ck_addr;
    UINT32 data_len = 0 , enc_len = 0, sig_len = 0 ;
    UINT32 chk_maxlen = 0;
    struct sto_device *flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    UINT8 *temp_buffer = (UINT8 *)m_ota_info.tmp_buf_addr;
    UINT32 temp_buffer_size = m_ota_info.tmp_buf_size;

    chk_maxlen = sto_fetch_long(chunk_addr + CHUNK_OFFSET);
    data_len = sto_fetch_long(chunk_addr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN; //for align
    sig_len= enc_len + CHUNK_HEADER_SIZE;

    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return ;
    }
    
    if(sig_len < temp_buffer_size)
    {
        sto_get_data(flash_dev,temp_buffer,chunk_addr,sig_len);
        if(TRUE==verify_see_code(temp_buffer, sig_len, g_cust_public_key))
    	{
    		*pversion = str2uint32(temp_buffer+CHUNK_VERSION, 8);
    	}
    	else
    	{
    		*pversion = 0;
    	}   
    }
}

BOOL get_main_code_in_mem(UINT8 *ck_addr,UINT32 *pversion)
{    
    UINT8 *block_addr = ck_addr;
    UINT8 *data = NULL;
    UINT32 data_len = 0;
    UINT32 chk_maxlen = 0;
    UINT32 len = 0;
    BOOL ret = FALSE;

    chk_maxlen = fetch_long(block_addr + CHUNK_OFFSET);
    data_len = fetch_long(block_addr + CHUNK_LENGTH);
	data = block_addr+CHUNK_HEADER_SIZE;

    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return FALSE;
    }

	len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
	//verify memory main code using upgraded rsa key
	if(TRUE==verify_code(data, len, g_cust_public_key, g_ecgk_key))
	{
		*pversion = str2uint32(block_addr+CHUNK_VERSION, 8);
        ret = TRUE;
	}
	else
	{
        *pversion = 0;		
	}

    return ret;
}

BOOL get_see_code_in_mem(UINT8 *ck_addr,UINT32 *pversion)
{
    BOOL ret = FALSE;
#ifdef NEW_SEE_VERIFY_FLOW    
    UINT32 data_len = 0 ;
    UINT32 sig_len = 0;
    UINT32 enc_len = 0;
    UINT8 *data = ck_addr;
    UINT32 chk_maxlen = 0;

    chk_maxlen = fetch_long(data + CHUNK_OFFSET);
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN; //for align
    sig_len= enc_len + CHUNK_HEADER_SIZE;

    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return FALSE;
    }
    
    if(TRUE==verify_see_code(data, sig_len, g_cust_public_key))
    {
        *pversion = str2uint32(data+CHUNK_VERSION, 8);
        ret = TRUE;
    }
    else
    {
        *pversion = 0;
    }
#else
    //OLD way, main and see has same verify flow.
    ret = get_main_code_in_mem(ck_addr,pversion);
#endif
    return ret;
}

void vendor_id_select(int vid)
{
    UINT64 smi_boot_vendor_id = 0x564d00000014f2a5ULL;
    
    if(vid == VID_CHOOSE_SMI_VENDORID)
    {
        otp_etsi_kdf_trigger(0, smi_boot_vendor_id);
	    otp_etsi_kdf_trigger(1, smi_boot_vendor_id);
	    otp_etsi_kdf_trigger(2, smi_boot_vendor_id);
    }
    else
    {        
        otp_etsi_kdf_trigger(0, main_vendor_id);
        otp_etsi_kdf_trigger(1, main_vendor_id);
        otp_etsi_kdf_trigger(2, main_vendor_id);
    }
}

extern RET_CODE test_smi_rsa_ram(const UINT32 addr, const UINT32 len);
//for SMI OTA check
#define OTAUPG_CHECK_SMI    otaupg_check
BOOL OTAUPG_CHECK_SMI(struct otaupg_ver_info *ver_info)
{
	struct sto_device *flash_dev = NULL;
	UINT32 chunk_id = 0;
	UINT8 *data=NULL;
	UINT32 data_len=0;
	UINT32 len=0;
	UINT8 *block_addr=NULL;
	UINT8 *temp_buffer=NULL;
	RET_CODE ret=RET_FAILURE;
	CHUNK_HEADER blk_header;
    UINT32 boot_total_area_len = 0;    

    get_boot_total_area_len(&boot_total_area_len);
	// get unzip size, temp_buffer will use compressed addr, get uncompressed_len here
	m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);
	chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len, m_ota_info.uncompressed_len-boot_total_area_len);

	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (flash_dev == NULL)
	{
		OTA_PRINTF("Can't find FLASH device!\n");
	}

	sto_open(flash_dev);
	sto_chunk_init(boot_total_area_len, flash_dev->totol_size-boot_total_area_len);

    OTA_PRINTF("Set the boot vendor ID\n");
	vendor_id_select(VID_CHOOSE_SMI_VENDORID);
    
	temp_buffer = (UINT8 *)m_ota_info.tmp_buf_addr;
	chunk_id=RSA_KEY_CHUNK_ID;
	block_addr=(UINT8 *)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if((UINT8 *)ERR_FAILUE!=block_addr)
	{
		data = block_addr+CHUNK_HEADER_SIZE;
		data_len = sto_fetch_long((unsigned long)(block_addr + CHUNK_LENGTH));
		len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
		sto_get_data(flash_dev, temp_buffer, (UINT32)data, len);
		ret=test_smi_rsa_ram(( UINT32 )temp_buffer, len);
		if((RET_SUCCESS== ret))
		{
        FETCH_KEY:
			MEMCPY(g_cust_public_key, temp_buffer, 516);
			if(sto_get_chunk_header(chunk_id, &blk_header))
			{
				ver_info->rsa_f_ver = str2uint32(blk_header.version, 8);
			}
			else
			{
				ver_info->rsa_f_ver=0;
			}
		}
		else
		{
            if(bootrom_sig_enable())
            {
			    ver_info->rsa_f_ver=0;
            }
            else
            {
                //test mode, fetch key from flash.
                goto FETCH_KEY;
            }
		}
	}
	else
	{
		ver_info->rsa_f_ver=0;
	}	
	
	chunk_id=ECGK_CHUNK_ID;
	block_addr=(UINT8 *)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if((UINT8 *)ERR_FAILUE!=block_addr)
	{
		data = block_addr+CHUNK_HEADER_SIZE;
		data_len = sto_fetch_long((UINT32)(block_addr + CHUNK_LENGTH));
		len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
		sto_get_data(flash_dev, temp_buffer, (UINT32)data, len);
		MEMCPY(g_ecgk_key, temp_buffer, 32);
		if(sto_get_chunk_header(chunk_id, &blk_header))
		{
			ver_info->ecgk_f_ver = str2uint32(blk_header.version, 8);
		}
	}
	else
	{
		ver_info->ecgk_f_ver=0;
	}	
    
#ifndef NEW_OTA_FLOW
	chunk_id=SEE_BL_CODE_CHUNK_ID;
	if(sto_get_chunk_header(chunk_id, &blk_header))
	{
		ver_info->s_bl_f_ver = str2uint32(blk_header.version, 8);
	}
#endif

	chunk_id=MAIN_CODE_CHUNK_ID;
	if(sto_get_chunk_header(chunk_id, &blk_header))
	{
		ver_info->m_f_ver = str2uint32(blk_header.version, 8);
	}

	chunk_id=SEE_CODE_CHUNK_ID;
	if(sto_get_chunk_header(chunk_id, &blk_header))
	{
		ver_info->s_f_ver = str2uint32(blk_header.version, 8);
	}
    
    #ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("check OUC in flash\n");
    
    chunk_id = OUC_CHUNK_ID;
	block_addr = (UINT8*)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((UINT32)block_addr != (UINT32)ERR_FAILUE)
    {
        UINT32 chunk_addr = 0;
        //skip OUC header.
        OTA_PRINTF("check ota main code in flash\n");
        chunk_addr = (UINT32)block_addr + CHUNK_HEADER_SIZE; 
        get_main_version_in_flash(chunk_addr,&ver_info->o_m_f_ver);        

        OTA_PRINTF("check ota see code in flash\n");
        chunk_addr += sto_fetch_long(chunk_addr + CHUNK_OFFSET); 
        get_see_version_in_flash(chunk_addr,&ver_info->o_s_f_ver);             
    }
    #endif	

	OTA_PRINTF("check rsa key in memory\n");
	chunk_id = RSA_KEY_CHUNK_ID;
	m_ota_info.rsa_key_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (m_ota_info.rsa_key_addr)
	{
		block_addr = (UINT8 *)m_ota_info.rsa_key_addr;
		m_ota_info.rsa_key_size = fetch_long(block_addr + CHUNK_OFFSET);
		data_len = fetch_long(block_addr + CHUNK_LENGTH);
		data = block_addr+CHUNK_HEADER_SIZE;

		len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
		ret=test_smi_rsa_ram(( UINT32 )data, len);
		if((RET_SUCCESS== ret)/*||(!bootrom_enable())*/)
		{
			ver_info->rsa_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
		}
		else
		{
            if(bootrom_sig_enable())
            {
			    ver_info->rsa_m_ver=0; //invalid rsa key in memory
            }
            else
            {
                //test mode, fetch key from stream.
                ver_info->rsa_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
            }
		}
	}
	else
	{
		ver_info->rsa_m_ver=0; //invalid rsa key in memory
	}
	OTA_PRINTF("RSA key: 0x%X, 0x%X\n", m_ota_info.rsa_key_addr, m_ota_info.rsa_key_size);

	if((0==ver_info->rsa_f_ver) && (0==ver_info->rsa_m_ver))
	{
		OTA_PRINTF("no valid rsa key in flash or memory\n");
		return FALSE;
	}
	
	if((ver_info->rsa_m_ver > ver_info->rsa_f_ver)||(MEMCMP(g_cust_public_key, data, 516)!=0)) //cstm rsa key diff, need to upgrade rsa key and ecgk
	{
		MEMCPY(g_cust_public_key, data, 516); //use upgraded cust public key to verify main, see and ota loader
		ca_switch_mode = 1;
        OTA_PRINTF("ca_switch_mode = 1\n");
	}
	else
	{
		ca_switch_mode = 0;
        OTA_PRINTF("ca_switch_mode = 0\n");
	}

	OTA_PRINTF("check ecgk memory\n");
	chunk_id = ECGK_CHUNK_ID;
	m_ota_info.ecgk_key_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (m_ota_info.ecgk_key_addr)
	{
		block_addr = (UINT8 *)m_ota_info.ecgk_key_addr;
		m_ota_info.ecgk_key_size = fetch_long(block_addr + CHUNK_OFFSET);	
		ver_info->ecgk_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
		data = block_addr+CHUNK_HEADER_SIZE;
		if((ca_switch_mode == 1)||(ver_info->ecgk_m_ver > ver_info->ecgk_f_ver))
		{
			MEMCPY(g_ecgk_key, data, 32); //use upgraded ECGK to decrypt main, see
			ca_switch_mode = 1;
		}
	}
	else
	{
		ver_info->ecgk_m_ver=0;
	}
	OTA_PRINTF("ECGK: 0x%X, 0x%X\n", m_ota_info.ecgk_key_addr, m_ota_info.ecgk_key_size);

	if((0==ver_info->ecgk_f_ver) && (0==ver_info->ecgk_m_ver))
	{
		OTA_PRINTF("no ecgk key in flash or memory\n");
		return FALSE;
	}	

	OTA_PRINTF("check ota main code in memory\n");
    #ifdef _OUC_LOADER_IN_FLASH_
    chunk_id = OUC_CHUNK_ID;
	block_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if(block_addr)
    {
       m_ota_info.ota_main_code_addr = (UINT32)block_addr + CHUNK_HEADER_SIZE;
    }
    #else
	chunk_id = OTA_MAIN_CODE_CHUNK_ID;
	m_ota_info.ota_main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    #endif
	if (m_ota_info.ota_main_code_addr)
	{
		block_addr = (UINT8 *)m_ota_info.ota_main_code_addr;
		m_ota_info.ota_main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_main_code_in_mem(block_addr,&ver_info->o_m_m_ver))
        {
            OTA_PRINTF("Verify ota main in memory failed\n");
            return FALSE;
        }		
	}
	OTA_PRINTF("ota main code: 0x%X, 0x%X\n", m_ota_info.ota_main_code_addr, m_ota_info.ota_main_code_size);	

	OTA_PRINTF("check ota see code in memory\n");
    #ifdef _OUC_LOADER_IN_FLASH_
    if(m_ota_info.ota_main_code_addr)
    {
        UINT32 main_ck_id = fetch_long((UINT8*)m_ota_info.ota_main_code_addr + CHUNK_ID) ;
        UINT32 main_offset = fetch_long((UINT8*)m_ota_info.ota_main_code_addr + CHUNK_OFFSET) ;
        if(main_ck_id == OTA_MAIN_CODE_CHUNK_ID)
        {
            m_ota_info.ota_see_code_addr = m_ota_info.ota_main_code_addr + main_offset;
        }
    }
    #else
	chunk_id = OTA_SEE_CODE_CHUNK_ID;
	m_ota_info.ota_see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    #endif
	if (m_ota_info.ota_see_code_addr)
	{
		block_addr = (UINT8 *)m_ota_info.ota_see_code_addr;
		m_ota_info.ota_see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_see_code_in_mem(block_addr,&ver_info->o_s_m_ver))
        {
            OTA_PRINTF("Verify ota see in memory failed\n");
            return FALSE;
        }      
	}
	OTA_PRINTF("ota see code: 0x%X, 0x%X\n", m_ota_info.ota_see_code_addr, m_ota_info.ota_see_code_size);	

#ifndef NEW_OTA_FLOW	
	OTA_PRINTF("check see boot loader code in memory\n");
	chunk_id = SEE_BL_CODE_CHUNK_ID;
	m_ota_info.see_bl_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (m_ota_info.see_bl_code_addr)
	{
		block_addr = (UINT8 *)m_ota_info.see_bl_code_addr;
		m_ota_info.see_bl_code_size = fetch_long(block_addr + CHUNK_OFFSET);
		data_len = fetch_long(block_addr + CHUNK_LENGTH);
		data = block_addr+CHUNK_HEADER_SIZE;

		len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
		//verify memory see boot loader code using upgraded rsa key
		if(TRUE==verify_code(data, len, g_cust_public_key, g_ecgk_key))
		{
			ver_info->s_bl_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
		}
		else
		{
			return FALSE;
		}		
	}
	OTA_PRINTF("see boot loader code: 0x%X, 0x%X\n", m_ota_info.see_bl_code_addr, m_ota_info.see_bl_code_size);
#endif

	OTA_PRINTF("check main code in memory\n");
	chunk_id = MAIN_CODE_CHUNK_ID;
	m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (m_ota_info.main_code_addr)
	{
		block_addr = (UINT8 *)m_ota_info.main_code_addr;
		m_ota_info.main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_main_code_in_mem(block_addr,&ver_info->m_m_ver))
        {
            OTA_PRINTF("Verify main in memory failed\n");
            return FALSE;
        }			
	}
	OTA_PRINTF("main code: 0x%X, 0x%X\n", m_ota_info.main_code_addr, m_ota_info.main_code_size);

	OTA_PRINTF("check see code in memory\n");
	chunk_id = SEE_CODE_CHUNK_ID;
	m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (m_ota_info.see_code_addr)
	{
		block_addr = (UINT8 *)m_ota_info.see_code_addr;
		m_ota_info.see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
		data_len = fetch_long(block_addr + CHUNK_LENGTH);
        if(!get_see_code_in_mem(block_addr,&ver_info->s_m_ver))
        {
            OTA_PRINTF("Verify see in memory failed\n");
            return FALSE;
        } 
	}
	OTA_PRINTF("see code: 0x%X, 0x%X\n", m_ota_info.see_code_addr, m_ota_info.see_code_size);

#ifdef NEW_OTA_FLOW
    OTA_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
		ver_info->o_m_f_ver, ver_info->o_m_m_ver, ver_info->o_s_f_ver, ver_info->o_s_m_ver,
		ver_info->m_f_ver, ver_info->m_m_ver, 
		ver_info->s_f_ver, ver_info->s_m_ver, ver_info->rsa_f_ver, ver_info->rsa_m_ver);
#else
	OTA_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
		ver_info->o_m_f_ver, ver_info->o_m_m_ver, ver_info->o_s_f_ver, ver_info->o_s_m_ver,
		ver_info->s_bl_f_ver, ver_info->s_bl_m_ver, ver_info->m_f_ver, ver_info->m_m_ver, 
		ver_info->s_f_ver, ver_info->s_m_ver, ver_info->rsa_f_ver, ver_info->rsa_m_ver);
#endif
	return TRUE;	//save apps to flash
}
#undef OTAUPG_CHECK_SMI
#else

#ifdef FAKE_VERITY_SIGNATURE
static uint8_t magic[MAGIC_CODE_LEN] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

static uint32_t crc32(uint8_t *data, uint32_t len)
{
    return mg_table_driven_crc(0xFFFFFFFF, data, len);
}

uint64_t get_number( uint8_t* ptr )
{
    uint64_t tmp_val = 0;
    int i;
    // data in the signature is big-endian
    for( i = 0; i < 8; i++ )
    {
        tmp_val = (tmp_val<<8) | ptr[i];
    }
    return tmp_val;
}

uint8_t check_the_data_and_hash(uint8_t *data, uint32_t len, uint8_t* signature)
{

    // this function verifies the hash values and determines whether a
    // re-encryption is needed
    uint32_t tmp_crc = crc32( data, len );

    /* we only have put 32-bits into the signature
    therefore we type-cast and correct the offset by 4 */
    if( ( (uint32_t)get_number( signature + 48 - 4 ) ) == tmp_crc )
    {
    #if 1   // here we "decrypt" by using xor
        UINT32 j;
        OTA_PRINTF("checkTheDataAndHash: \
               tmpCrc=0x%x match, decrypt the source\n",tmp_crc);
        for( j = 0; j < len; j++ )
            data[j] ^= 0xff;
    #endif
        return 0;
    }
    else
    {
        // here we can already "re-encrypt" by using xor
        UINT32 i;
        for( i = 0; i < len; i++ )
            data[i] ^= 0xff;
        tmp_crc = crc32( data, len );
        /* we only have put 32-bits into the signature
    therefore we type-cast and correct the offset by 4 */
        if( ( (uint32_t)get_number( signature + 48 - 4 ) ) != tmp_crc )
        {
            // the crc does not match therefore there it is an error
            OTA_PRINTF("checkTheDataAndHash: crc error tmpCrc=0x%x\n",tmp_crc);
            return 2;
        }
        return 1;
    }

}

uint8_t decrypt_signature( uint8_t* dec_signature, uint8_t* signature )
{
    // here we do signature decryption
    // we only copy here as the signature is not encrypted at all
    MEMCPY( dec_signature, signature, 256 );
    return 1;
}

int BC_VerifySignature( uint8_t* signature, uint8_t* src, uint8_t* tmp,\
                       uint32_t len, uint32_t max_len, \
               uint8_t mode,uint8_t *error_code )
{
    uint8_t dec_signature[256];
    UINT8 error_type=2;

    OTA_PRINTF("verifySignature: len=0x%x, maxLen=0x%x, mode=%d\n",\
    len, max_len, mode);
    OTA_PRINTF("verifySignature: signature crc=0x%x 0x%x 0x%x 0x%x\n",\
    signature[48],signature[49],signature[50],signature[51]);
    OTA_PRINTF("verifySignature: signature len=0x%x 0x%x 0x%x 0x%x\n",\
    signature[36],signature[37],signature[38],signature[39]);
    OTA_PRINTF("verifySignature: signature maxlen=0x%x 0x%x 0x%x 0x%x\n",\
    signature[44],signature[45],signature[46],signature[47]);
#ifdef DEBUG_MODE_VERITY_SIGNATURE
    // for loading real CT application to flash
    *error_code = 2;
    return 0;
#endif
    // we decrypt the signature, which you do not have to do here
    if( decrypt_signature( dec_signature, signature ) )
    {
        if( MEMCMP( dec_signature, magic, MAGIC_CODE_LEN) )
        {
            // error magic value is wrong, therefore signature is bad
            OTA_PRINTF("verifySignature: magic value wrong:0x%x 0x%x\n",\
        dec_signature[0], dec_signature[1]);
            *error_code = 1;
            return 0;
        }
        else
        {
            int i;
            for( i = 9; i < 24; i++ )
            {
                if( dec_signature[i] )
                {
                    // error verifier is not 0, therefore signature is bad
                    OTA_PRINTF("verifySignature: zero value wrong:\
            decSignature[%d]= 0x%x\n",\
            i,dec_signature[i]);
                    *error_code = 1;
                    return 0;
                }
            }
            // the signature has been decrypted correctly, now we check the
            // length values
            if( ( get_number( dec_signature+32 ) == len ) \
           &&( get_number( dec_signature+40 ) == max_len ) )
            {
                // now we do some additional checks for hash-value
                // and from this check we determine whether a
                // re-encryption is needed
                uint8_t re_encryption_needed = check_the_data_and_hash\
                                     ( src, len, dec_signature );
                OTA_PRINTF("verifySignature: re_encryption_needed=%d\n",\
                   re_encryption_needed);
                if(error_type== re_encryption_needed)
                {
                    // error the hash value is not correct,
                    // therefore signature is bad
                    *error_code = 1;
                    return 0;
                }
                // if re-encryption is needed we check, for the mode value.
                if( (re_encryption_needed) \
           && ( NUM_ONE==dec_signature[8] ) &&( NUM_ZERO==mode  ) )
                {
                    // error re-encryption when
            //starting from flash must not occur
                    // for loaded image, therefore signature is bad
                    OTA_PRINTF("verifySignature: mode not match error\n");
                    *error_code = 1;
                    return 0;
                }

                if( re_encryption_needed )
                {
                    // do the re-encryption
                    // we have already done it
                    *error_code = 2;
                    return 0;
                }
                else
                {
                    if( NUM_ONE==mode)
                    {
                        // error there should be
            //no re-encrypted image directly loaded
                        OTA_PRINTF("verifySignature: error mode should be 0\n");
                        * error_code = 1;
                        return 0;
                    }
                    else
                    {
                        // everything is fine
                        *error_code = 0;
                        return 1;
                    }
                }
            }
            else
            {
                // error one of the length values does not match,
                // therefore signature is bad
                OTA_PRINTF("verifySignature: len wrong: \
        len=0x%x 0x%x 0x%x 0x%x\n",\
        dec_signature[32+4], dec_signature[32+5],\
        dec_signature[32+6], dec_signature[32+7]);
                OTA_PRINTF("verifySignature: maxlen wrong: \
        maxlen=0x%x 0x%x 0x%x 0x%x\n",\
        dec_signature[40+4], dec_signature[40+5],\
        dec_signature[40+6], dec_signature[40+7]);
                *error_code = 1;
                return 0;
            }
        }
    }
    else
    {
        // error decryption failed, therefore signature is bad
        OTA_PRINTF("verifySignature: decryption failed\n");
        *error_code = 1;
        return 0;
    }
}
#endif

#ifdef _SECURE1_5_SUPPORT_
//VMX Plus
UINT32 get_code_version(UINT8 *data_addr, UINT32 data_len)
{
    //UINT32 code_len=0;
    UINT32 ver = 0;

    if (NUM_ZERO == test_rsa_ram((UINT32)data_addr, data_len))
    {
        ver = str2uint32_dec(data_addr + CHUNK_VERSION, CODE_VERSION_LEN);
        OTA_PRINTF("\tcode version: 0x%X\n", ver);
    }
    else
    {
        OTA_PRINTF("\tverify code failed!\n");
    }
    return ver;
}

UINT32 get_code_version_ext(UINT8 *data_addr, UINT32 data_len,UINT8 rsa_key_id)
{
    //UINT32 code_len=0;
    UINT32 ver = 0;

    if (NUM_ZERO == test_rsa_ram_by_id((UINT32)data_addr, data_len,rsa_key_id))
    {
        ver = str2uint32_dec(data_addr + CHUNK_VERSION, CODE_VERSION_LEN);
        OTA_PRINTF("\tcode version: 0x%X\n", ver);
    }
    else
    {
        OTA_PRINTF("\tverify code failed!\n");
    }
    return ver;
}

BOOL get_chunk_header_in_flash(UINT32 ck_id,UINT8* buf)
{
    BOOL ret = TRUE;
	UINT32 chunk_hdr = 0;
    struct sto_device *test_flash_dev = NULL;
    //UINT32 data_base = 0;
    
    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO); 
    if((NULL == buf)|| (NULL == test_flash_dev))
    {
        return FALSE;
    }
    chunk_hdr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFF0000, 1); 
    if ((ERR_PARA == (INT32)chunk_hdr) || (ERR_FAILUE == (INT32)chunk_hdr) )
    {
        return FALSE;
    }

    ret = sto_get_data(test_flash_dev, buf, chunk_hdr, CHUNK_HEADER_SIZE);
    if (CHUNK_HEADER_SIZE != ret)
    {
        return FALSE;
    }
    
    return TRUE;    
}

UINT32 get_chunk_in_flash_ex(UINT32 ck_id,UINT8 *buf,UINT32 buf_size)
{
    INT32 ret = 0;
	UINT32 chunk_hdr = 0;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    
    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO); 
    if((NULL == buf)|| (NULL == test_flash_dev) || !buf_size)
    {
        OTA_PRINTF("%s() Invalid parameter\n",__FUNCTION__);
        return 0;
    }
    chunk_hdr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFFFFFF, 1); 
    if ((ERR_PARA == (INT32)chunk_hdr) || (ERR_FAILUE == (INT32)chunk_hdr) )
    {
        OTA_PRINTF("%s() Cannot find chunk(0x%08x)\n",__FUNCTION__);
        return 0;
    }
    ret = sto_get_data(test_flash_dev, buf, chunk_hdr, CHUNK_HEADER_SIZE);
    if (CHUNK_HEADER_SIZE != ret)
    {
        OTA_PRINTF("%s() get chunk header error\n",__FUNCTION__);
        return 0;
    }
    data_len = fetch_long(buf + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;    
    ret = sto_get_data(test_flash_dev, &buf[CHUNK_HEADER_SIZE], (chunk_hdr + CHUNK_HEADER_SIZE), enc_len);  
    if ((INT32)enc_len != ret)
    {
        OTA_PRINTF("%s() get chunk header error\n",__FUNCTION__);
        return 0;
    }

    if(buf_size < (enc_len + CHUNK_HEADER_SIZE))
    {
        OTA_PRINTF("%s() Error, buf two small !!!\n",__FUNCTION__);
    }
    
    return enc_len + CHUNK_HEADER_SIZE;
}

UINT32 get_chunk_data_in_flash(UINT32 ck_id,UINT8 *buf,UINT32 buf_size)
{
    INT32 ret = 0;
	UINT32 chunk_hdr = 0;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_len = 0;
    
    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO); 
    if((NULL == buf)|| (NULL == test_flash_dev))
    {
        return 0;
    }
    chunk_hdr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFFFFFF, 1); 
    if ((ERR_PARA == (INT32)chunk_hdr) || (ERR_FAILUE == (INT32)chunk_hdr) )
    {
        return 0;
    }
 
    data_len = sto_fetch_long(chunk_hdr + CHUNK_OFFSET) - CHUNK_HEADER_SIZE;
    ret = sto_get_data(test_flash_dev, buf, (chunk_hdr + CHUNK_HEADER_SIZE), data_len);
    if ((INT32)data_len != ret)
    {
        return 0;
    }

    if(buf_size < data_len)
    {
        OTA_PRINTF("%s() Error, buf two small !!!\n",__FUNCTION__);
    }
    
    return data_len;
}

UINT32 get_ram_chunk_version(UINT8* data,UINT8 key_pos,UINT8* tmp_buf)
{
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 ck_ver = 0;
    UINT32 chk_maxlen = 0;
    INT32 ret = 0;

    chk_maxlen = fetch_long(data + CHUNK_OFFSET);
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(tmp_buf,data,CHUNK_HEADER_SIZE);
    data += CHUNK_HEADER_SIZE;
    
    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return 0;
    }
    
    ret = aes_cbc_decrypt_ram_chunk(key_pos, &tmp_buf[CHUNK_HEADER_SIZE], data, enc_len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
    {
        ck_ver = get_code_version(tmp_buf, sig_len);
    }
    else
    {
        OTA_PRINTF("%s(): get version err \n",__FUNCTION__);
    }

    return ck_ver;
}

UINT32 get_ram_chunk_version_ext(UINT8* data,UINT8 key_pos,UINT8* tmp_buf,UINT8 rsa_key_id)
{
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 ck_ver = 0;
    UINT32 chk_maxlen = 0;
    INT32 ret = 0;

    chk_maxlen = fetch_long(data + CHUNK_OFFSET);
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(tmp_buf,data,CHUNK_HEADER_SIZE);
    data += CHUNK_HEADER_SIZE;
    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return 0;
    }
    
    ret = aes_cbc_decrypt_ram_chunk(key_pos, &tmp_buf[CHUNK_HEADER_SIZE], data, enc_len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
    {
        ck_ver = get_code_version_ext(tmp_buf, sig_len,rsa_key_id);
    }
    else
    {
        OTA_PRINTF("%s(): get version err \n",__FUNCTION__);
    }

    return ck_ver;
}


RET_CODE decrypt_universal_key_wrapped(UINT8 *pos,UINT32 app_key_idx)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 key_id = 0;  

    if(pos == NULL)
    {
        return RET_FAILURE;
    }

    if(ALI_C3711C ==sys_ic_get_chip_id())
    {
        key_id = (NEW_HEAD_ID & NEW_HEAD_MASK);        
        key_id |= FIRST_FLASH_KEY;       

        ret = decrypt_universal_key(pos, key_id) ;
    }
    else
    {   
        OTA_PRINTF("%s(): Unknown CHIP, need check \n",__FUNCTION__);
    }

    return ret;
}

UINT8 get_rsa_key_id_by_type(UINT8 key_type)
{
    UINT8 rsa_key_id = 0;

    return rsa_key_id;
}

static void dump_data(UINT8 *data , UINT32 len)
{
    UINT32 i = 0;

    for( i = 0 ; i < len; i++)
    {
        OTA_PRINTF("%02x ",data[i]);
        if(((i + 1) % 16) == 0)
        {
            OTA_PRINTF("\n");
        }
    }
    OTA_PRINTF("\n");    
}

void get_mem_chunk_version(UINT8 *ck_maddr,UINT8 key_pos,UINT8 *decrypt_addr,UINT32 *pversion)
{
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;    
    UINT32 chk_maxlen = 0;
    int ret = 0;

    if(NULL == pversion || NULL == decrypt_addr)
    {
        OTA_PRINTF("%s(): Invalid parameter \n", __FUNCTION__);
        return ;
    }

    chk_maxlen = fetch_long(ck_maddr + CHUNK_OFFSET);
    data_len = fetch_long(ck_maddr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(decrypt_addr,ck_maddr,CHUNK_HEADER_SIZE);
    ck_maddr += CHUNK_HEADER_SIZE;

    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return ;
    }
    
    ret = aes_cbc_decrypt_ram_chunk(key_pos, &decrypt_addr[CHUNK_HEADER_SIZE], ck_maddr, enc_len);
    if(0 != ret || !data_len)
    {
        OTA_PRINTF("%s(): aes_cbc_decrypt_ram_chunk failed(%d),data_len(%d)\n", \
            __FUNCTION__,ret,data_len);
        return ;
    }
    
    *pversion = get_code_version(decrypt_addr, sig_len);
    if(0 == *pversion)
    {
        OTA_PRINTF(" data_len(%x),enc_len(%x),sig_len(%x)\n",data_len,enc_len,sig_len);
        OTA_PRINTF(" header(16)\n");
        dump_data(decrypt_addr,16);
        OTA_PRINTF(" origin data(16)\n");
        dump_data(ck_maddr,16);
        OTA_PRINTF(" decrypt data(16)\n");
        dump_data(&decrypt_addr[CHUNK_HEADER_SIZE],16);
    }
}

void get_see_version_in_mem(UINT8 *ck_maddr,UINT32 *pversion)
{    
    UINT32 data_len = 0 , enc_len = 0, sig_len = 0 ; 
    UINT32 chk_maxlen = 0;

    if(NULL == pversion || NULL == ck_maddr)
    {
        OTA_PRINTF("%s(): Invalid parameter \n", __FUNCTION__);
        return ;
    }

    chk_maxlen = fetch_long(ck_maddr + CHUNK_OFFSET);
    data_len = fetch_long(ck_maddr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN; //for align
    sig_len= enc_len + CHUNK_HEADER_SIZE;

    if(data_len > chk_maxlen || chk_maxlen > MAX_CHUNK_LEN)
    {
        OTA_PRINTF("%s():Invalid Chunk Header, data_len(%x),chk_maxlen(%x)\n",\
            __FUNCTION__,data_len,chk_maxlen);
        return ;
    }
   
    *pversion = get_code_version(ck_maddr, sig_len); 
}

//VMX PLUS SC1.5
#define OTAUPG_CHECK_BC_SC15   otaupg_check
BOOL OTAUPG_CHECK_BC_SC15(struct otaupg_ver_info *ver_info)
{
#ifdef  _BUILD_OTA_E_
    struct sto_device *flash_dev = NULL;
#endif
    UINT8 *addr = NULL ;
    UINT8 *decrypt_addr = NULL;
    UINT32 chunk_id = 0;
    UINT8 *data = NULL ;
    UINT8 key_pos = 0;
    int ret = 0;
    UINT32 boot_total_area_len = 0;
    UINT32 buf_size = 0;

    get_boot_total_area_len(&boot_total_area_len);

    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);

    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len, m_ota_info.uncompressed_len-boot_total_area_len);

    // decrypt universal key, fetch public key from flash
#ifdef  _BUILD_OTA_E_
    //because the function of ota_load_parameter
    //will call sto_chunk_init to find ota parameter.
    //so cann't find the key chunk,get the wrong key.
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        OTA_PRINTF("Can't find FLASH device!\n");
    }
    sto_open(flash_dev);
    sto_chunk_init(boot_total_area_len, flash_dev->totol_size);
#endif

    OTA_PRINTF("fetch public key\n");
    ret = 0;

    ret = decrypt_universal_key_wrapped(&key_pos,0);
    if (ret < NUM_ZERO)
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // get the versions of codes in flash
    addr = (UINT8 *)m_ota_info.cipher_buf_addr;    
    decrypt_addr = (UINT8 *)m_ota_info.decrypted_data_addr; 
    buf_size = m_ota_info.cipher_buf_size > m_ota_info.decrypted_data_size ? m_ota_info.decrypted_data_size : m_ota_info.cipher_buf_size;
    
    OTA_PRINTF("check main code in flash\n");   
    get_chunk_in_flash_ex(MAIN_CODE_CHUNK_ID,addr,buf_size);    
    get_mem_chunk_version(addr,key_pos,decrypt_addr,&ver_info->m_f_ver); 

    OTA_PRINTF("check see code in flash\n");
    get_chunk_in_flash_ex(SEE_CODE_CHUNK_ID,addr,buf_size); 
    get_see_version_in_mem(addr,&ver_info->s_f_ver);

#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("check ouc loader main in flash\n");  
    ret = get_chunk_data_in_flash(OUC_CHUNK_ID,addr,buf_size);
    if(ret)
    {        
        UINT8* ouc_main_addr = addr;
        if(fetch_long(ouc_main_addr) != OTA_MAIN_CODE_CHUNK_ID)
        {
            OTA_PRINTF("Error addr(0x%08x) for 0x%08x\n",addr,OTA_MAIN_CODE_CHUNK_ID);  
        }
  		if ((*ouc_main_addr) + (*(ouc_main_addr+1)) == 0xFF)
		{            
            get_mem_chunk_version(ouc_main_addr,key_pos,decrypt_addr,&ver_info->o_m_f_ver); 
		}

        OTA_PRINTF("check ouc loader see in flash\n");  
        UINT8* ouc_see_addr = ouc_main_addr + fetch_long(ouc_main_addr + CHUNK_OFFSET);
        if(fetch_long(ouc_see_addr) != OTA_SEE_CODE_CHUNK_ID)
        {
            OTA_PRINTF("Error addr(0x%08x) for 0x%08x\n",addr,OTA_MAIN_CODE_CHUNK_ID);  
        }
        get_mem_chunk_version(ouc_main_addr,key_pos,decrypt_addr,&ver_info->o_s_f_ver); 
    } 
#endif      

    OTA_PRINTF("\n");    
    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        data = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(data + CHUNK_OFFSET);
        get_mem_chunk_version(data,key_pos,decrypt_addr,&ver_info->m_m_ver);        
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n", m_ota_info.main_code_addr, m_ota_info.main_code_size);

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);    
    m_ota_info.see_code_size = fetch_long((unsigned char *)(m_ota_info.see_code_addr + CHUNK_OFFSET));
    get_see_version_in_mem((UINT8*)m_ota_info.see_code_addr,&ver_info->s_m_ver);

    OTA_PRINTF("check ouc loader main code in memory\n");     
    chunk_id = OTA_MAIN_CODE_CHUNK_ID;
    m_ota_info.ota_loader_size = m_ota_info.ota_bin_size;//for ota upgrade all size calculate
    m_ota_info.ota_main_code_addr = m_ota_info.ota_bin_addr;
    if (m_ota_info.ota_main_code_addr)
    {
        data = (UINT8 *)m_ota_info.ota_main_code_addr;
        m_ota_info.ota_main_code_size = fetch_long(data + CHUNK_OFFSET);        
        get_mem_chunk_version(data,key_pos,decrypt_addr,&ver_info->o_m_m_ver);
        OTA_PRINTF("ota loader main code: 0x%X, 0x%X\n",\
                m_ota_info.ota_main_code_addr, m_ota_info.ota_main_code_size);

        OTA_PRINTF("check ouc loader see code in memory\n");     
        m_ota_info.ota_see_code_addr = m_ota_info.ota_bin_addr + fetch_long(((UINT8 *)m_ota_info.ota_bin_addr) + CHUNK_OFFSET);
        if (m_ota_info.ota_see_code_addr)
        {    
            data = (UINT8 *)m_ota_info.ota_see_code_addr;
            m_ota_info.ota_see_code_size = fetch_long(data + CHUNK_OFFSET); 
            get_see_version_in_mem(data,&ver_info->o_s_m_ver);
            OTA_PRINTF("ota loader see code: 0x%X, 0x%X\n",\
                m_ota_info.ota_see_code_addr, m_ota_info.ota_see_code_size);
        }  
    }

#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("version info: (0x%X,0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_m_f_ver, ver_info->o_m_m_ver,ver_info->o_s_f_ver,ver_info->o_s_m_ver,\
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    if(!ver_info->o_m_m_ver || !ver_info->o_s_m_ver ||!ver_info->m_m_ver || !ver_info->s_m_ver)
    {
        OTA_PRINTF("Invalid OTA stream \n");
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }
#else
    OTA_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver, ver_info->m_f_ver,
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#endif
    OTA_PRINTF("\n"); 
    ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
    return ret;
}
#undef OTAUPG_CHECK_BC_SC15

#else
extern int BC_VerifySignature( uint8_t* signature, uint8_t* src, uint8_t* tmp,\
                       uint32_t len, uint32_t max_len, \
               uint8_t mode,uint8_t *error_code );
//VMX OLD 
BOOL otaupg_check(struct otaupg_ver_info *ver_info)
{
    struct sto_device *flash_dev __MAYBE_UNUSED__= NULL;
    UINT32 chunk_id = 0;
    UINT8 *data;
    UINT32 data_len;
    UINT8 *block_addr;
    int ret;
    uint8_t* signature;
    uint8_t* tmp_buffer;
    uint32_t len, max_len;
    uint8_t mode, error_code;
    unsigned int header_crc=0;
    //UINT32 block_header, version;
    CHUNK_HEADER blk_header;
    UINT8 verify_result=0;
    UINT8 error_type=2;
#ifdef _BUILD_LOADER_COMBO_
    struct UKEY_INFO ukey;
    UINT32 boot_type = 0;
    boot_type = sys_ic_get_boot_type();
    MEMSET(&ukey, 0x0, sizeof(UKEY_INFO));
#endif
	UINT32 boot_total_area_len = 0;

    get_boot_total_area_len(&boot_total_area_len);
    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size\
                                  ((UINT8 *)m_ota_info.compressed_addr);
    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len, m_ota_info.uncompressed_len-boot_total_area_len);

    tmp_buffer = (UINT8 *)m_ota_info.tmp_buf_addr;
    mg_setup_crc_table();
#ifndef _BUILD_LOADER_COMBO_
    OTA_PRINTF("check ota loader in memory\n");

    block_addr = (UINT8 *)m_ota_info.ota_bin_addr;
    m_ota_info.ota_bin_size = fetch_long(block_addr + CHUNK_OFFSET);
    data_len = fetch_long(block_addr + CHUNK_LENGTH);
    data = block_addr+CHUNK_HEADER_SIZE;

    len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
    signature=data+len;
    //maxLen=m_ota_info.ota_bin_size;  //Clarify 20120806
    max_len=len;
    mode=1;        //from loader
    ret=BC_VerifySignature(signature, data, \
        tmp_buffer, len, max_len, mode,&error_code);
    if(NUM_ONE==ret)  //app can be started
    {
        OTA_PRINTF("ota loader cannot be started in loader mode\n");
    }
    else    // don't start app
    {
        if(error_type==error_code)    //store app to flash and reboot later
        {
            ver_info->o_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
            //calculate crc value for chunk header
            header_crc=mg_table_driven_crc(0xFFFFFFFF, \
                   block_addr+CHUNK_NAME, data_len);
            *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
            *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
            *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
            *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
            OTA_PRINTF("ota block header_crc=0x%x first byte=0x%x\n",\
                   header_crc,*(block_addr+CHUNK_CRC));
            OTA_PRINTF("ota loader verify ok and re-encrypt\n");
            verify_result|=SAVE_OTA_LOADER;
        }
        else if(NUM_ONE==error_code)
        {
            OTA_PRINTF("ota code verify failed\n");
        }
    }

    OTA_PRINTF("ota loader: 0x%X, 0x%X\n", \
              m_ota_info.ota_bin_addr, m_ota_info.ota_bin_size);
#else
	verify_result|=SAVE_OTA_LOADER;
#endif

    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        block_addr = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        data_len = fetch_long(block_addr + CHUNK_LENGTH);
        data = block_addr+CHUNK_HEADER_SIZE;

        len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
        signature=data+len;
        //maxLen=m_ota_info.main_code_size; //Clarify 20120806
        max_len=len;
        mode=1;        //from loader
        ret=BC_VerifySignature(signature, data, tmp_buffer,\
        len, max_len, mode,&error_code);
        if(NUM_ONE==ret)  //app can be started
        {
            OTA_PRINTF("main code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(error_type==error_code)    //store app to flash and reboot later
            {
                ver_info->m_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
                //calculate crc value for chunk header
                header_crc=mg_table_driven_crc(0xFFFFFFFF,\
                   block_addr+CHUNK_NAME, data_len);
                *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
                *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
                *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
                *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
                OTA_PRINTF("main block header_crc=0x%x first byte=0x%x\n",\
                   header_crc,*(block_addr+CHUNK_CRC));
                OTA_PRINTF("main code verify ok and re-encrypt\n");
                verify_result|=SAVE_MAIN;
            }
            else if(NUM_ONE==error_code)
            {
                OTA_PRINTF("main code verify failed\n");
            }
        }
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n", \
               m_ota_info.main_code_addr, m_ota_info.main_code_size);

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        block_addr = (UINT8 *)m_ota_info.see_code_addr;
        m_ota_info.see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        data_len = fetch_long(block_addr + CHUNK_LENGTH);
        data = block_addr+CHUNK_HEADER_SIZE;

		#ifndef _BUILD_LOADER_COMBO_
        len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
        signature=data+len;
        //maxLen=m_ota_info.see_code_size;  //Clarify 20120806
        max_len=len;
        mode=1;        //from loader
        ret=BC_VerifySignature(signature, data, tmp_buffer,\
        len, max_len, mode,&error_code);
		#else
	 	len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
	
	 	if(RET_SUCCESS != generate_bc_see_key(&ukey))
       	{
        	OTA_PRINTF("Error :generate see key failed!\n");
         	return FALSE;
    	}
	 	#endif
        if(NUM_ONE==ret)  //app can be started
        {
            OTA_PRINTF("see code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(error_type==error_code)    //store app to flash and reboot later
            {
				#ifndef _BUILD_LOADER_COMBO_			
                ver_info->s_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
                //calculate crc value for chunk header
                header_crc=mg_table_driven_crc(0xFFFFFFFF,\
                   block_addr+CHUNK_NAME, data_len);
                *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
                *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
                *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
                *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
                OTA_PRINTF("see block header_crc=0x%x first byte=0x%x\n",\
                   header_crc,*(block_addr+CHUNK_CRC));
                OTA_PRINTF("see code verify ok and re-encrypt\n");
                verify_result|=SAVE_SEE;
				#else
				ver_info->s_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
		   		get_see_root_pub_key(see_rsa_pub_key,sizeof(see_rsa_pub_key));
		   
		   		if (RET_SUCCESS == test_cust_rsa_ram((UINT32)block_addr+ CHUNK_HEADER_SIZE, len,see_rsa_pub_key))
		   		{
					//encrypte SeeCode
					OTA_PRINTF("enc see\n");
					tmp_buffer = (UINT8*)( (0xFFFFFFF8&(UINT32)MALLOC(m_ota_info.see_code_size+0xf)) );
			
					ret = aes_crypt_puredata_with_ce_key(block_addr+ CHUNK_HEADER_SIZE, tmp_buffer, len, ukey.kl_key_pos, DSC_ENCRYPT);
					MEMCPY(block_addr+ CHUNK_HEADER_SIZE, tmp_buffer, len);
					//calculate crc value for chunk header
	        		header_crc=mg_table_driven_crc(0xFFFFFFFF, block_addr+CHUNK_NAME, data_len);
	        		*(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
	        		*(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
	        		*(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
	        		*(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
	        		OTA_PRINTF("SeeCode block header_crc=0x%x first byte=0x%x\n",header_crc,*(block_addr+CHUNK_CRC));

					FREE(tmp_buffer);
		      		tmp_buffer=NULL;
					verify_result|=SAVE_SEE;
	        			OTA_PRINTF("SeeCode encrypt done\n");
				}
				#endif
            }
            else if(NUM_ONE==error_code)
            {
                OTA_PRINTF("see code verify failed\n");
            }
        }

    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n", m_ota_info.see_code_addr, \
               m_ota_info.see_code_size);

#ifdef    _BUILD_OTA_E_
    //because the function of ota_load_parameter
    //will call sto_chunk_init to find ota parameter.
    //sto_chunk_init again
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        OTA_PRINTF("Can't find FLASH device!\n");
    }
    sto_open(flash_dev);
    sto_chunk_init(boot_total_area_len, flash_dev->totol_size-boot_total_area_len);
#endif

    chunk_id=MAIN_CODE_CHUNK_ID;
    if(sto_get_chunk_header(chunk_id, &blk_header))
    {
        ver_info->m_f_ver = str2uint32(blk_header.version, 8);
    }

    chunk_id=SEE_CODE_CHUNK_ID;
    if(sto_get_chunk_header(chunk_id, &blk_header))
    {
        ver_info->s_f_ver = str2uint32(blk_header.version, 8);
    }

    OTA_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver, ver_info->m_f_ver,
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    OTA_PRINTF("verify result=0x%x\n",verify_result);
    if((SAVE_MAIN|SAVE_SEE|SAVE_OTA_LOADER) == verify_result)
    {
        return TRUE;    //save apps to flash
    }
    else
    {
        return FALSE;
    }
}
#endif
#endif

static BOOL check_chunk_addr(UINT32 ck_id, UINT32 mem_ck_addr)
{
    UINT32 ck_addr = 0;
    UINT32 mem_offset = mem_ck_addr -m_ota_info.uncompressed_addr ;
    
    ck_addr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFFFFFF, 1);
    if((UINT32)ERR_FAILUE != ck_addr)
    {
        if(ck_addr == mem_offset)
        {
            return TRUE;
        }
        else
        {
            OTA_PRINTF("%s():Flash mapping is not match(0x%x-0x%x)\n",\
                __FUNCTION__,ck_addr,mem_offset);
        }        
    }
    else
    {
        OTA_PRINTF("%s():Cannot find chunk(%d) \n",__FUNCTION__,ck_id);
    }

    return FALSE;
}

static UINT32 get_chunk_addr(UINT32 ck_id, UINT32 previous_ck_id,UINT32 mem_ck_addr)
{
    UINT32 ck_addr = 0;
    UINT32 mem_offset = mem_ck_addr -m_ota_info.uncompressed_addr ;
    UINT32 prev_ck_addr = 0 ;
    CHUNK_HEADER ck_header;
    UINT8 check_chunk_order = 0;

    MEMSET(&ck_header,0,sizeof(ck_header));
    
    ck_addr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFFFFFF, 1);
    if((UINT32)ERR_FAILUE != ck_addr)
    {
        OTA_PRINTF("%s():Find chunk(0x%x) \n",__FUNCTION__,ck_id);
        check_chunk_order ++;
        goto FINDIT;
    }
    
    ck_addr = (UINT32)sto_chunk_goto(&previous_ck_id, 0xFFFFFFFF, 1);
    if((UINT32)ERR_FAILUE != ck_addr && sto_get_chunk_header(previous_ck_id,&ck_header))
    {
        ck_addr += ck_header.offset;        
        OTA_PRINTF("%s():Find previous chunk(0x%x), calc the next chunk \n",__FUNCTION__,previous_ck_id);
        goto FINDIT;
    }

    OTA_PRINTF("%s():Use the chunk offset in Memory \n",__FUNCTION__);
    ck_addr = mem_offset;
    return ck_addr;
    
    
FINDIT:
    if(ck_addr != mem_offset)
    {
        OTA_PRINTF("%s():Warn: OTA stream flash mapping is different ! \n",__FUNCTION__);
    }

    if(check_chunk_order)
    {
        prev_ck_addr = (UINT32)sto_chunk_goto(&previous_ck_id, 0xFFFFFFFF, 1);
        if((UINT32)ERR_FAILUE != prev_ck_addr && sto_get_chunk_header(previous_ck_id,&ck_header))
        {
            prev_ck_addr += ck_header.offset;        
            OTA_PRINTF("%s():Find previous chunk(0x%x), calc the next chunk offset \n",\
                __FUNCTION__,previous_ck_id);

            if(prev_ck_addr != ck_addr)
            {
                OTA_PRINTF("%s():Error,Wrong prevoius chunk, please check ! \n",__FUNCTION__);
            }            
        }
    }
    
    return mem_offset;    
}

BOOL check_main_see_size(UINT32 size_in_upgrade)
{
    CHUNK_HEADER main_hdr;
    CHUNK_HEADER see_hdr;
    UINT32 chk_id = 0;
    UINT32 total_size = 0;

    MEMSET(&main_hdr,0,sizeof(main_hdr));
    MEMSET(&see_hdr,0,sizeof(see_hdr));

    chk_id= MAINCODE_ID;
    if(!sto_get_chunk_header(chk_id, &main_hdr))
    {
        OTA_PRINTF("Cannot find Main in flash\n");
        return 0;
    }

    chk_id= SEECODE_ID;
    if(!sto_get_chunk_header(chk_id, &see_hdr))
    {
        OTA_PRINTF("Cannot find SEE in flash\n");
        return 0;
    }

    total_size = main_hdr.offset + see_hdr.offset;
    if(size_in_upgrade != total_size)
    {
        OTA_PRINTF("Warning:Main+SEE size, in flash (%x) , in Upgrade (%x)\n",total_size,size_in_upgrade);
    }
    else
    {
        OTA_PRINTF("Main+SEE size (%x) matched \n",total_size);
    }

    if(size_in_upgrade > total_size)
    {
        OTA_PRINTF("Main+SEE size (%x) in Upgrade is too big \n",size_in_upgrade);
        return 1;
    }

    return 0;
}

//VMX or VMX+SMI
#define WIN_OTAUPG_BURNFLASH_BC win_otaupg_burnflash
INT32 WIN_OTAUPG_BURNFLASH_BC(void)
{
    //BOOL fw_not_match = FALSE;
    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_BURNFLASH;
    //win_popup_choice_t choice;
    UINT8 back_saved;
    struct otaupg_ver_info ver_info;
    MEMSET(&ver_info, 0x0, sizeof (ver_info));
    //struct sto_device *flash_dev = NULL;
    UINT32 chunk_id __MAYBE_UNUSED__, chunk_addr = 0, chunk_size = 0;
    BOOL check_ret;
#ifdef _BUILD_LOADER_COMBO_	
    CHUNK_HEADER blk_header;	  
    UINT32 flash_total_code_size=0;
#endif

    osal_flag_clear(g_ota_flg_id, OTA_FLAG_PTN);

    init_storage();

    setup_ota_parameters();

    MEMSET(&ver_info, 0, sizeof(struct otaupg_ver_info));
    check_ret=otaupg_check(&ver_info);
	#ifdef _SMI_ENABLE_
    //reset the main code vendor ID
    vendor_id_select(VID_CHOOSE_VMX_VENDORID);

    #ifdef NEW_OTA_FLOW
    if((check_ret!=TRUE)||(ver_info.m_m_ver != ver_info.s_m_ver)
        || (ver_info.m_m_ver == 0) || (ver_info.o_m_m_ver == 0))    
    #else
    if((check_ret!=TRUE)||(ver_info.m_m_ver != ver_info.s_m_ver) \
		|| (ver_info.s_bl_m_ver != ver_info.s_m_ver) || (ver_info.m_m_ver != ver_info.s_bl_m_ver))    
    #endif
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Invalid OTA data, cannot burn it!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
	#else
	if((check_ret!=TRUE)||(ver_info.m_m_ver != ver_info.s_m_ver))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Invalid OTA data, cannot burn it!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
	#endif

    UINT32 upgrade_size = 0;
	#ifdef _SMI_ENABLE_
    #ifndef NEW_OTA_FLOW
    if((ver_info.m_m_ver >= ver_info.m_f_ver)\
       &&(ver_info.s_m_ver >= ver_info.s_f_ver) &&(ver_info.s_bl_m_ver >= ver_info.s_bl_f_ver)) 
    {
        upgrade_size += m_ota_info.see_bl_code_size; 
        upgrade_size += m_ota_info.main_code_size;
        upgrade_size += m_ota_info.see_code_size;
    }
    #else
    if((ver_info.m_m_ver >= ver_info.m_f_ver)\
       &&(ver_info.s_m_ver >= ver_info.s_f_ver)) 
    {
        upgrade_size += m_ota_info.main_code_size;
        upgrade_size += m_ota_info.see_code_size;
    }
    #endif
	#else
	if((ver_info.m_m_ver >= ver_info.m_f_ver))
    {
        //upgrade_size += m_ota_info.see_bl_code_size; 
        upgrade_size += m_ota_info.main_code_size;
        upgrade_size += m_ota_info.see_code_size;
    }
	#endif
#ifdef _BUILD_LOADER_COMBO_
    else if (loader_check_osm_parameter()==3)
    {
        upgrade_size += m_ota_info.main_code_size;
        upgrade_size += m_ota_info.see_code_size;
    }
    
#else    
#ifndef _BUILD_OTA_E_
    else
    {
        //OSM_TRIGGERS* pOSM_Triggers=GetOsmTriggers();
        //if(pOSM_Triggers->OTA_Type==OSM_FORCED)
        if(backup_ota_type==OSM_FORCED)
        {
            upgrade_size += m_ota_info.main_code_size;
            upgrade_size += m_ota_info.see_code_size;
        }
    }
#endif
#endif
#ifdef _SMI_ENABLE_
	//if(ver_info.rsa_m_ver > ver_info.rsa_f_ver) //ca switching, update rsa key and ECGK
	if(ca_switch_mode==1)
	{
		upgrade_size += m_ota_info.rsa_key_size;
		upgrade_size += m_ota_info.ecgk_key_size;
	}
#endif

    if (NUM_ZERO==upgrade_size )
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Nothing need to be upgrade!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }

#ifndef _BUILD_OTA_E_ 
    if(check_main_see_size(m_ota_info.main_code_size + m_ota_info.see_code_size))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Upgrade code size is too big, cannot do upgrade", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;		
    }
#endif
    
#ifdef NEW_OTA_FLOW
#ifndef _BUILD_OTA_E_
    if(!check_chunk_addr(MAIN_CODE_CHUNK_ID,\
        m_ota_info.main_code_addr ))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("OTA stream flash mapping don't match !", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;
    }
#endif    
#endif

#ifndef _BUILD_LOADER_COMBO_
#ifndef _BUILD_OTA_E_
#ifdef _OUC_LOADER_IN_FLASH_    
    upgrade_size += m_ota_info.ota_main_code_size;
    upgrade_size += m_ota_info.ota_see_code_size;
    upgrade_size += 0x10000;//ota param.
#else
    upgrade_size += m_ota_info.ota_bin_size;
    upgrade_size += m_ota_info.user_db_len;
#endif
#ifdef _SMI_ENABLE_
    if(ca_switch_mode == 1)
    {
       upgrade_size += 0x20000; //upgrade size. 
       #ifndef _BUILD_OTA_E_
       upgrade_size += 0x20000; //backup rsa and ecgk to user db
       #endif
    }	
#endif
#else
#ifndef _OUC_LOADER_IN_FLASH_ 
    upgrade_size += m_ota_info.user_db_len;
#endif
#endif
#endif

    m_ota_info.update_total_sectors = upgrade_size / C_SECTOR_SIZE;
    m_ota_info.update_secotrs_index = 0;

    INT32 ret = 0;
    UINT32 flash_pos;
    do
    {
    #ifndef _BUILD_OTA_E_
        #ifndef _OUC_LOADER_IN_FLASH_
        // TODO: step 1: backup UserDB
        OTA_PRINTF("BackupDataBase!\n");
        ret = ota_upg_backup_database();
        if (SUCCESS != ret)
            break;        
        
        // TODO: step 2: burn OTA to UserDB
        OTA_PRINTF("Burn OTALOADER!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.ota_bin_addr,\
          m_ota_info.user_db_start_sector , m_ota_info.ota_bin_sectors);
        if(SUCCESS != ret)
            break;
        #endif
    #ifdef _OUC_LOADER_IN_FLASH_
        #ifdef _SMI_ENABLE_ 
        chunk_addr = get_chunk_addr(OUC_CHUNK_ID,CHUNKID_VENDOR_ID,\
                (m_ota_info.ota_main_code_addr -CHUNK_HEADER_SIZE));//
        #else
        chunk_addr = get_chunk_addr(OUC_CHUNK_ID,CHUNKID_VENDOR_ID,\
                (m_ota_info.ota_main_code_addr -CHUNK_HEADER_SIZE));//
        #endif
        chunk_addr += CHUNK_HEADER_SIZE;
        chunk_size = m_ota_info.ota_main_code_size;
        OTA_PRINTF("upgrade OUC main code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.ota_main_code_addr, flash_pos, \
          chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;

        chunk_addr += chunk_size;
        chunk_size = m_ota_info.ota_see_code_size + 0x10000;
        OTA_PRINTF("upgrade OUC see code and param\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.ota_see_code_addr, flash_pos, \
          chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;
    #endif
	#ifdef _SMI_ENABLE_  
        if(ca_switch_mode == 1)
        {
    		// TODO:  burn RSA key and ECGK to UserDB last sector		
    		OTA_PRINTF("Burn RSA key and ECGK  to Last 4 sector !\n");  
            m_ota_info.backup_key_addr = SMI_KEY_BACKKUP_ADDR;
            flash_pos = m_ota_info.backup_key_addr / C_SECTOR_SIZE;
    		ret = ota_upg_burn_block((UINT8*)m_ota_info.rsa_key_addr, flash_pos , 1);
    		if(SUCCESS != ret)
                break;
    		ret = ota_upg_burn_block((UINT8*)m_ota_info.ecgk_key_addr, flash_pos + 1, 1);
    		if(SUCCESS != ret)
                break;      
        }
	#endif
	#endif
    #ifdef _SMI_ENABLE_
		// TODO:  calculate  RSA key and ECGK chunk addr
		//if(ver_info.rsa_m_ver > ver_info.rsa_f_ver) //ca switching, update rsa key
		if(ca_switch_mode == 1)
		{
			//upgrade RSA key
			chunk_id = RSA_KEY_CHUNK_ID;
			chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
			if ((UINT32)ERR_FAILUE == chunk_addr)
			{				
                chunk_addr = get_chunk_addr(RSA_KEY_CHUNK_ID,OUC_CHUNK_ID,\
                    m_ota_info.rsa_key_addr );
                if(chunk_addr > 0x400000 || (!m_ota_info.rsa_key_addr))
                {
                    OTA_PRINTF("Cannot find rsa key chunk!\n");
				    ret = ERR_FAILED;
				    break;
                }
			}
			// burn new rsa key
			chunk_size = m_ota_info.rsa_key_size;
			OTA_PRINTF("upgrade rsa key\n");
			flash_pos = chunk_addr / C_SECTOR_SIZE;
			ret = ota_upg_burn_block((UINT8 *)m_ota_info.rsa_key_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
			if (SUCCESS != ret)
				break;

			//upgrade ECGK
			#if 0
			chunk_id = ECGK_CHUNK_ID;
			chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
			if ((UINT32)ERR_FAILUE == chunk_addr)
			{
				OTA_PRINTF("Cannot find ecgk chunk!\n");
				ret = ERR_FAILED;
				break;
			}
			#else
			chunk_addr += chunk_size;
			#endif
			// burn new ecgk
			chunk_size = m_ota_info.ecgk_key_size;
			OTA_PRINTF("upgrade ecgk\n");
			flash_pos = chunk_addr / C_SECTOR_SIZE;
			ret = ota_upg_burn_block((UINT8 *)m_ota_info.ecgk_key_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
			if (SUCCESS != ret)
				break;
		}
	#endif


        // TODO: step 3: calculate see bloader code chunk_addr
	#ifdef _SMI_ENABLE_
		if(ca_switch_mode == 0)
		{
            #ifndef NEW_OTA_FLOW
			chunk_id = SEEBOOTLOADERCODE_ID;
			chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
			if ((UINT32)ERR_FAILUE == chunk_addr)
			{
				//find the prev chunk,and the calculate maincode chun addr		
				chunk_id = ECGK_CHUNK_ID;
				chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
				chunk_size = m_ota_info.ecgk_key_size;
			}	
			else
			{
				chunk_size = 0;
			}
            #endif
		}
	/*#else 
	    SEE BL not upgrade NOW.
		chunk_id = SEEBOOTLOADERCODE_ID;
        chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if ((UINT32)ERR_FAILUE == chunk_addr)
        {
            OTA_PRINTF("Cannot find maincode's chunk!\n");
            ret = ERR_FAILED;
            break;		
        }
        chunk_size = 0;*/
    #endif	

#if 0//ndef NEW_OTA_FLOW
        // burn new seebootloadercode.bin
        // see boot loader code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.see_bl_code_size;
        OTA_PRINTF("upgrade main code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.see_bl_code_addr, flash_pos, \
          chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;
#endif

        // burn new maincode.bin
        // main code chunk addr and size
        #ifdef NEW_OTA_FLOW
        #ifdef _SMI_ENABLE_
        //SMI
        chunk_addr = get_chunk_addr(MAIN_CODE_CHUNK_ID,DEFAULT_DB_ID,\
                m_ota_info.main_code_addr);
        #else
        //BC
        chunk_addr = get_chunk_addr(MAIN_CODE_CHUNK_ID,DEFAULT_DB_ID,\
                m_ota_info.main_code_addr);
        #endif
        #else
        chunk_addr += chunk_size;
        #endif
        chunk_size = m_ota_info.main_code_size;
        OTA_PRINTF("upgrade main code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.main_code_addr, flash_pos, \
          chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;

        // see code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = m_ota_info.see_code_size;
        OTA_PRINTF("upgrade see code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
        ret = ota_upg_burn_block((UINT8 *)m_ota_info.see_code_addr, flash_pos,\
          chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
            break;

    #ifndef _OUC_LOADER_IN_FLASH_
    #ifndef _BUILD_OTA_E_    
        // TODO: burn back UserDB
        OTA_PRINTF("BurnBack UserDB!\n");
        ret = ota_upg_burn_block((UINT8*)m_ota_info.backup_db_addr, \
          m_ota_info.user_db_start_sector, \
          m_ota_info.user_db_sectors);
        if(SUCCESS != ret)
            break;         
    #elif !defined(_BUILD_LOADER_COMBO_) 
        OTA_PRINTF("reset UserDB!\n");
        ret = ota_upg_burn_block((UINT8*)NULL, m_ota_info.user_db_start_sector, \
          m_ota_info.user_db_sectors);
        if(SUCCESS != ret)
            break;
	#endif	
    #endif
    } while(0);

    osal_flag_set(g_ota_flg_id,(SUCCESS == ret) ? PROC_SUCCESS : PROC_FAILURE);
    return ret;
}
#undef WIN_OTAUPG_BURNFLASH_BC

#else
#ifdef _NV_PROJECT_SUPPORT_ 
static unsigned int get_unzip_size(unsigned char *in)
{
    unsigned int size;
    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

BOOL otaupg_check(void)
{
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0;
    UINT8 *data;
    UINT32 data_len;
    UINT8 *block_addr;
    int ret;
    UINT8* signature;
    UINT32 len, maxLen;
    UINT8 mode, errorCode;
    unsigned int header_crc=0;
    //UINT32 block_header, version;
    CHUNK_HEADER blk_header;
    UINT8 verify_result=0;

    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)get_unzip_size((UINT8 *)m_ota_info.compressed_addr);

#if 1
    ret = upg_ssd_check((TUnsignedInt8*)m_ota_info.uncompressed_addr, (TUnsignedInt32)m_ota_info.uncompressed_len, NULL, NULL);
    if(SSD_CHECK_VALID_NO_ERROR != ret)
    {
        OTA_PRINTF("SSD check fail!\n");
        return FALSE;
    }
#endif

    init_upg_bock((UINT8 *)m_ota_info.uncompressed_addr, (UINT32)m_ota_info.uncompressed_len, &ota_upgrade_size);

    if(0 == ota_upgrade_size)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static INT32 ota_upg_verify_block(UINT8 *pBuffer, UINT32 Pos, UINT8 NumSectors)
{
    struct sto_device *sto_dev;
    UINT8 *read_buf = NULL;
    INT32 offset = 0;
    UINT8 i;

    read_buf = MALLOC(C_SECTOR_SIZE);
    if(NULL == read_buf)
    {
        goto ERR;
    }
    OTA_PRINTF("[%s(%d)] read_buf addr=0x%x\n",__FUNCTION__,__LINE__, read_buf);

    sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    for(i=0; i<NumSectors; i++)
    {
        offset = (Pos + i) * C_SECTOR_SIZE;
        if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
        {
            OTA_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
            goto ERR;
        }

        if(sto_read(sto_dev, (UINT8 *)read_buf, C_SECTOR_SIZE) != C_SECTOR_SIZE)
        {
            OTA_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
            goto ERR;
        }

        if(0 != MEMCMP(read_buf, &pBuffer[C_SECTOR_SIZE * i], C_SECTOR_SIZE))
        {
            OTA_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
            goto ERR;
        }
        else
        {
            break;
        }
    }

SUC:
    free(read_buf);
    return SUCCESS;

ERR:
    OTA_PRINTF("[%s(%d)] verify_block fail\n",__FUNCTION__,__LINE__);
    free(read_buf);
    return ERR_FAILED;

}

INT32 win_otaupg_burnflash(void)
{
    BOOL fw_not_match = FALSE;
    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_BURNFLASH;
    win_popup_choice_t choice;
    UINT8 back_saved;
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id, chunk_addr, chunk_size;
    BOOL check_ret;
    INT32 ret = 0;
    UINT32 flash_pos;
    INT32 id = 0;
    int da2_update_retry_cnt = 2;
    int da2_update_done = 0; /* 0:not done; 1:done */
    block_info_t block_infor;
    block_info_t sto_block_infor;

    osal_flag_clear(g_ota_flg_id, OTA_FLAG_PTN);

    init_storage();

    //setup_ota_parameters();

    check_ret = otaupg_check();
    if(check_ret!=TRUE)
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Invalid data or Nothing need to be upgraded!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(3000);
        win_compopup_smsg_restoreback();
        goto ERR;
    }

    m_ota_info.update_total_sectors = ota_upgrade_size / C_SECTOR_SIZE;
    m_ota_info.update_secotrs_index = 0;

    //update DA2
    block_info_get(FLASH_BLK_DA2, &block_infor);
    sto_block_info_get(FLASH_BLK_DA2, &sto_block_infor);
    chunk_size = sto_block_infor.block_len;
    chunk_addr = sto_block_infor.block_start;
    flash_pos = chunk_addr / C_SECTOR_SIZE;
    #if 0
    if(TRUE ==is_da2_change(block_infor, sto_block_infor))
    #else
    if(TRUE == is_block_data_change(block_infor, sto_block_infor))
    #endif
    {
        OTA_PRINTF("[%s(%d)] chunk_size=0x%x, chunk_addr=0x%x, flash_pos=0x%x\n",
                __FUNCTION__,__LINE__, chunk_size, chunk_addr, flash_pos);
        while(da2_update_retry_cnt--)
        {
            OTA_PRINTF("[%s(%d)] update DA2 da2_update_retry_cnt=%d\n", __FUNCTION__,__LINE__, da2_update_retry_cnt);

            if(SUCCESS != ota_upg_burn_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE))
            {
                OTA_PRINTF("[%s(%d)] burn DA2 fail\n", __FUNCTION__,__LINE__);
                continue;
            }

            if(SUCCESS != ota_upg_verify_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE))
            {
                OTA_PRINTF("[%s(%d)] verify DA2 fail\n", __FUNCTION__,__LINE__);
                continue;
            }
            else
            {
                da2_update_done = 1;
                break;
            }
        }

        if(0 == da2_update_done)
        {
            OTA_PRINTF("[%s(%d)] DA2 update fail\n", __FUNCTION__,__LINE__);
            goto ERR;
        }
    }

    //update other blocks
    for(id=FLASH_BLK_SCS_TOTAL_AREA; id<BLK_MAX; id++)
    {
        if((FLASH_BLK_MAIN_SW != id) && (FLASH_BLK_SEE_SW != id) && (FLASH_BLK_LOGO != id))
        {
            continue;
        }

        block_info_get(id, &block_infor);
        sto_block_info_get(id, &sto_block_infor);
    #if 1
        OTA_PRINTF("[%s(%d)] id=%d, block_infor.version=%d, sto_block_infor.version=%d\n",
            __FUNCTION__,__LINE__, id, block_infor.version, sto_block_infor.version);
    #endif
    #if 0
        if(block_infor.version > sto_block_infor.version)
    #else
        //every block should compare version and data
        if(TRUE == is_block_data_change(block_infor, sto_block_infor))
    #endif
        {
            chunk_size = sto_block_infor.block_len;
            chunk_addr = sto_block_infor.block_start;
            flash_pos = chunk_addr / C_SECTOR_SIZE;
            OTA_PRINTF("[%s(%d)] chunk_size=0x%x, chunk_addr=0x%x, flash_pos=0x%x\n",
                __FUNCTION__,__LINE__, chunk_size, chunk_addr, flash_pos);
            ret = ota_upg_burn_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
            {
                goto ERR;
            }
        }
    }

SUCC:
    osal_flag_set(g_ota_flg_id, PROC_SUCCESS);
    otaset_switch_to_dvb();
    return SUCCESS;

ERR:
    osal_flag_set(g_ota_flg_id, PROC_FAILURE);
    otaset_switch_to_dvb();
    return ERR_FAILED;
}
#else
#ifdef M36F_CHIP_MODE
#ifdef DUAL_ENABLE
static BOOL otaupg_fw_signed_check(void)
{
    UINT8 bl_signed[256] = {0xFF};
    struct sto_device *psto_dev = NULL;
    UINT32 offset = 0;
    UINT32 cnt = 0;
    BOOL ret = FALSE;
    INT32 __MAYBE_UNUSED__ sto_get_data_ret=0;

    psto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    offset = sto_fetch_long(0+CHUNK_OFFSET) - 0x100;
    sto_get_data_ret=sto_get_data(psto_dev, bl_signed, offset, 0x100);
    for (cnt = 0; cnt < 256; cnt++)
    {
        if ((0xFF != bl_signed[cnt]) && (0x0 != bl_signed[cnt]))
        {
            ret = TRUE;
            break;
        }
    }
    if (SIGN_MAX_VAUE == cnt)
    {
        ret = FALSE;
    }

    return ret;
}

static BOOL otaupg_allcode_check(UINT8 *pbuff, UINT32 length)
{
    //struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0xFFFFFFFF;
    CHUNK_HEADER chunk_header;
    UINT8 *data_addr = NULL;
    UINT32 data_len = 0;
    BOOL ret = TRUE;

    MEMSET(&chunk_header,0,sizeof(CHUNK_HEADER));
    if ((NULL == pbuff) || (NUM_ZERO == length))
    {
        ASSERT(0);
        return FALSE;
    }

    chunk_init((UINT32)pbuff, length);
    // step 2: maincode check
    // get maincode raw data address & length
    chunk_id = MAINCODE_ID;
    if(!get_chunk_header(chunk_id, &chunk_header))
    {
        ret = FALSE;
        libc_printf("get main code chunk header failed!\n");
        return ret;
    }
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    data_len = chunk_header.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

    if (NUM_ZERO != test_rsa_ram((UINT32)data_addr, data_len))
    // this function operate RAW Data and RAW length
    {
        ret = FALSE;
        return ret;
    }

    // step 3: seecode check
    // get raw data address & length from memory
    chunk_id = SEECODE_ID;
    if (!get_chunk_header(chunk_id, &chunk_header))
    {
        ret = FALSE;
        return ret;
    }
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    data_len = chunk_header.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    if (NUM_ZERO != test_rsa_ram((UINT32)data_addr, data_len))
    {
        ret = FALSE;
        return ret;
    }

    return ret;
}
#endif
#endif

//RET
INT32 win_otaupg_burnflash(void)
{
    //BOOL fw_not_match = FALSE;
    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_BURNFLASH;
    //win_popup_choice_t choice;
    UINT8 back_saved;
    INT32 ret = 0;

    osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);

#if 1
    init_storage();
    ci_flag=bool_flash_mem();   //add by colin
    ret = setup_ota_parameters();
    if(ret == FALSE)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Invalid OTA stream !", NULL, 0);
        win_compopup_open_ext(&back_saved);
        win_compopup_smsg_restoreback();
        osal_flag_set(g_ota_flg_id, PROC_FAILURE);
        return -1;      
    }

#if 0 //it's not available for 3711C
    UINT32 ota_fw_chip_id = (*(UINT32 *)(m_ota_info.ota_fw_addr \
                           + CHUNK_HEADER_SIZE + 0x80)) & 0xFFFF0000;

    if( ALI_S3602F<=sys_ic_get_chip_id())
    {
        if(((*(UINT32 *)(0xb8000000))&0xffff0000) != ota_fw_chip_id)
            fw_not_match = TRUE;
    }
    else
    {
        if(OTA_FW_CHIP_ID == ota_fw_chip_id)
            fw_not_match = TRUE;
    }

    if (fw_not_match)
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg("FW/IC Version not match!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        if (WIN_POP_CHOICE_NO == choice)
        {
            osal_flag_set(g_ota_flg_id, PROC_FAILURE);
            return -1;
        }
    }
#endif    

#ifdef CI_PLUS_SUPPORT
    UINT8   *ci_plus_internal_keys;
    CHUNK_HEADER ci_plus_chuck_hdr;
    UINT32 ci_plus_chuck_id = 0x09F60101;

    if(!backup_chunk_data(m_ota_info.ota_fw_addr, \
       m_ota_info.ota_fw_size, ci_plus_chuck_id))
        return -1;

#endif

#ifdef M36F_CHIP_MODE
#ifdef DUAL_ENABLE
    if (TRUE == otaupg_fw_signed_check())
    {
        if (FALSE == otaupg_allcode_check\
       ((UINT8 *)m_ota_info.ota_fw_addr, m_ota_info.ota_fw_size))
            return -1;
    }
#endif
#endif

    ret = burn_flash();
    osal_flag_set(g_ota_flg_id,(SUCCESS == ret)?PROC_SUCCESS:PROC_FAILURE);
#else

    //ota_start_burnflash(win_ota_upgrade_process_update);
    ota_start_burnflash(win_ota_upgrade_process_update_ex);
#endif
    return 0;
}
#endif
#endif
#endif
#endif /* _CAS9_CA_ENABLE_ */

INT32 win_otaupg_reboot(void)
{
    ota_proc_ret = 0;
    ota_user_stop = 0;
    //ota_proc_step = OTA_STEP_BURNFLASH;
    #ifdef AUTO_OTA
    set_ota_upgrade_found(FALSE);
    #endif
    osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);

#if defined _NV_PROJECT_SUPPORT_  && defined _BUILD_UPG_LOADER_
    dvb_upgloader_switch(SWITCH_TO_DVB, UPGTYPE_OTA);
#endif

    key_pan_display("OFF ",4);
    osal_task_sleep(500);
    power_off_process(0);

    // Oncer 2011/03/0915:43
    // for OTA upgrade, we need to save UserDB,
    // so don't need to do factory init!!!
//    sys_data_factroy_init();
//    sys_data_set_factory_reset(TRUE);
    sys_data_set_cur_chan_mode(TV_CHAN);

    power_on_process();

    while(1)
    {
         ;
     }
     return 0;
}


void win_otaup_set_btn_msg_display(BOOL update)
{
    MULTI_TEXT *mtxt_msg=NULL;
    TEXT_FIELD *txt_btn=NULL;
    UINT16 btn_strid=0;
    TEXT_CONTENT *tcont=NULL;
    UINT16 cur_sw_ver = (UINT16)(SYS_SW_VERSION&0xffff);
    UINT8 *src=NULL;
    UINT8 str_len=0;
    INT32 btn_state=0;
    LPVSCR apvscr = NULL;
    //PRESULT obj_ret=PROC_PASS;
    UINT32 trans_ret=0;
    int cpy_ret=0;

	if(0 == trans_ret)
	{
		;
	}
	if(0 == cpy_ret)
	{
		;
	}
	MEMSET(&apvscr,0,sizeof(LPVSCR));
    tcont = otamsg_mtxt_content;
    tcont->b_text_type = STRING_ID;

    mtxt_msg = &otaupg_mtxt_msg;
    txt_btn = &otaupg_txt_btn;

/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
    btn_state = win_otaup_get_btn_state();
    btn_strid = otaupg_get_btn_strid(btn_state);

    switch(ota_proc_step)
    {
    case OTA_STEP_GET_INFO:

        switch(ota_proc_ret)
        {
        case 0:
            tcont->text.w_string_id = RS_OTA_COLLECT_INFORMATION;
            break;
        case PROC_SUCCESS:

            if(ota_dl_info.sw_version <= cur_sw_ver)
            {
                if(ota_dl_info.sw_version < cur_sw_ver)
                {
                            trans_ret=com_asc_str2uni((UINT8 *)"SW is older than your STB's SW!\
                               Do you want to continue?",len_display_str);
                }
                else
                {
                trans_ret=com_asc_str2uni((UINT8 *)"SW is same to your STB's SW! \
                    Do you want to continue?",len_display_str);
                }
            }
            else
            {
                src = osd_get_unicode_string(RS_DISPLAY_NEXT);
                cpy_ret=com_uni_str_copy_char((UINT8*)len_display_str, src);
                str_len = com_uni_str_len(len_display_str);
                trans_ret=com_asc_str2uni((UINT8 *)" : ",&len_display_str[str_len]);
                str_len = com_uni_str_len(len_display_str);
                src = osd_get_unicode_string(RS_DISPLAY_DOWNLOADING);
                cpy_ret=com_uni_str_copy_char\
                ((UINT8*)&len_display_str[str_len], src);
            }

            tcont->b_text_type = STRING_UNICODE;
            tcont->text.p_string = len_display_str;

            break;
        case PROC_STOPPED:
            trans_ret=com_asc_str2uni((UINT8 *)"User canceled",len_display_str);

            tcont->b_text_type = STRING_UNICODE;
            tcont->text.p_string = len_display_str;
            break;
        case PROC_FAILURE:
            tcont->text.w_string_id = \
        RS_OTA_COLLECT_INFORMATION_FAIL;
            break;
        default:
            break;
        }

        break;
    case OTA_STEP_DOWNLOADING:

        switch(ota_proc_ret)
        {
        case 0:
            tcont->text.w_string_id = RS_DISPLAY_DOWNLOADING;
            break;
        case PROC_SUCCESS:
            src = osd_get_unicode_string(RS_DISPLAY_NEXT);
            cpy_ret=com_uni_str_copy_char((UINT8*)len_display_str, src);
            str_len = com_uni_str_len(len_display_str);
            trans_ret=com_asc_str2uni((UINT8 *)" : ",&len_display_str[str_len]);
            str_len = com_uni_str_len(len_display_str);
            src = osd_get_unicode_string(RS_OTA_BURNFLASH);
            cpy_ret=com_uni_str_copy_char((UINT8*)&len_display_str[str_len], src);

            tcont->b_text_type = STRING_UNICODE;
            tcont->text.p_string = len_display_str;

            break;
        case PROC_STOPPED:
            trans_ret=com_asc_str2uni((UINT8 *)"User canceled",len_display_str);

            tcont->b_text_type = STRING_UNICODE;
            tcont->text.p_string = len_display_str;
            break;
        case PROC_FAILURE:
            tcont->text.w_string_id = RS_DISPLAY_DOWNLOAD_NOT_FINISH;
            break;
        default:
            break;
        }
        break;

    case OTA_STEP_BURNFLASH:

        switch(ota_proc_ret)
        {
        case 0:
            tcont->text.w_string_id = RS_OTA_BURNFLASH_DO_NOT_POWER_OFF;
            break;
        case PROC_SUCCESS:
            src = osd_get_unicode_string(RS_DISPLAY_NEXT);
            cpy_ret=com_uni_str_copy_char((UINT8*)len_display_str, src);
            str_len = com_uni_str_len(len_display_str);
            trans_ret=com_asc_str2uni((UINT8 *)" : ",&len_display_str[str_len]);
            str_len = com_uni_str_len(len_display_str);
            src = osd_get_unicode_string(RS_MSG_UPGRADE_REBOOT);
            cpy_ret=com_uni_str_copy_char((UINT8*)&len_display_str[str_len], src);

            tcont->b_text_type = STRING_UNICODE;
            tcont->text.p_string = len_display_str;
            break;
        //case PROC_STOPPED:
        //    break;
        case PROC_FAILURE:
            tcont->text.w_string_id = RS_MSG_UPGRADE_BURN_FLASH_FAIL;
            break;
        default:
            break;
        }
        break;


    default:
            break;
    }

#ifdef _NV_PROJECT_SUPPORT_ 
    btn_strid = RS_DISPLAY_PLEASE_WAIT;
    tcont = otamsg_mtxt_content;
    tcont->b_text_type = STRING_ID;
    tcont->text.w_string_id = RS_DISPLAY_PLEASE_WAIT;
#else
    osd_set_text_field_content(txt_btn, STRING_ID, (UINT32)btn_strid);
#endif

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
        apvscr = osd_get_task_vscr(osal_task_get_current_id());
        if((apvscr != NULL) && (apvscr->lpb_scr != NULL))
        {
            apvscr->update_pending = 1;
            osd_update_vscr(apvscr);
        }
        if((OTA_STEP_BURNFLASH==ota_proc_step  ) &&( NUM_ZERO==ota_proc_ret))
        {
            osd_draw_object( (POBJECT_HEAD)txt_btn, C_UPDATE_ALL);
        }
        else
        {
            osd_track_object((POBJECT_HEAD)txt_btn, C_UPDATE_ALL);
        }
        apvscr = osd_get_task_vscr(osal_task_get_current_id());
        if((apvscr != NULL) && (apvscr->lpb_scr != NULL))
        {
            apvscr->update_pending = 1;
            osd_update_vscr(apvscr);
        }
    }
}

void win_otaup_set_swinfo_display(BOOL update)
{
    MULTI_TEXT *mtxt_swinfo=NULL;
    TEXT_CONTENT *tcont=NULL;
    struct dl_info *info=NULL;
    char str[50]={0};
    //PRESULT obj_ret=PROC_PASS;
    UINT32 str_ret=0;
    int sprintf_ret __MAYBE_UNUSED__=0;

	if(0 == str_ret)
	{
		;
	}
    mtxt_swinfo = &otaupg_mtxt_swinfo;
    info = &ota_dl_info;

    tcont = &otasw_mtxt_content[0];
    tcont->text.p_string = display_strs[10];

    tcont = &otasw_mtxt_content[1];
    tcont->text.p_string = display_strs[11];

    tcont = &otasw_mtxt_content[2];
    tcont->text.p_string = display_strs[12];


    if( ((OTA_STEP_GET_INFO == ota_proc_step) && (PROC_SUCCESS == ota_proc_ret))
        || (ota_proc_step>OTA_STEP_GET_INFO))
    {
        tcont = &otasw_mtxt_content[0];
#if (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_)\
    || defined(_BC_CA_ENABLE_) || defined(FTA_ONLY))
        snprintf(str, 50, "     H/W ver : %d.%d.%d.%d",
            (info->hw_version>>12) & 0xf,
            (info->hw_version>>8)  & 0xf,
            (info->hw_version>>4)  & 0xf,
            (info->hw_version>>0)  & 0xf);
#else
       snprintf(str, 50, "    H/W ver : %d.%d",
            (info->hw_version>>4) & 0xfff,
            (info->hw_version>>0) & 0xf);
#endif
        str_ret=com_asc_str2uni((UINT8 *)str,tcont->text.p_string);

        tcont = &otasw_mtxt_content[1];
#if (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_)\
     || defined(_BC_CA_ENABLE_)|| defined(FTA_ONLY))
        sprintf_ret = snprintf(str, 50, "     S/W ver : %d.%d.%d.%d",
            (info->sw_version>>12) & 0xf,
            (info->sw_version>>8)  & 0xf,
            (info->sw_version>>4)  & 0xf,
            (info->sw_version>>0)  & 0xf);
#else
        snprintf(str, 50, "    S/W ver : %d.%d.%d",
            (info->sw_version>>8) & 0xff,
            (info->sw_version>>4) & 0xf,
            (info->sw_version>>0) & 0xf);
#endif
        str_ret=com_asc_str2uni((UINT8 *)str,tcont->text.p_string);

        tcont = &otasw_mtxt_content[2];
        snprintf(str, 50, "    S/W size : %lu bytes",info->sw_size);
        com_asc_str2uni((UINT8 *)str,tcont->text.p_string);
    }
    else
    {
        tcont = &otasw_mtxt_content[0];
        tcont->text.p_string[0] = 0;//str_ret=ComAscStr2Uni("",tcont->text.pString);

        tcont = &otasw_mtxt_content[1];
        tcont->text.p_string[0] = 0;//str_ret=ComAscStr2Uni("",tcont->text.pString);

        tcont = &otasw_mtxt_content[2];
        tcont->text.p_string[0] = 0;//str_ret=ComAscStr2Uni("",tcont->text.pString);
    }

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)mtxt_swinfo, C_UPDATE_ALL);
    }
}


void win_ota_upgrade_process_update_old(UINT32 process)
{
    PROGRESS_BAR *bar = &otaupg_progress_bar;
    TEXT_FIELD  *txt = &otaupg_progress_txt;
    TEXT_CONTENT tcont;
    MULTI_TEXT *mtxt_msg = &otaupg_mtxt_msg;
    UINT16 str_buf[50]={0};
    char    progress_strs[30]={0};
    LPVSCR apvscr = NULL;

    UINT32 str_ret=0;
    int sprintf_ret=0;
    //PRESULT obj_ret=PROC_PASS;

	if(0 ==sprintf_ret)
	{
		;
	}
	if(0 ==str_ret)
	{
		;
	}
    MEMSET(&apvscr,0,sizeof(LPVSCR));
    MEMSET(&tcont,0,sizeof(TEXT_CONTENT));
#ifdef WATCH_DOG_SUPPORT
    dog_set_time(0, 0);
#endif
    tcont.text.p_string = str_buf;
    tcont.b_text_type = STRING_UNICODE;
    osd_task_buffer_init(osal_task_get_current_id(),NULL);
    if(OTA_START_BACKUP_FLAG == process)
    {
        str_ret=com_asc_str2uni((UINT8 *)"Start backup OTA data ...", str_buf);
        osd_set_multi_text_content(mtxt_msg, &tcont);
        osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
        osd_set_multi_text_content(mtxt_msg, &otamsg_mtxt_content[0]);
    }
    else if(OTA_START_END_FLAG == process)
    {
        osd_set_multi_text_content(mtxt_msg, &otamsg_mtxt_content[0]);
        osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
    }
    else
    {
        if(OTA_UPDATE_PROGRESS<process )
        {
            process = 0;
        }

        osd_set_progress_bar_pos(bar,process);
        sprintf_ret=snprintf(progress_strs,30, "%lu%%",process);
        osd_set_text_field_content(txt,STRING_ANSI,(UINT32)progress_strs);

        osd_draw_object((POBJECT_HEAD)bar,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
    }
    apvscr = osd_get_task_vscr(osal_task_get_current_id());
    if (NULL == apvscr)
    {
        return;
    }
    if(apvscr->lpb_scr != NULL)
    {
        apvscr->update_pending = 1;
    }
    osd_update_vscr(apvscr);
}



void win_ota_upgrade_process_update_ex(UINT32 process)
{
    //UINT32 old_process = 0;
    //BOOL send_msg_flag=FALSE;

    if(old_process == process)
    {
        return;
    }
    //send_msg_flag = 
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_OTA_PROCESS_MSG,process,TRUE);
#ifdef WATCH_DOG_SUPPORT
    dog_set_time(0, 0);
#endif
    osal_task_sleep(1);
    old_process = process;
    //libc_printf("2 process:%d\n",process);
}
#endif


