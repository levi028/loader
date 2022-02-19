/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_task_init.c
 *
 *    Description: This source file contains control task initialize process.
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
#include <hld/sto/sto.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
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
#include <api/libsi/si_tdt.h>
#include <api/libmp/pe.h>
#include <api/libfs2/fs_main.h>
#include <api/libchunk/chunk.h>
#include "osd_rsc.h"
#include "copper_common/com_api.h"
#include "platform/board.h"
#include "control.h"
#include "usb_tpl.h"
#include "disk_manager.h"
#include "menus_root.h"
#include "ctrl_debug.h"
#include "ctrl_task_init.h"
#include "win_mute.h"
#include "ap_ctrl_time.h"
#include "key.h"
#include "ap_ctrl_ci.h"
#include "win_epg.h"
#include "win_prog_name.h"
#include "win_time.h"
#include "win_mainmenu_submenu.h"
#include "win_tvsys.h"
#include "ota_ctrl.h"
#ifdef BG_TIMER_RECORDING
#include "pvr_ctrl_timer.h" //derek201406,by derek.yang 2014 Jun related to timer rec (stand by)
#endif
#if defined(HDCP_BY_URI) && defined(CAS9_V6) 
#include "conax_ap/win_ca_uri.h"
#endif

#if (defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
#include "ctrl_cas_ap.h"
#include "conax_ap/cas_fing_msg.h"
#ifdef CAS9_VSC
#include <api/libcas/mcas.h>
#endif
#endif

#ifdef _C0700A_VSC_ENABLE_
#include "ctrl_cas_ap.h"
#endif

#ifdef NETWORK_SUPPORT
#include <hld/net/ethtool.h>
#include <hld/wifi/wifi_api.h>
#include "ctrl_network.h"
#endif
#if defined(SUPPORT_BC)
#include <api/libcas/bc/bc_consts.h>
#endif
#ifdef USB3G_DONGLE_SUPPORT
#include <api/libusb3g/lib_usb3g.h>
#endif

#ifdef _DTGSI_ENABLE_
#include "ap_ctrl_dtg.h"
#endif

#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif

#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#ifdef SFU_TEST_SUPPORT
#include "sfu_test.h"
#endif

#ifdef SAT2IP_SUPPORT
#include <api/libsat2ip/sat2ip.h>
#include "sat2ip/sat2ip_control.h"
#include "sat2ip/sat2ip_upnp.h"
#endif

#ifdef CEC_SUPPORT
#include <api/libcec/lib_cec.h>
#include "cec_link.h"
#endif

#ifdef _BUILD_OUC_LOADER_
#include "win_ota_set.h"
#endif

#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_mmap.h>
#ifdef _BUILD_UPG_LOADER_
#include <udi/nv/ca_upg.h>
#include "win_ota_set.h"
#endif
OSAL_ID id_dsc_mutex = NULL;
#endif
#include "gos_ui/gacas_loader_db.h"//new or change add by yuj

#define MEMORY_CACHE_CONVERT    0x8fffffff
#define MAX_FLASH_CHECK_CNT     8

#ifdef _INVW_JUICE
int         inview_deferred_start = 0;
#endif

#if (defined(SUPPORT_BC_STD)&& defined(BC_PATCH_BC_IOCTL))||defined(SUPPORT_BC)
extern void ioctl_check_pin_function(enBcNotify_t en_bc_notify);
#endif

#ifdef WIFI_SUPPORT
extern void ap_wifi_display_callback(E_WIFI_MSG_TYPE type, UINT32 param);
#endif

#ifdef SAT2IP_SERVER_SUPPORT
extern BOOL http_mg_server_is_started(void);
extern BOOL http_mg_server_start(void);
extern BOOL media_provider_pvr_init(void);
#endif

#ifdef HILINK_SUPPORT
int hilink_attach(void);
#endif

#ifdef _BC_CA_NEW_
extern BOOL loader_set_run_parameter(BOOL set,BOOL type);
extern BOOL loader_check_run_parameter();
#endif
#ifndef _BUILD_OTA_E_
static void cb_vdec_output_mode_check(UINT32 param1, UINT32 __MAYBE_UNUSED__ param2)
{
    BOOL            ret = FALSE;
    enum tvsystem   tv_mode = TV_SYS_INVALID;
    BOOL            bprogressive = FALSE;
    enum tvsystem   tv_mode_pre = TV_SYS_INVALID;

    BOOL        bprogressive_pre = FALSE;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    api_get_tv_mode_pre(&tv_mode_pre, &bprogressive_pre);
    if (TV_MODE_AUTO == sys_data_get_tv_mode())
    {
        if (RET_SUCCESS == vdec_io_control(get_selected_decoder(), VDEC_IO_GET_MODE, (UINT32) (&tv_mode)))
        {
            bprogressive = sys_data_is_progressive(tv_mode_to_sys_data(tv_mode));
            if ((tv_mode_pre != tv_mode || bprogressive_pre != bprogressive)
            && VIEW_MODE_PREVIEW != hde_get_mode() && system_state != SYS_STATE_USB_MP)
            {
                ret = TRUE;
            }
        }
    }

    *(BOOL *) (param1) = ret;
}
#endif

#ifdef SUPPORT_FRANCE_HD
static void  ap_france_hd_init(UINT32 db_addr, UINT32 db_len)
{
    struct NIT_VER_INFO     ver_info;
    struct nit_section_info *nit_info = NULL;
    struct NIT_VER_INFO     *ser_info = NULL;
    //init nit info setting
    INT32 malloc_size = 0;
    INT32 ret = RET_FAILURE;

    init_nit_info((db_addr + db_len + (64 * 1024)), 64 * 1024);
    if (load_nit_data((UINT8 *) (&ver_info), sizeof(struct NIT_VER_INFO)) == SUCCESS)
    {
        malloc_size = sizeof(struct NIT_VER_INFO) + sizeof(struct NIT_SER_INFO) * (ver_info.num - 1);
        ser_info = (struct NIT_VER_INFO *)malloc(malloc_size);
        if(NULL == ser_info)
        {
            ASSERT(0);
            return;
        }
        MEMSET(ser_info, 0, malloc_size);
        ret = load_nit_data(ser_info, malloc_size);
        if (ret == SUCCESS)
        {
            nit_info = (struct nit_section_info *)malloc(sizeof(struct nit_section_info));
            if(NULL == ser_info)
            {
                ASSERT(0);
                return;
            }
            nit_info->version_num = ver_info.verison_number;
            if (ver_info.num > PSI_MODULE_MAX_NIT_SINFO_NUMBER)
            {
                ver_info.num = PSI_MODULE_MAX_NIT_SINFO_NUMBER;
            }

            nit_info->s_nr = ver_info.num;
            for (i = 0; i < nit_info->s_nr; i++)
            {
                nit_info->s_info[i].onid = ser_info->nit_service[i].onid;
                nit_info->s_info[i].sid = ser_info->nit_service[i].sid;
                nit_info->s_info[i].tsid = ser_info->nit_service[i].tsid;
            }

            nit_monitor_init(nit_info);
            free(nit_info);
        }

        free(ser_info);
    }

}
#endif

#ifdef RAM_TMS_TEST
static void init_ram_tms_test(void)
{
    UINT32  ram_len = RAM_DISK_SIZE;
    UINT32  ram_addr = (void *)(RAM_DISK_ADDR & 0x0fffffff | 0x80000000);
    struct pvr_register_info    pvr_reg_info;

    if (sys_data_get_ram_tms_en())
    {
        ramdisk_create((UINT32) ram_addr, ram_len);

        MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
        strncpy(pvr_reg_info.mount_name, "/mnt/rda1", 15);
        pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
        pvr_reg_info.sync = 1;
        pvr_reg_info.init_list = 0;
        pvr_reg_info.check_speed = 0;
        pvr_register((UINT32) & pvr_reg_info, 0);
    }
}
#endif

static __MAYBE_UNUSED__ INT32 sto_erase_sector(struct sto_device *sto_dev, UINT32 start_addr, UINT32 len)
{
    UINT32 param;
    UINT32 _param[2];
    UINT32 cmd;
    UINT32 start_sect;
    UINT32 nb_sectors;
    INT32 ret;
    UINT32 i = 0;
    struct
    {
        UINT32 flash_size;
        UINT32 total_sectors;
        UINT32 sector_size;
    } flash_info;

    if (sto_dev == NULL)
    {
        return -1;
    }

    ret = sto_io_control(sto_dev, STO_DRIVER_GET_FLASH_INFO, (UINT32)(&flash_info));
    if (ret != SUCCESS)
    {
        return -1;
    }

    if (start_addr % flash_info.sector_size || len % flash_info.sector_size)
    {
        return -1;
    }

    start_sect = start_addr / flash_info.sector_size;
    nb_sectors = len / flash_info.sector_size;

    if ((start_sect + nb_sectors) >= flash_info.total_sectors)
    {
        return -1;
    }

    for (i=0; i<nb_sectors; i++)
    {
        if (sto_dev->totol_size <= 0x400000)
        {
            param = (start_sect + i) * flash_info.sector_size;
            param = (param << 10) + (flash_info.sector_size >> 10);
            cmd = STO_DRIVER_SECTOR_ERASE;
        }
        else
        {
            _param[0] = (start_sect + i) * flash_info.sector_size;
            _param[1] =  (flash_info.sector_size >> 10);
            param = (UINT32)_param;
            cmd = STO_DRIVER_SECTOR_ERASE_EXT;

        }

        if(sto_io_control(sto_dev, cmd, param) != SUCCESS)
        {
            return -1;
        }
    }

    /*begin to erase*/

    return 0;
}

#ifdef DB_SUPPORT_HMAC
#ifndef _BUILD_OTA_E_
static UINT8 headload_def = 0;
static UINT8 headhmac_fail = 0;

BOOL loaddef_tips(void)
{
    return headload_def;
}

static __MAYBE_UNUSED__ void load_default_db(void)
{
    UINT8   db_ret = 0;
    UINT32  default_db_addr = 0;
    UINT32  default_db_len = 0;
    UINT32  chunk_id = 0;

    chunk_id = 0x03FC0100;
    db_ret = api_get_chunk_add_len(chunk_id, &default_db_addr, &default_db_len);   
    if(db_ret)
    { 
        set_default_value(DEFAULT_ALL_PROG, default_db_addr + 16);
        sys_data_check_channel_groups();
        sys_data_set_cur_group_index(0);
        recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
        sys_data_set_cur_group_channel(0);
    }
}
static INT32 db_hmac_hander(UINT32 msg_type, UINT32 msg_param)
{
    SYSTEM_DATA* p_sys_data=sys_data_get();
    UINT8 zero_hmac[HMAC_OUT_LENGTH];
    UINT32 node_id = 0;
    UINT32 __MAYBE_UNUSED__ back_saved = 0;
    sys_state_t system_state = SYS_STATE_NORMAL;

    MEMSET(zero_hmac,0,HMAC_OUT_LENGTH);    
    system_state = api_get_system_state();
    
    if(MSG_HMAC_INIT == msg_type)
    {  
        if( (0 == p_sys_data->hmac_valid) 
        && (0 == MEMCMP(zero_hmac,p_sys_data->hs_hmac,HMAC_OUT_LENGTH)))
        { 
            return -1 ;
        }
        //MEMCPY(p_sys_data->hs_hmac,msg_param,HMAC_OUT_LENGTH);
        //p_sys_data->hmac_valid = 0x88;
        //sys_data_save(0);
    }
    else if(MSG_HMAC_SAVENEW == msg_type)
    {
        MEMCPY(p_sys_data->hs_hmac,(UINT8*)msg_param,HMAC_OUT_LENGTH);
        p_sys_data->hmac_valid = 0x88;
        sys_data_save(0);
    }
    else if(MSG_HMAC_GETSAVE == msg_type)
    {
        MEMCPY((UINT8*)msg_param,p_sys_data->hs_hmac,HMAC_OUT_LENGTH);        
    }
    else if( MSG_HMAC_RESET == msg_type )
    {
        p_sys_data->hmac_valid = 0;
        MEMSET(p_sys_data->hs_hmac,0,HMAC_OUT_LENGTH);         
    }
    else if( MSG_HMAC_NODE_FAILED == msg_type)
    {     
        //ota upgrade use db to backup otaloader.
        if(SYS_STATE_OTA_UPG != system_state) 
        {
            node_id = msg_param;
            del_node_by_id_wom(node_id);
        }
    }
    else if( MSG_HMAC_HEAD_FAILED == msg_type)
    {   
        //load_default_db();
        headhmac_fail = 1;
    }

    return 0;
}
#endif //_BUILD_OTA_E_
#endif

#ifdef FLASH_SOFTWARE_PROTECT 
UINT32 align_protect_len(UINT32 offset)
{
    UINT32 protect_len = offset; 

    if(protect_len > 0x400000 && protect_len < 0x800000)
    {
        protect_len = 0x400000;
    }
    else if(protect_len < 0x400000 && protect_len > 0x200000)
    {
        protect_len = 0x200000;
    }
    else if(protect_len < 0x200000)
    {
        protect_len = 0x100000;
    }

    return protect_len;
}

RET_CODE ap_set_flash_lock_len(INT32 len)
{//only support set 0/1/2/4M
    RET_CODE ret = RET_FAILURE;
    INT32 cur_lock_start = 0;
    INT32 cur_lock_len = 0;

    if((len!=DEFAULT_PROTECT_ADDR) && (len!=KEY_AREA_END_ADDR) && (len!=OTA_LOADER_END_ADDR) && (len!=0))
        return ret;

    if(SUCCESS !=sto_get_lock_range(g_sto_dev, &cur_lock_start, &cur_lock_len))
    {							
		libc_printf("%d %s sto_get_lock_range Fail!\n", __LINE__, __FUNCTION__);
        return ret;				
	}    
    libc_printf("cur lock range [0x%x ~ 0x%x]\n", cur_lock_start, cur_lock_len);
    
    if(len == cur_lock_len)
        libc_printf("0x%x aleady lock\n", len);
    else if(len > cur_lock_len)
    {
		if(len != sto_lock(g_sto_dev,0, len))
		{								
			libc_printf("%d %s sto_lock 0x%x Fail!\n", __LINE__, __FUNCTION__, len);
			return ret;				   
		}
        libc_printf("%s lock [0 - %x] OK!\n",  __FUNCTION__, len);
	}
    else//len < cur_lock_len
    {
        if(SUCCESS != sto_unlock(g_sto_dev, 0, cur_lock_len))
        {								
		    libc_printf("%d %s sto_unlock Fail2!\n", __LINE__, __FUNCTION__);
		    return ret;				   
		}
       
        if(len && (len != sto_lock(g_sto_dev,0, len)))
		{								
			libc_printf("%d %s sto_lock 0x%x Fail2!\n", __LINE__, __FUNCTION__, len);
			return ret;				   
		}
        libc_printf("%s lock [0 - %x] OK2!\n",  __FUNCTION__, len);
    }
    
    return RET_SUCCESS;
}
#endif

#ifndef _BUILD_OTA_E_
static RET_CODE ap_stb_db_init(UINT32 *db_addr, UINT32 *db_len)
{
    INT32   i = 0;
    UINT32  chunk_id = 0;
    UINT8   db_ret = 0;
    UINT32  flash_check[MAX_FLASH_CHECK_CNT] = { 0 };
    UINT32  default_db_addr = 0;
    UINT32  default_db_len = 0;
    struct sto_device   *sto_dev = NULL;
#ifdef _NV_PROJECT_SUPPORT_
    TBlockParam     xBlkParam;
#endif
#if 0//def FLASH_SOFTWARE_PROTECT 
    UINT32 protect_len = 0;
    UINT32 is_lock = 0;
    UINT32 prot_end_chunk_id = 0;
    UINT32 prot_end_chunk_addr = 0;
    UINT32 prot_end_chunk_len = 0;
#endif

#ifdef _NV_PROJECT_SUPPORT_
    BlkParamGet(LG_USER_DATABASE, &xBlkParam);
    *db_addr = xBlkParam.xFlashOffset;
    *db_len = xBlkParam.xSize;
//    sto_erase_sector(
//        (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),
//        *db_addr,
//        *db_len);
#else
    //user db chunk
    chunk_id = 0x04FB0100;
    if (!api_get_chunk_add_len(chunk_id, db_addr, db_len)) //db_len is pointer
    {
        PRINTF("api_get_chunk_add_len() failed!\n");
    }

    chunk_id = 0x03FC0100;
    db_ret = api_get_chunk_add_len(chunk_id, &default_db_addr, &default_db_len);
    if (0 ==  db_ret)
    {
        return RET_FAILURE;
    }
#endif

#ifdef BACKUP_TEMP_INFO
    *db_len = *db_len - (64 * 1024) * 2;      //
#else
    *db_len = *db_len - (64 * 1024);          //last sector for temp info
#endif

    sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (sto_dev != NULL)
    {
        sto_get_data(sto_dev, (UINT8 *)flash_check, *db_addr, 8 * sizeof(UINT32));
        for (i = 0; i < 8; i++)
        {
            if (flash_check[i] != 0xffffffff)
            {
                break;
            }
        }
    }

#ifndef _BUILD_OTA_E_
#if 0//def FLASH_SOFTWARE_PROTECT 
#ifdef _CAS9_VSC_ENABLE_
    prot_end_chunk_id = 0x21DE0100;
#else
    prot_end_chunk_id = 0x04FB0100;
#endif
    if(api_get_chunk_add_len(prot_end_chunk_id, &prot_end_chunk_addr, &prot_end_chunk_len))
    {
        protect_len = align_protect_len(prot_end_chunk_addr);
    }
    else
    {
        protect_len = 0x100000;
    }    

    sto_is_lock(sto_dev, 0, protect_len, &is_lock);
    if(0 == is_lock)
    {
        libc_printf("%s lock [0 - %x]!\n", __FUNCTION__,protect_len);
        sto_lock(sto_dev,0, protect_len);
    }
#endif    
#endif

    db_lcn_init();
    
    #ifdef DB_SUPPORT_HMAC
    register_db_callback(db_hmac_hander);
    #endif
    
    if (init_db(*db_addr, *db_len, *db_addr + *db_len, 0x10000) != SUCCESS)
    {
        osal_task_sleep(50);
        if (init_db(*db_addr, *db_len, *db_addr + *db_len, 0x10000) != SUCCESS)
        {
            osal_task_sleep(50);
            set_default_value(DEFAULT_ALL_PROG, (UINT32)(default_db_addr + 16));
            #ifdef DB_SUPPORT_HMAC
            if(headhmac_fail)
            {
                headload_def = TRUE;
            }
            #endif
        }
    }

    if (MAX_FLASH_CHECK_CNT == i)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

static void recover_time_setting_from_standby(void)
{
    INT32   ret = -1;
    UINT32  wakeup_time = 0;
    UINT32  sys_time = 0;
    INT32   hour_off = 0;
    INT32   min_off = 0;
    INT32   sec_off = 0;
    date_time   dt;
    SYSTEM_DATA *sys_data = NULL;

    MEMSET(&dt,0,sizeof(dt));
    sys_data = sys_data_get();
    ret = sys_reboot_get_timer(&wakeup_time, &sys_time);
    if ((SUCCESS == ret) && ((sys_time >> 22) & 0xF) /*month is ok*/
    && ((sys_time >> 17) & 0x1F) /*day is ok*/
    )
    {
        dt.year = ((sys_data->sys_dt.year / 100) * 100) + (sys_time >> 26);
        dt.month = (sys_time >> 22) & 0xF;
        dt.day = (sys_time >> 17) & 0x1F;
        dt.hour = (sys_time >> 12) & 0x1F;
        dt.min = (sys_time >> 6) & 0x3F;
        dt.sec = sys_time & 0x3F;
        get_stc_offset(&hour_off, &min_off, &sec_off);
        convert_time_by_offset(&dt, &dt, -hour_off, -min_off);
        set_stc_time(&dt);
        init_utc(&dt);
    }
}

static void startup_display_show(void)
{
    UINT8       cur_mode = 0;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
    if (!sys_data->bstandmode)
    {
        key_pan_display("strt", 4);
    }

    if (sys_data->b_mute_sate)
    {
        cur_mute_state = MUTE_STATE;    //fix bug46738  wakeup standby has audio but show mute
        set_mute_on_off(FALSE);
    }

    cur_mode = sys_data_get_cur_chan_mode();
    vdec_stop(g_decv_dev, 0, 0);
    if (!sys_data->bstandmode)
    {
        if (RADIO_CHAN == cur_mode)
        {
            api_show_radio_logo();
        }
    }
    else
    {
        key_pan_display("----", 4);
    }
}
#endif
#ifndef _BUILD_OTA_E_
static void startup_time(void)
{
    UINT32  cntry_code = 0xFFFFFF;
    UINT8   *cntry = NULL;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
    cntry = win_time_get_country_code(sys_data->country);
    if (cntry != NULL)
    {
        cntry_code = (cntry[0] << 16) | (cntry[1] << 8) | cntry[2];
    }

    tdt_set_country_code(cntry_code);
    if (0 == sys_data->gmt_select)
    {
        enable_time_parse();
        enable_time_offset_parse();
    }
    else if (1 == sys_data->gmt_select)
    {
        enable_time_parse();
        disable_time_offset_parse();
    }
    else if (2 == sys_data->gmt_select)
    {
        disable_time_parse();
        disable_time_offset_parse();
    }

    start_tdt();

    init_clock(time_refresh_callback);

}

#ifdef CI_SUPPORT
static void ap_init_cfg_ci_mode(void)
{
    //Default CI Case
    if (SINGLE_TUNER == g_tuner_num)
    {
        sys_data_set_ci_mode(CI_SERIAL_MODE);
        api_ci_set_mode(CI_SERIAL_MODE);
    }
    else
    {
        if (SINGLE_CI_SLOT == g_ci_num)
        {
            sys_data_set_ci_mode(CI_SERIAL_MODE);
            api_ci_set_mode(CI_SERIAL_MODE);
        }
        else if (DUAL_CI_SLOT == g_ci_num)
        {
            sys_data_set_ci_mode(CI_PARALLEL_MODE);
            api_ci_set_mode(CI_PARALLEL_MODE);
        }
    }
}
#endif
#endif

#ifdef SAT2IP_SUPPORT
static void ap_init_config_sat2ip(void)
{
    IP_LOC_CFG  ip_cfg2;

#ifdef SAT2IP_SERVER_SUPPORT
    sat2ip_control_init();
#endif

    get_local_ip_cfg(&ip_cfg2);
    if (ip_cfg2.ip_addr != 0)
    {
        ap_upnp_init();
    }
#if defined(SAT2IP_SERVER_SUPPORT) 
    if (http_mg_server_is_started()==FALSE)
    {
        http_mg_server_start();
    }
#endif

#ifdef SAT2IP_SERVER_SUPPORT
    ap_sat2ip_start_server();
    if (sys_data_get_satip_onoff_config())
    {
        rtsp_server_resume();
        http_server_start();
        ap_upnp_server_enable();
    }
    else
    {
        rtsp_server_pause();
    }
    #ifdef PVR2IP_SERVER_SUPPORT
    if (sys_data_get_pvrip_onoff_config())
    {
        media_provider_pvr_init();
    }
    #endif
#endif //SAT2IP_SERVER_SUPPORT
#ifdef SAT2IP_CLIENT_SUPPORT
    ap_upnp_client_enable();    // default enable
#endif
}
#endif //SAT2IP_SUPPORT

#ifdef NETWORK_SUPPORT

#ifdef GPIO_CONTROL_NET_LED
static void network_led_gpio_ioctl(INT32 *params)
{
     struct net_device *ndev = NULL;
    
    if(NULL == params)
    {
        return;
    }
   
    ndev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    if(NULL == ndev)
        libc_printf("net_device: Can not find net device!\n");
    else
        net_io_ctrl(ndev, MAC_LINKLED_GPIO_SET, (UINT32)params);

    return;
    
}

static void network_led_gpio_config(void)
{
    //params[0]: gpio for Network active LED control
    //params[1]: gpio for Network speed LED control
    //params[2]: the value of the LED GPIO,0=light, 1=not light
    INT32 params[3] = {0}; 
    if(SYS_MAIN_BOARD == BOARD_DB_M3823_02V01 || SYS_MAIN_BOARD == BOARD_DB_M3823_01V01)
    {
        params[0] = 15;
        params[1] = 2;
        params[2] = 0;
        network_led_gpio_ioctl(params);
    }
    else if(SYS_MAIN_BOARD == BOARD_DB_M3527_01V01 ||SYS_MAIN_BOARD == BOARD_DB_M3727_01V01)
    {
        params[0] = 0;
        params[1] = 1;
        params[2] = 0;
        network_led_gpio_ioctl(params); 
    } 
    return;
}
#endif


static void ap_init_config_network(void)
{
	struct libnet_config netcfg;
	netcfg.cb = network_callback;
	netcfg.lwip_cfg.mem.enable = TRUE;
	netcfg.lwip_cfg.mem.start = (UINT8 *)(__MM_LWIP_MEM_ADDR & MEMORY_CACHE_CONVERT);
	netcfg.lwip_cfg.mem.length = __MM_LWIP_MEM_LEN;
	netcfg.lwip_cfg.memp.enable = TRUE;
	netcfg.lwip_cfg.memp.start = (UINT8 *)(__MM_LWIP_MEMP_ADDR & MEMORY_CACHE_CONVERT);
	netcfg.lwip_cfg.memp.length = __MM_LWIP_MEMP_LEN;
	libnet_init(&netcfg);

#ifdef GPIO_CONTROL_NET_LED
        network_led_gpio_config();
#endif

#ifdef USB_ETHERNET_SUPPORT /* Barry : support USB to Ethernet  */
    mcs_usb_init();
    usbnet_init();
#endif

#ifdef WIFI_SUPPORT
    wifi_module_init(ap_wifi_display_callback);
#else
#if ((!(defined _S3281_||defined _M3503_ || defined _M3821_ || defined _S3811_ ||defined(_M3505_)))||(!defined _BUILD_OTA_E_))//add by comer
    wi_fi_device_enable(FALSE);
#endif
#endif

}

#if (_ALI_PLAY_VERSION_ >= 2)
void init_aliplay_service()
{
	aliplay_service_start();
}
#else
void init_aliplay_service()
{
	libc_printf("ali play 1.0 $$$\n");
}
#endif

static void ap_init_network_app(void)
{
#if (_ALI_PLAY_VERSION_ >= 2)
    init_aliplay_service();
#endif

#if defined(DLNA_SUPPORT)
    win_dlna_init();
#endif

#ifdef SAT2IP_SUPPORT
    ap_init_config_sat2ip();
#endif
}

#endif

#ifdef CI_PLUS_SUPPORT
static void ap_init_ciplus_cert(void)
{
#ifdef CI_WRITE_AES_DES_DATA
    api_ci_plus_write_aes_data();
    osal_task_sleep(100);
#endif
#ifdef CI_PLUS_NEW_CC_LIB
    {
        UINT8   i;
        for (i = 0; i < CONFIG_SLOT_NUM; i++)
        {
            ci_cc_attach(i, ci_cc_dev_init);
        }
    }
#endif
    read_ciplus_cert_data();
    read_ciplus_akh_etc();
}
#endif
#ifdef USB_MP_SUPPORT
static void ap_init_usb_mp(void)
{
#ifndef USB_INTERNAL_DISK_PARSE
#ifdef DISK_MANAGER_SUPPORT
    dm_init();
#endif
    fs_init(fs_apcallback);     // fs_init should be after dm_init, or dm will be clear.

#ifdef RAM_TMS_TEST
    init_ram_tms_test();
#endif
#else
    if (bdev_init() != SUCCESS)
    {
        libc_printf("bdev_init() failed\n");
    }

    if (bdev_attach(mp_apcallback) != SUCCESS)
    {
        libc_printf("bdev_attach() failed\n");
    }
#endif
#ifdef USB_LOGO_TEST
    usb_test_reg_ui_call_back(ui_usb_test_show_string, ui_usb_test_show_tpl);
    usb_test_reg_usbd_call_back(usb_test_bus_notify_ex);
    usb_test_init();
#endif
#ifndef DISABLE_USB
//	if(ALI_C3505 != sys_ic_get_chip_id())
	{
		usb_init();	
	}
    
#endif
}
#endif

#ifndef _BUILD_OTA_E_
static void ap_init_cec(void)
{
    SYSTEM_DATA *temp_sys_data = NULL;

    temp_sys_data = sys_data_get();
	if(NULL == temp_sys_data)
	{
		;
	}
#if (defined CEC_SUPPORT || defined HDMI_CERTIFICATION_UI)
    if ((temp_sys_data->hdmi_cec < 0) || (temp_sys_data->hdmi_cec > 1))
    {
        temp_sys_data->hdmi_cec = 1;
    }
#endif

#ifdef CEC_SUPPORT
    cec_link_module_init();
    if (temp_sys_data->hdmi_cec)
    {
        api_cec_set_func_enable(TRUE);
    }
    else
    {
        api_cec_set_func_enable(FALSE);
    }
#endif
}


static void check_reset_setting(BOOL bset_default)
{
    UINT8   db_ret = 0;
    UINT32  default_db_addr = 0;
    UINT32  default_db_len = 0;
    UINT32  chunk_id = 0;
	if(0 == db_ret)
	{
		;
	}
    chunk_id = 0x03FC0100;
    db_ret = api_get_chunk_add_len(chunk_id, &default_db_addr, &default_db_len);
    if (sys_data_get_factory_reset())
    {
        sys_data_set_factory_reset(FALSE);
        if ((0 == get_sat_num(VIEW_ALL)))
        {
            //init_default_value();
            set_default_value(DEFAULT_ALL_PROG, default_db_addr + 16);
            sys_data_check_channel_groups();
            sys_data_set_cur_group_index(0);
            recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
            sys_data_set_cur_group_channel(0);
        }
    }

#ifdef SAT2IP_CLIENT_SUPPORT
    // For Delete extra satellite if exception power down when satip_client...
    win_satip_delete_old_sat(0, 0, 0, 0);
#endif
    if ((0 == get_sat_num(VIEW_ALL)))
    {
        osal_task_sleep(50);
        if ((0 == get_sat_num(VIEW_ALL)))
        {
            osal_task_sleep(50);
            if (TRUE == bset_default)
            {
                set_default_value(DEFAULT_ALL_PROG, default_db_addr + 16);
            }

#ifdef DVBC_SUPPORT
            if (0 != db_ret)
            {
                set_default_value(DEFAULT_ALL_PROG, default_db_addr + 16);
            }

            //because dvbc cannot del sat,so sat is default, sat num don't equal 0.
            //when sat num equal 0 , the db is maybe destroyed,eg.ota upgrade.
            // so we load default,also we can add a sat to resolve the problem.
#endif
        }
    }
}
#endif

#ifdef BG_TIMER_RECORDING
static UINT8 check_schedule_recording(void)
{
    int i = 0;
    SYSTEM_DATA *sys_data = NULL;
    TIMER_SET_T *timer_set = NULL;
    TIMER_SET_CONTENT *timer = NULL;
    date_time dt;
    date_time start_dt;
    UINT8 ret = FALSE;
	
    get_local_time(&dt);
    sys_data = sys_data_get();
    timer_set = &sys_data->timer_set;

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        timer = &timer_set->timer_content[i];
        //   system_state = api_get_system_state();
    
        start_dt.year   = timer->wakeup_year;
        start_dt.month  = timer->wakeup_month;
        start_dt.day    = timer->wakeup_day;
        start_dt.hour   = timer->wakeup_time/60;
        start_dt.min    = timer->wakeup_time%60;
        start_dt.sec    = 0; 
        // libc_printf("Year :%d %d", start_dt.year,dt.year);
        //libc_printf("Timer %d, min :%d wmin :%d\n",i,start_dt.min,dt.min);
	 
        if( (TIMER_MODE_OFF!=timer->timer_mode) && (0 == api_compare_day_time(&start_dt,&dt)))
        {
            if (TIMER_STATE_RUNING == timer->wakeup_state)
            {
                //   libc_printf("Timer Running : %d\n",i);		
            }
            else /* Wakeup a timer */
            {					
                /* If it's channel timer and specified channel not exit, turn off the timer */
                if(TIMER_SERVICE_DVR_RECORD == timer->timer_service )
                {
                    //libc_printf("Record Timer found:%d, timer_mode(%d)\n",i,timer->timer_mode); //TIMER_MODE_OFF
                    ret = TRUE;
                    break;
                }
            }
        }  
    }

    return ret;
}

static void disable_av_display(void)
{ 
    UINT8 scart_out=SCART_RGB;
	
    #ifdef SD_UI     //case of M3281
        if(NULL!=g_vpo_dev)
        {
             //URI_PRINTF("1.Un Reg YUV\n");
             vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_YUV_TYPE);
             osal_task_sleep(50);

             //URI_PRINTF("2.Un Reg RGB\n");
             vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_RGB_TYPE);
             osal_task_sleep(50);

            //URI_PRINTF("3.Un Reg CVBS\n");
            vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_CVBS_TYPE);
            osal_task_sleep(50);
        }
        if(NULL!=g_sd_vpo_dev)
        {
            //URI_PRINTF("4.Un Reg CVBS (SD OSD)\n");
            vpo_ioctl(g_sd_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_CVBS_TYPE);
            osal_task_sleep(50);
        }
    #else //case of M3515
        scart_out = sys_data_get_scart_out();
        #ifdef TVE_VDEC_PLUG_DETECT
            if(NULL!=g_vpo_dev)
            {
                vpo_ioctl(g_vpo_dev, VPO_IO_ENABLE_VDAC_PLUG_DETECT, FALSE);
                osal_task_sleep(50);
            }
        #endif

        if (SCART_YUV==scart_out)
        {
            if(NULL!=g_vpo_dev)
            {
                vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
                osal_task_sleep(50);
            }

            #if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
            if(NULL!=g_sd_vpo_dev)
            {
                vpo_ioctl(g_sd_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
                osal_task_sleep(50);
            }
            #endif
        }
        else    //ex RGB
        {
            if(NULL!=g_vpo_dev) //needed after testing
            {
                vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
                osal_task_sleep(50);
            }

            #if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
            if(NULL!=g_sd_vpo_dev)
            {
                vpo_ioctl(g_sd_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
                osal_task_sleep(50);
            }
            #endif
        }
    #endif
	
    //disable hdmi
    api_hdmi_switch(0);
	
    snd_io_control(g_snd_dev, SND_I2S_OUT, 0);
}
#endif


/*
return value:
    0  - Boot at normal state(need to play channel or show menu when no channels)
    -1 - Boot at standby mode
    -2 - Boot at time wakeup */
#ifndef _BUILD_OTA_E_
static INT32 system_init_process(void)
{
    SYSTEM_DATA             *sys_data = NULL;
    INT32                   i = 0;
    INT32                   ret = 0;
    RET_CODE                db_ret = RET_FAILURE;
    UINT32                  db_addr = 0;
    UINT32                  db_len = 0;
#ifndef HDTV_SUPPORT
    UINT8                   tv_out = 0;
#endif
    BOOL                    bset_default = FALSE;
    UINT8                   gp_index = 0;
    UINT8                   gp_type = 0;
    UINT8                   gp_pos = 0;
    UINT16                  gp_channel = 0;

    dm_set_onoff(DM_OFF);   /* Disable moniter */
    key_pan_set_key_table(board_get_ir_key_map());

#ifndef _BUILD_OTA_E_
#if (SYS_PROJECT_SM == PROJECT_SM_CI)
    g_ci_num = CONFIG_SLOT_NUM;
#else
    g_ci_num = 0;
#endif
#endif

#ifdef DVR_PVR_SUPPORT
    pvr_module_init();
#ifdef CC_USE_TSG_PLAYER
    pvr_set_with_tsg_using_status(1);
#else
    pvr_set_with_tsg_using_status(0);
#endif
#endif

#ifdef USB_MP_SUPPORT
#ifndef DISK_MANAGER_SUPPORT
    storage_dev_ctrl_init();
#endif
#endif

    set_lnbshort_detect(1);
    set12vshort_detect(1);

    db_ret = ap_stb_db_init(&db_addr, &db_len);
#ifdef SUPPORT_FRANCE_HD
    ap_france_hd_init(db_addr, db_len);
#else
#ifdef TEMP_INFO_HEALTH
#ifdef BACKUP_TEMP_INFO
    init_swap_info(db_addr + db_len + 64 * 1024, 64 * 1024);
    restore_swap();
#endif
    sys_data_register_check_function((PCHECK_CB)sys_data_check_av_set);
    sys_data_register_check_function((PCHECK_CB)sys_data_check_peg_field);
#endif
#endif

    ap_osd_int();
#ifndef HDTV_SUPPORT
    // set OSD in sys_data_load();
    tv_out = api_video_get_tvout();
    api_osd_set_tv_system(tv_out == TV_MODE_NTSC358 || tv_out == TV_MODE_PAL_M || tv_out == TV_MODE_NTSC443);
#endif

    /* Load temp data */
    sys_data_load();

    if (RET_FAILURE == db_ret)
    {
        bset_default = TRUE;
    }
    check_reset_setting(bset_default);

#ifdef DVBC_SUPPORT
    board_update_nim_valid(); //add on 2011-11-07 for combo S2+DVBC to set the DVBT/ISDBT/DVBC nim valid
#endif

    sys_data = sys_data_get();
    if(NULL != sys_data)
    {
        gp_index = sys_data_get_cur_group_index();    
        sys_data_get_cur_mode_group_infor(gp_index,&gp_type, &gp_pos, &gp_channel);
        if(ALL_SATE_GROUP_TYPE == gp_type)
        {
            recreate_prog_view(VIEW_ALL | sys_data->cur_chan_mode[0], 0);
        }
        else
        { 
            sys_data_change_group(gp_index); 
        }
    }
    else
    {
        recreate_prog_view(VIEW_ALL, 0);
    }
    

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
#if (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
    init_ota_dvbt_param();
#endif
#ifdef DVBC_SUPPORT
    init_ota_dvbc_param();
#endif
#endif

    /* If support low power*/
#if (REAL_STANDBY == SYS_FUNC_ON)
    recover_time_setting_from_standby();
#endif

#ifdef BG_TIMER_RECORDING
    //Add schedule recording check and DA
    if(TRUE == check_schedule_recording())
    {
        //libc_printf("[%s]-Timer from standby\n",__FUNCTION__); 
        g_silent_schedule_recording = TRUE;
        disable_av_display();
    }
    else
    {
        //libc_printf("No Timer \n");
        g_silent_schedule_recording = FALSE;
    }
#endif  
#ifdef CVBS_HDMI_SWITCH
	//SYSTEM_DATA *sys_data = NULL;
	sys_data = sys_data_get();
	if (sys_data->tv_output == CVBS_ON)
	{
		api_set_hdmi_phy_clk_onoff(0);
	}
	else if (sys_data->tv_output == HDMI_ON)
	{
		if (NULL!=g_vpo_dev)
		{
			vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
			osal_task_sleep(50);
		}
		//api_set_hdmi_phy_clk_onoff(1);
	}	
#endif

#ifdef HDTV_SUPPORT
#ifndef _BOOT_LOGO_ENABLE_
    api_show_bootup_logo();
#endif
#if defined(_FAST_BOOT_UP) || defined(BOOT_SPEED_UP)
    osal_task_sleep(100); // chage for fast boot up
#else
    osal_task_sleep(200);
#endif
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT
#if (SYS_SDRAM_SIZE >= 128)
    sys_data_set_ad_mode(sys_data_get_ad_default_mode());   // set ad mode when system power on
    api_audio_set_ad_volume_offset((INT32) sys_data_get_ad_volume_offset());
#endif
#endif

#ifdef CC_USE_TSG_PLAYER
    pvr_r_set_scramble_ext(!sys_data_get_scramble_record_mode());
#endif



    /* Panel & screen initital display */
    startup_display_show();

    /* Set all timer in not runing status */
    for (i = 0; i < MAX_TIMER_NUM; i++)
    {
        sys_data->timer_set.timer_content[i].wakeup_state = TIMER_STATE_READY;
    }

    /* Clear all message */
    #ifdef BOOT_SPEED_UP
    osal_task_sleep(200);
    #else
    osal_task_sleep(2000);
    #endif
    pan_buff_clear();
    ap_clear_all_message();

    vdec_stop(g_decv_dev, 0, 0);
    dm_set_onoff(DM_NIMCHECK_ON);
    ret = 0;

#ifndef _BUILD_OTA_E_
    /* init tdt module */
    startup_time();
    epg_init(SIE_EIT_WHOLE_TP, (UINT8 *)__MM_EPG_BUFFER_START, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
#endif

#ifdef MULTIFEED_SUPPORT
    multifeed_register(ap_multifeed_call_back);
#endif
    if (OSAL_INVALID_ID == epg_flag)
    {
        epg_flag = osal_flag_create(EPG_MSG_FLAG_MUTEX);
    }

    sim_open_monitor(0);

#ifdef _DTGSI_ENABLE_
    ap_dtg_set_sdt_callback();
#endif

#ifdef USB_MP_SUPPORT
    ap_init_usb_mp();
#endif

#ifndef _BUILD_OTA_E_
#ifdef CI_SUPPORT
    ap_init_cfg_ci_mode();
#endif
#endif

#ifdef NETWORK_SUPPORT
    ap_init_config_network();

    ap_init_network_app();
#endif

#ifdef CI_PLUS_SUPPORT
    ap_init_ciplus_cert();
#endif

#ifdef FSC_SUPPORT
    fsc_control_init();
#endif

    ap_init_cec();

#ifdef DVBC_SUPPORT
    get_default_bandparam(sys_data->country, (band_param *) &u_country_param);
#endif

    return ret;
}
#endif


#ifdef _BUILD_USB_LOADER_
static void ap_ctrl_init_usb_loader(void)
{
    BOOL ret=0;
#ifdef _BUILD_UPG_LOADER_
    UINT32 upg_flag = 0;
#endif

#ifdef USB_MP_SUPPORT
#ifndef USB_INTERNAL_DISK_PARSE
#ifdef DISK_MANAGER_SUPPORT
    dm_init();
#endif
    fs_init(fs_apcallback); // fs_init should be after dm_init, or dm will be clear.
#else
    if (bdev_init() != SUCCESS)
    {
        //libc_printf("bdev_init() failed\n");
    }

    if (bdev_attach(mp_apcallback) != SUCCESS)
    {
        //libc_printf("bdev_attach() failed\n");
    }
#endif
    LDR_PRINT("bf usb_init \n");
    usb_init();
#endif

#ifdef _BUILD_LOADER_COMBO_
#ifdef DISK_MANAGER_SUPPORT
    dm_init();
#endif
    fs_init(fs_apcallback); // fs_init should be after dm_init, or dm will be clear.
    LDR_PRINT("bf usb_init \n");
    usb_init();
#endif

#ifdef _BUILD_OUC_LOADER_
#if 1//new or change add by yuj
    LDR_PRINT("bf gacas_loader_db_init \n");
	gacas_loader_db_init();
	gacas_loader_db_deal_one_time_all();
    ret = gacas_loader_db_is_usb();
    LDR_PRINT("af gacas_loader_db_is_usb (ret=%d) \n", ret);
	if(ret == TRUE)
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) &g_win2_usbupg, TRUE);
	}
	else
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & win_otaupg_con, TRUE);
	}
	//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win2_usbupg, TRUE);
#else
    if (ouc_get_upgrade_mode() == OUC_TYPE_OTA )
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&win_otaupg_con, TRUE);
    }
    else
    {
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win2_usbupg, TRUE);
    }
#endif      
#else
#ifdef _BUILD_UPG_LOADER_
    /* get ota_cfg */
    upg_flag = *((volatile UINT32*)0xb8050000);
    LDR_PRINT("af gacas_loader_db_is_usb (ret=%d) \n", upg_flag);
    if(UPDATE_FLAG_OTA == upg_flag)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&win_otaupg_con, TRUE);
    }
    else
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_usbupg, TRUE);//new or change add by yuj
    }
#else
#ifdef _BUILD_LOADER_COMBO_
    ret = loader_check_run_parameter();
    LDR_PRINT("af loader_check_run_parameter (ret=%d) \n", ret);
	if (ret==1)
	{
		 ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win2_usbupg, TRUE);//new or change add by yuj
	}
	else
    {
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&win_otaupg_con, TRUE);
	}
#else
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win2_usbupg, TRUE);//new or change add by yuj
#endif
#endif
#endif
}
#endif

#ifdef USB3G_DONGLE_SUPPORT
void usb3g_dongle_event_callback(enum USB3G_DONGLE_EVENT event, struct usb3gdongle *dongle);
static void ap_ctrl_init_usb3g(void)
{
    usb3g_attach(usb3g_dongle_event_callback);

    //cn
    usb3g_reg_isp_info("46000", "*98*1#", "CMNET", NULL, NULL); //china mobile
    usb3g_reg_isp_info("46002", "*98*1#", "CMNET", NULL, NULL); //china mobile
    usb3g_reg_isp_info("46007", "*98*1#", "CMNET", NULL, NULL); //china mobile
    usb3g_reg_isp_info("46001", "*99#", "3GNET", NULL, NULL);   //china unicom
    usb3g_reg_isp_info("46003", "#777", NULL, "ctnet@mycdma.com", "vnet.mobi"); //china telecom

    //tw
    usb3g_reg_isp_info("46601", "*99#", "internet", "username", "password");    //远传电讯
    usb3g_reg_isp_info("46606", "*99#", "internet", "username", "password");    //TUNTEX
    usb3g_reg_isp_info("46668", "*99#", "internet", "username", "password");    //ACeS
    usb3g_reg_isp_info("46688", "*99#", "internet", "username", "password");    //和信电讯
    usb3g_reg_isp_info("46692", "*99#", "internet", "username", "password");    //中华电讯
    usb3g_reg_isp_info("46693", "*99#", "internet", "username", "password");    //东信电讯
    usb3g_reg_isp_info("46697", "*99#", "internet", "username", "password");    //台湾大哥大
    usb3g_reg_isp_info("46699", "*99#", "internet", "username", "password");    //泛亚电讯
}
#endif

#ifdef _BUILD_OTA_E_
static void ap_ctrl_init_ota(void)
{
#ifdef _BC_CA_ENABLE_
    INT16   bc_ret;
#endif

    ap_osd_int();
    osd_set_lang_environment(0);

	if(ALI_C3702 == sys_ic_get_chip_id())
	{
		sys_data_set_video_format(SYS_DIGITAL_FMT_YCBCR_444);
	}
   else
	{
		sys_data_set_video_format(SYS_DIGITAL_FMT_RGB);
	}

#ifdef _BUILD_USB_LOADER_
    LDR_PRINT("bf ap_ctrl_init_usb_loader \n");
    ap_ctrl_init_usb_loader();
#else
    LDR_PRINT("_BUILD_USB_LOADER_ not define \n");
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & win_otaupg_con, TRUE);
#endif

#ifdef _BC_CA_ENABLE_
    libc_printf("BC_Init\n");
    otp_init(NULL);
	#ifndef _BUILD_LOADER_COMBO_ //temp
    bc_ret = BC_Init();
    #endif
    libc_printf("BC_Init: bc_ret=%d\n", bc_ret);
#endif

#ifdef SD_PVR
    switch_tv_mode(PAL, FALSE);
#else
    //Seiya add,fix bug when ota update(Down load out file),HDMI no A/V output problem;
    switch_tv_mode(LINE_720_30, TRUE);
#endif
}
#endif

int key_store_write(UINT32 on_off)
{
#ifdef FLASH_SOFTWARE_PROTECT
    if(on_off)
    {
        ap_set_flash_lock_len(OTA_LOADER_END_ADDR);
    }
    else
    {
        ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
    }
#endif

    return 0;
}

#ifdef _C0700A_VSC_ENABLE_
#include <api/libcas/abv/cas_abv.h>
void ap_cas_callback(CAS_MSG_TYPE_T type, UINT32 param)
{
	UINT32 msg_code;
	msg_code = (UINT32)((param & 0xffffff) | (type << 24));
	ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS, msg_code, FALSE );
}
#endif

extern 	signed int gacas_init(void);
void ap_control_init(void)
{
#ifndef _BUILD_OTA_E_
    POBJECT_HEAD    first_menu = NULL;
    SYSTEM_DATA     *sys_data = NULL;
    vdec_cbfunc     pcallback = cb_vdec_output_mode_check;
    UINT32          prog_num = 0;
    INT32           ret = 0;
	if(0 == ret)
	{
		;
	}
#endif

#ifdef _BUILD_OTA_E_
    LDR_PRINT("bf ap_ctrl_init_ota \n");
    ap_ctrl_init_ota();
#else
    ap_auto_chchg_init();
    set_avc_output_mode_check_cb(pcallback);

    ret = system_init_process();   /* system initializaton */
#ifdef DVR_PVR_SUPPORT
    pvr_2recorder_skip_prog();
#endif
    sys_data = sys_data_get();
    prog_num = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode[0], 0);

    api_audio_set_mute(FALSE);
    if (0 == prog_num)
    {
#ifdef _INVW_JUICE
        inview_deferred_start = 1;
#endif

#ifdef SHOW_WELCOME_FIRST
#ifdef _BOOT_LOGO_ENABLE_
        api_show_bootup_logo();
        osal_task_sleep(200);
        vdec_stop(g_decv_dev, 0, 0);
#endif
        if (ALI_S3821 == sys_ic_get_chip_id() &&
            check_rs232_item_is_active())
        {
            first_menu = MAIN_MENU_HANDLE;      /* Main menu */
        }
        else
        {
            first_menu = ((POBJECT_HEAD) & g_win_welcom);
        }
#else
        //first_menu = MAIN_MENU_HANDLE;      /* Main menu */
        first_menu = ((POBJECT_HEAD) & g_win2_mainmenu);
        //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_START_SEARCH, 2, TRUE);
	//first_menu = NULL;
#endif

#if defined(HDCP_BY_URI) && defined(CAS9_V6) 
        //in menu or no prog, default disable HDCP
        if(FALSE==api_cnx_uri_get_hdcp_disable())
        {
            api_cnx_uri_set_hdcp_disbale(TRUE);
            api_set_hdmi_hdcp_onoff(FALSE);
            libc_printf("%s-HDCP off\n",__FUNCTION__);
        }
#elif defined(FORCE_HDCP_ENABLE)  
        if(FALSE == api_get_hdmi_hdcp_onoff())
        {
            api_set_hdmi_hdcp_onoff(TRUE);
        }
#endif
    }
    else
    {
#ifdef _INVW_JUICE
        inview_init();
        inview_start();
        osal_delay(100);
        sysdbg_test(0, 0);
#endif
        first_menu = CHANNEL_BAR_HANDLE;    /* Channel bar menu */
        show_and_playchannel = 1;
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        set_mmi_showed(CA_MMI_PRI_10_DEF);
#endif
    }

#ifdef SUPPORT_FRANCE_HD
    nit_monitor_register(nit_change_callback);
#endif

    if (first_menu != NULL)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) first_menu, TRUE);
    }

#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
    if (first_menu == CHANNEL_BAR_HANDLE)
    {
        int inview_code = 0,
        int ret_code = 0;
        ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
        inview_resume(inview_code);
        inview_handle_ir(inview_code);
    }
#endif
#endif

	if (((ALI_S3281 == sys_ic_get_chip_id()) \
		|| (ALI_S3503 == sys_ic_get_chip_id()) \
		|| (ALI_S3821 == sys_ic_get_chip_id()) \
		|| (ALI_C3505 == sys_ic_get_chip_id()) \
		|| (ALI_C3503D == sys_ic_get_chip_id()) \
		|| (ALI_C3711C == sys_ic_get_chip_id())) && 
			check_rs232_item_is_active())
	{
		/*if 3281,3503,3823 erom upgrade, jump to erom upgrade menu directly.*/
		first_menu = ((POBJECT_HEAD) & win_rs232upg_con); 
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) first_menu, TRUE);
	}

#ifdef SUPPORT_CAS9
#if (defined(CAS9_VSC) && defined(CAS9_SC_UPG))
    key_store_erase();
#endif
#if (defined(CAS9_VSC) && (!defined(CAS9_SC_UPG)))
    ret = api_mcas_vsc_init ();
#endif

#ifdef CAS9_VSC
   if(ALI_CAP210 <= sys_ic_get_chip_id())
   {
      otp_std_kdf_trigger(1, 0);
      otp_std_kdf_trigger(1, 1);
      otp_std_kdf_trigger(1, 3);
   }
#endif  

#ifdef CAS9_V6
    //api_mcas_enable_keystore_hmac(1);//new prj use 1 for HMAC
    //api_mcas_set_pre_cw_valid(1);
    //#ifdef NEW_TIMER_REC
        //libc_printf("Enable new timer rec via api_mcas_set_bg_timer_rec(TRUE) !!\n");
        //api_mcas_set_bg_timer_rec(TRUE);
    //#endif

    struct mcas_config_par ca_config;
    MEMSET(&ca_config, 0, sizeof(ca_config));
    ca_config.pre_cw_valid = 1;
    #ifdef NEW_TIMER_REC
    ca_config.bg_timer_rec = 1;
    #else
    ca_config.bg_timer_rec = 0;
    #endif
    ca_config.rec_emm_fp = 0;
    ca_config.mail_msg_sp_operate = 0;
    ca_config.keystore_hmac_enable = 1;//new prj should set 1 for hmac check, old prj set 0
    ca_config.apdu_cb = NULL;
    ca_config.sp_cb = NULL;
    
    api_mcas_config(&ca_config);
    //ttx_reg_callback(ca_notify_ttx_pid);
#if (defined(CAS9_VSC) && defined(CAS9_SC_UPG))       
    api_mcas_config_keystore_write_cb(key_store_write);
#endif
#if 1    //for cached pin syncronize with ca lib
    api_mcas_set_cached_pin(sys_data->cached_ca_pin,4);
#endif
#endif

    // Only M36G support VPO_IO_SEL_OSD_SHOW_TIME functiona
#if defined (CAS9_V6)
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SEL_OSD_SHOW_TIME, TRUE); // driver never set pisr_show_time->time_in_ms = 3;
#endif
    cas_task_init();
    api_mcas_register(ap_mcas_display);

    //init_finger_clock();
    init_msg_clock();
    mem_protect_check();
#if (defined(CAS9_VSC) && (!defined(CAS9_SC_UPG)))
    if (RET_SUCCESS == ret)
    {
        api_vsc_lib_start (MCAS_CARD_IN);
    }
    else
    {
        api_vsc_lib_start (MCAS_VSC_SYSTEM_ERROR);
    }
#endif
#endif

#ifdef _C0700A_VSC_ENABLE_
    UINT32 test_vsc_tick = osal_get_tick();
    libc_printf("before VSC init\n");
    ret = api_mcas_vsc_init ();
    libc_printf("VSC time %d\n", osal_get_tick()-test_vsc_tick);
    
    cas_monitor_init(ap_cas_callback);//add

#if 0//test
libc_printf("test\n");
    UINT8 cmd_test[10]={0x11};
    UINT8 resp_test[4096]={0};
    UINT16 test_len = 0;
    UINT16 sw =0;
    vsc_lib_dispatch_cmd (cmd_test, 10, resp_test, &test_len, &sw);
    libc_printf("end of vsc_lib_dispatch_cmd, 0x%x, 0x%x, 0x%x\n", resp_test[0], test_len, sw);
#endif
#else
    #if 0
    libc_printf("No VSC, test only\n");

    UINT32 vsc_lib_addr = __MM_VSC_CODE_START_ADDR + 0x200;
    UINT32 vsc_lib_len = 0x20000;
    vsc_lib_init(&vsc_lib_addr, &vsc_lib_len);
    #endif
#endif

#ifdef _NV_PROJECT_SUPPORT_
#ifndef _BUILD_UPG_LOADER_
    id_dsc_mutex = osal_mutex_create();
#ifdef _C0200A_CA_ENABLE_
#ifdef _C0200A_CA_DAL_TEST_
    nasc_dal_test();
#else
    nvcak_init_and_start();
#endif
#endif
#endif
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)

    // VMX KDF vender id
    //UINT64 vendor_id = 0x564d78463401f6df;// VMX vender id
    UINT64 vendor_id = 0x564d000000010328;// VMX SMI vender id
    otp_etsi_kdf_trigger(0, vendor_id);
    otp_etsi_kdf_trigger(1, vendor_id);
    otp_etsi_kdf_trigger(2, vendor_id);

    bc_cas_module_init();
    INT16   bc_ret;

    otp_init(NULL);
    bc_ret = BC_Init();
    BC_InitWindow(OSD_MAX_WIDTH, OSD_MAX_HEIGHT, "                       ");

#if defined(BC_PATCH_BC_IOCTL) || defined(SUPPORT_BC)
    BC_Ioctl(k_ConnectBc, ioctl_check_pin_function, NULL);
#endif

    bc_cas_task_init();
#endif

#ifdef SFU_TEST_SUPPORT
    //UINT16  channel;
    //P_NODE  p_node;
    if (bootup_checking_for_sfutesting() == TRUE)
    {
        api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);
    }
#endif

#ifdef SUPPORT_CAS7    //SUPPORT_CAS7 alone
    BOOL    swca_fp_task_init(void);
    api_mcas_register(ap_mcas_display);
    swca_fp_task_init();
#endif

#ifdef CAS7_PVR_SCRAMBLE
    api_ca_conax_setinvalidcw();            // for default CW
#endif

#ifdef GEN_CA_PVR_SUPPORT
    gen_ca_task_init();
    gen_ca_pvr_init();
#endif
#ifdef SUPPORT_GACAS
	gacas_init();
#endif

#if ((defined(CAS9_PVR_SUPPORT)) || (defined(CAS9_PVR_SCRAMBLED)))
    cas9_pvr_init();
#endif

#if defined(BC_PVR_SUPPORT)
    bc_pvr_init();
#endif

#if defined(C0200A_PVR_SUPPORT)
    c200a_pvr_init();
#endif

#if ((defined(CI_PLUS_PVR_SUPPORT)) && !defined(BC_STD_PVR_CRYPTO))
    ciplus_pvr_init();
#endif

#ifdef CAS7_PVR_SUPPORT
    if (sys_ic_get_chip_id() != ALI_S3811)  //3811 has no aes dev,just support csa.
    {
        pvr_crypto_init(PVR_CRYPTO_MODE_AES_ECB);
    }
#endif

#ifdef BC_STD_PVR_CRYPTO
#if defined(SUPPORT_BC_STD) && defined(BC_PVR_STD_SUPPORT)
    if (ALI_S3281 == sys_ic_get_chip_id())
    {
        bl_generate_store_key_for_pvr();
    }

    pvr_crypto_init2(PVR_CRYPTO_MODE_AES_ECB, KEY_0_3);         //use 4th Key (zero based)(i.e. 0x59~0x5C)
#else
    pvr_crypto_init(PVR_CRYPTO_MODE_AES_ECB);
#endif
#endif

#ifdef UPGRADE_HOST_MONITOR
    key_set_upgrade_check_flag(UPGRADE_CHECK_SLAVE | UPGRADE_CHECK_HOST);
#else
    key_set_upgrade_check_flag(UPGRADE_CHECK_SLAVE);
#endif

#ifdef IDE_SUPPORT
    storage_switch_sd_ide_proc(sys_data_get_sd_ide_statue(), TRUE);
#elif defined SDIO_SUPPORT
    storage_switch_sd_ide(TRUE, TRUE);
#endif
#endif

#ifdef USB3G_DONGLE_SUPPORT
    ap_ctrl_init_usb3g();
#endif

#ifdef HILINK_SUPPORT
    hilink_attach();
#endif

    api_set_system_state(SYS_STATE_NORMAL);
    set_tv_aspect_ratio_mode(0);

#ifdef DVR_PVR_SUPPORT
    api_pvr_set_back2menu_flag(FALSE);
    set_rec_hint_state(FALSE);
#endif

#ifdef DB_SUPPORT_HMAC
#ifndef _BUILD_OTA_E_
    if(loaddef_tips())
    {
        UINT8 back_saved = 0;
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("DB head is destoyed,have loaded default !", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
    }
#endif
#endif    
}

