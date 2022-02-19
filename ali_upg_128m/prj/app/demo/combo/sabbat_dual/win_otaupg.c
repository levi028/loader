#if 0//new or change add by yuj
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg.c
*
*    Description:   The realize of OTA upgrade
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
#include <api/libc/fast_crc.h>

#ifdef _M3503_
#include <api/libzip/gnuzip.h>
#endif

#include "control.h"
#include "ctrl_util.h"
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
#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
#include <hld/crypto/crypto.h>
//#include "conax_ap/nn.h"
#endif
#include <hld/dsc/dsc.h>
#include <api/librsa/flash_cipher.h>
#include <api/librsa/rsa_verify.h>
#include <boot/boot_common.h>

#ifdef AUTO_OTA
#include <hld/pan/pan_dev.h>
#endif

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#if defined(SUPPORT_BC)
//#include <api/libcas/bc/bc_types.h>
#include "bc_ap/bc_cas.h"
#include "bc_ap/bc_osm.h"
#endif

#ifdef _NV_PROJECT_SUPPORT_ 
#include <udi/nv/ca_upg.h>
#endif

#ifdef _BC_CA_NEW_
extern BOOL loader_set_run_parameter(BOOL set,BOOL type);
extern BOOL loader_check_run_parameter();
#endif
#include <api/libota/lib_ota.h>
#include "win_otaupg_private.h"

UINT32 ota_proc_ret=0;  /* PROC_SUCCESS / PROC_FAILURE / PROC_STOPPED */
UINT8  ota_user_stop=0;
OTA_STEP ota_proc_step=0;
static UINT8 *p_otaosd_buff  = NULL;
UINT16 ci_flag=0;
//add by colin: used for judging wether the third block id
//(ciplus chunk id) of the flash and the memory is identical
struct dl_info ota_dl_info;
OTA_INFO m_ota_info;
/*******************************************************************************
* Objects declaration
*******************************************************************************/
#define HEX_ZERO   0x30
#define HEX_NIME   0x39
#define HEX_A_LOWERCASE  97
#define HEX_F_LOWERCASE   102
#define HEX_A_UPPERCASE  65
#define HEX_F_UPPERCASE   70

#define OTA_NEWFLOW
//#define SIG_LEN     (256)


static VACTION otaupg_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT otaupg_btn_callback(POBJECT_HEAD pobj,\
               VEVENT event, UINT32 param1, UINT32 param2);

static VACTION otaupg_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT otaupg_callback(POBJECT_HEAD pobj,\
               VEVENT event, UINT32 param1, UINT32 param2);
#ifdef _BUILD_LOADER_COMBO_
extern UINT8 loader_check_osm_parameter();
#endif

#ifdef FLASH_SOFTWARE_PROTECT 
void flash_protect_onoff(UINT8 mode);
extern UINT32 align_protect_len(UINT32 offset);
#endif


LDEF_MTXT(g_win_otaupg,otaupg_mtxt_msg,&otaupg_txt_btn,\
    MTXT_MSG_L, MTXT_MSG_T, MTXT_MSG_W, \
    MTXT_MSG_H,MTXT_MSG_SH_IDX,1,otamsg_mtxt_content)

#ifdef _NV_PROJECT_SUPPORT_ 
LDEF_TXT_BTN(g_win_otaupg,otaupg_txt_btn,\
    &otaupg_mtxt_swinfo,1,1,1,\
    BTN_L, BTN_T, BTN_W, BTN_H,\
    RS_DISPLAY_PLEASE_WAIT)
#else
LDEF_TXT_BTN(g_win_otaupg,otaupg_txt_btn,\
    &otaupg_mtxt_swinfo,1,1,1,\
    BTN_L, BTN_T, BTN_W, BTN_H,\
    RS_DISEQC12_MOVEMENT_STOP)
#endif

LDEF_MTXT(g_win_otaupg,otaupg_mtxt_swinfo,\
    &otaupg_progress_bar,\
    MTXT_SWINFO_L, MTXT_SWINFO_T, \
    MTXT_SWINFO_W, MTXT_SWINFO_H,MTXT_SWINFO_SH_IDX,\
    3,otasw_mtxt_content)
#ifndef SD_UI
LDEF_PROGRESS_BAR(g_win_otaupg,otaupg_progress_bar,\
        &otaupg_progress_txt,    \
        BAR_L, BAR_T, BAR_W, BAR_H, \
    PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,\
    4,5,BAR_W-8,14)
#else
LDEF_PROGRESS_BAR(g_win_otaupg,otaupg_progress_bar,\
        &otaupg_progress_txt,    \
        BAR_L, BAR_T, BAR_W, BAR_H, \
    PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,\
    2,2,BAR_W-4,12)
#endif
LDEF_TXT_PROGRESS(g_win_otaupg,otaupg_progress_txt, NULL, \
        TXTP_L, TXTP_T, TXTP_W, TXTP_H,display_strs[0])

LDEF_WIN(g_win_otaupg,&otaupg_mtxt_msg,W_L,W_T,W_W,W_H,1)

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/

TEXT_CONTENT otamsg_mtxt_content[] =
{
    {STRING_ID,{0}},
};

TEXT_CONTENT otasw_mtxt_content[] =
{
    {STRING_UNICODE,{0}}, //display_strs[10]
    {STRING_UNICODE,{0}}, //isplay_strs[11]
    {STRING_UNICODE,{0}}//display_strs[12]
};

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
OTA_STEP    prev_ota_step;
UINT32      g_otacurrent_time=0, g_otastart_time=0;
BOOL        b_send_key=FALSE;
BOOL        b_proc_failure_send_key=FALSE;
#endif

static PRESULT  win_otaupg_message_proc(UINT32 msg_type, UINT32 msg_code);

static INT32 win_otaupg_set_memory(void);
static INT32 win_otaupg_get_info(void);
static INT32 win_otaupg_download(void);

static void win_ota_upgrade_process_draw(UINT32 process);
/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
static void win_otaupg_init();
struct sto_device *otaupg_sto_device = NULL;

unsigned int otaupg_get_unzip_size(unsigned char *in)
{
    unsigned int size=0;

    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

static void step_burning_progress(void)
{
    m_ota_info.update_secotrs_index++;
    win_ota_upgrade_process_update_old((UINT8)(m_ota_info.update_secotrs_index*100 / m_ota_info.update_total_sectors));
}

#if 0
static void change_flash_protect(UINT32 low_addr, UINT32 high_addr)
{
    sto_flash_soft_protect(low_addr, high_addr);
    sflash_soft_protect_init();
}
#endif

INT32 ota_upg_burn_block(UINT8 *pbuffer, UINT32 pos, UINT8 numsectors)
{
    struct sto_device *sto_dev = otaupg_sto_device;
    UINT32 param[2]={0};
    INT32 offset=0;
    INT32 er = SUCCESS;
    UINT8 i=0;
    UINT32 temp=0;

    for(i=0; i<numsectors; i++)
    {
        offset = (pos + i) * C_SECTOR_SIZE;
        temp=(UINT32)C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] =temp>> 10; // length in K bytes
        OTA_PRINTF("%s: erase sector %d\n", __FUNCTION__, pos+i);
        er = sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param);
        if(er != SUCCESS)
        {
            OTA_PRINTF("%s: erase sector %d failed\n", __FUNCTION__, i);
            break;
        }

        if(pbuffer)
        {
            OTA_PRINTF("%s: seek sector %d, offset = %xh\n", __FUNCTION__, pos+i,offset);
            if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
            {
                OTA_PRINTF("%s: seek sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }

            OTA_PRINTF("%s: write sector %d, src = %xh\n",__FUNCTION__, pos+i,&pbuffer[C_SECTOR_SIZE * i]);
            if(sto_write(sto_dev, &pbuffer[C_SECTOR_SIZE * i],C_SECTOR_SIZE) != C_SECTOR_SIZE)
            {
                OTA_PRINTF("%s: write sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }
        }
        else
        {
            OTA_PRINTF("%s: Error in Line(%d), Please check !!!\n",__FUNCTION__, __LINE__);
        }

        step_burning_progress();
    }
    return er;
}

static INT8 char2hex(UINT8 ch)
{
    INT8 ret =  - 1;

    if ((ch <= HEX_NIME) && (ch >= HEX_ZERO))
    {
            // '0'~'9'
        ret = ch &0xf;
    }
    else if ((ch <= HEX_F_LOWERCASE) && (ch >= HEX_A_LOWERCASE))
    {
            //'a'~'f'
        ret = ch - 97+10;
    }
    else if ((ch <= HEX_F_UPPERCASE) && (ch >= HEX_A_UPPERCASE))
    {
            //'A'~'F'
        ret = ch - 65+10;
    }

    return ret;
}


UINT32 str2uint32(UINT8 *str, UINT8 len)
{
    UINT32 ret = 0;
    UINT8 i=0;
    INT temp=0;


    if (NULL == str)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        temp = char2hex(*str);
        str++;
        if (-1 == temp)
        {
            return 0;
        }

        ret = (ret << 4) | temp;
    }

    return ret;
}

static INT8 char2dec(UINT8 ch)
{
    INT8 ret = - 1;

    if ((ch <= HEX_NIME) && (ch >= HEX_ZERO))
    {
            // '0'~'9'
        ret = ch &0xf;
    }
    return ret;
}

UINT32 str2uint32_dec(UINT8 *str, UINT8 len)
{
    UINT32 ret = 0;
    UINT8 i=0;
    INT temp=0;

    if (NULL == str)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        temp = char2dec(*str);
        str++;
        if (-1 == temp)
        {
            return 0;
        }

        ret = ret *10 + temp;
    }

    return ret;
}


#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
static void update_flash_data(UINT32 offset, INT32 len, UINT8 *data)
{
    UINT8 *app_flash_buffer = NULL;
    INT32 func_ret=0;
	if(0 == func_ret)
	{
		;
	}

    struct sto_device *flash_dev = NULL;

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    app_flash_buffer = malloc(64*1024);

    func_ret=sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)app_flash_buffer);
    func_ret=sto_io_control(flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
    func_ret=sto_put_data(flash_dev, (UINT32)offset, data, (INT32)len);

    if(app_flash_buffer)
    {
        free(app_flash_buffer);
        app_flash_buffer = NULL;
        func_ret=sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
        func_ret=sto_io_control(flash_dev, STO_DRIVER_SET_FLAG, 0);
    }
}

static INT32 ota_cmd_start_resume(UINT32 image_addr, UINT32 image_size)
{
    INT32 ret = 0;
    INT8 i=0;
    UINT32 id=0;
    UINT32 mask=0;
    CHUNK_HEADER *p_m_header=NULL;
    CHUNK_HEADER *p_f_header=NULL;
    CHUNK_HEADER *p_r_header=NULL;
    UINT32 start_addr = 0;
    BOOL compat = TRUE;

    UINT8 m_chk_num=0;
    UINT8 f_chk_num=0;
    UINT8 r_chk_num=0;
    int func_ret=0;
    CHUNK_HEADER *m_chkhds = NULL;
    CHUNK_HEADER *f_chkhds = NULL;
    CHUNK_HEADER *r_chkhds = NULL;

	if(0 == r_chk_num)
	{
		;
	}
	
	 UINT32 boot_total_area_len=0;

    get_boot_total_area_len(&boot_total_area_len);
	m_chkhds = (CHUNK_HEADER *)malloc(CHUNK_ARRAY_SIZE);
    if (NULL == m_chkhds)
    {
        ASSERT(0);
        return -1;
    }
    f_chkhds = (CHUNK_HEADER *)malloc(CHUNK_ARRAY_SIZE);
    if (NULL == f_chkhds)
    {
        free(m_chkhds);
        ASSERT(0);
        return -1;
    }
    r_chkhds = (CHUNK_HEADER *)malloc(CHUNK_ARRAY_SIZE);
    if (NULL == r_chkhds)
    {
        free(m_chkhds);
        free(f_chkhds);
        return -1;
    }

    chunk_init(image_addr, image_size);

    m_chk_num = chunk_count(0,0);
    f_chk_num = sto_chunk_count(0,0);

    // memory chunk information
    MEMSET(m_chkhds, 0, CHUNK_ARRAY_SIZE);
    p_m_header = m_chkhds;
    start_addr = 0;

    for(i = 0; i < m_chk_num; i++)
    {
        id = 0;
        mask = 0;

        if(NULL != chunk_goto(&id, mask, i+1))
        {
            func_ret=get_chunk_header(id, p_m_header);
            if(func_ret != 1)
            {
                ret = -1;
                goto fail;
            }
            MEMCPY(p_m_header->reserved, &start_addr, 4);

            start_addr += p_m_header->offset;
        }

        p_m_header++;
    }

    // flash chunk information
    MEMSET(f_chkhds, 0, CHUNK_ARRAY_SIZE);
    p_f_header = f_chkhds;
    start_addr = 0;

    for(i = 0; i < f_chk_num; i++)
    {
        id =0;
        mask = 0;
        if(ERR_FAILURE != (INT32)sto_chunk_goto(&id, mask, i+1))
        {
            func_ret=sto_get_chunk_header(id, p_f_header);
            if(func_ret != 1)
            {
                ret = -1;
                goto fail;
            }

            MEMCPY(p_f_header->reserved, &start_addr, 4);

            start_addr += p_f_header->offset;

        }

        p_f_header++;
    }

    // if memory chunk number is bigger than flash chunk number,
    //then there must be something wrong
    r_chk_num = m_chk_num - f_chk_num;
    if(f_chk_num < m_chk_num)
    {
        MEMSET(r_chkhds, 0, sizeof(r_chkhds));

        p_m_header = m_chkhds;
        p_f_header = f_chkhds;
        for(i = 0; i < f_chk_num; i++)
        {
            if(p_m_header->id == p_f_header->id)
            {
                if((TRUE == compat )&& (p_m_header->offset != p_f_header->offset))
                {
                    compat = FALSE;
                }
            }
            else
            {
                break;
            }

            p_m_header++;
            p_f_header++;
        }

        p_r_header = r_chkhds;

        if(i == f_chk_num)// IDs of memory and flash chunk list are same.
        {
            if(TRUE == compat)
            {
                for(i = f_chk_num; i < m_chk_num; i++)
                {  
                	 #if 0
                    MEMCPY(p_r_header,  p_m_header, sizeof(CHUNK_HEADER));
                    MEMCPY((UINT8 *)&start_addr, p_m_header->reserved, 4);

                    if(sto_fetch_long(start_addr +p_m_header->offset) == (p_m_header+1)->id)
            //next chunk id is matched with with one chunk link
                    {
                        //copy header to its target place
                        update_flash_data(start_addr, sizeof(CHUNK_HEADER), (UINT8 *)(image_addr+start_addr));

                        if(sto_chunk_count(0,0) == m_chk_num)
                        {
                            OTA_PRINTF("flash chun list is recovered\n");
                            break;
                        }
                    }
                  #endif
                    MEMCPY(p_m_header->version,"00000001",16);
                    MEMCPY(p_r_header,  p_m_header, sizeof(CHUNK_HEADER));
                    MEMCPY((UINT8 *)&start_addr, p_m_header->reserved, 4);
                    update_flash_data(start_addr+boot_total_area_len, sizeof(CHUNK_HEADER), (UINT8 *)(image_addr+start_addr));
                    if(sto_chunk_count(0,0) == m_chk_num)
                     {
                          OTA_PRINTF("flash chun list is recovered\n");
                          break;
                    }

                    p_r_header++;
                    p_m_header++;
                }
            }
        }

    }

fail:
    free(m_chkhds);
    free(f_chkhds);
    free(r_chkhds);
    return ret;
}
#ifdef OTA_NEWFLOW
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
#else
UINT32 get_code_version(UINT8 *data_addr, UINT32 data_len)
{
    UINT32 code_len=0;
    UINT32 ver = 0;

    if (NUM_ZERO == test_rsa_ram((UINT32)data_addr, data_len))
    {
        code_len = fetch_long(data_addr + data_len - CODE_LENGTH_OFFSET);
        OTA_PRINTF("\tdecrypt_len:0x%08x\n", data_len);
        OTA_PRINTF("\tcode len:0x%08x\n", code_len);

        if ((code_len <= data_len - CODE_APPENDED_LEN- CODE_VERSION_PADDED_LEN_MIN) &&
            (code_len >= data_len - CODE_APPENDED_LEN - CODE_VERSION_PADDED_LEN_MAX))
        {
            ver = str2uint32(data_addr + code_len- CODE_VERSION_OFFSET, CODE_VERSION_LEN);
        }
        else
        {
            OTA_PRINTF("Invalid code length!\n");
        }
        OTA_PRINTF("\tcode version: 0x%X\n", ver);
    }
    else
    {
        OTA_PRINTF("\tverify code failed!\n");
    }
    return ver;
}
#endif

#define NCRC1 0x40232425
#define NCRC2 0x4E435243

UINT32 get_see_code_version_f(UINT32 chk_id)
{
    //UINT32 code_len=0;
    UINT32 ver = 0;
    struct sto_device *flash_dev = NULL;
	UINT8 *buffer = NULL;
    CHUNK_HEADER    chk_hdr;
    UINT32 chunk_addr = 0;
    UINT32 check_crc = 0;
    UINT32 calc_crc = 0;
    UINT32 check_len = 0;
    int ret = 0;

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO); 
    if(NULL == flash_dev)
    {
        return ver;
    }

    MEMSET(&chk_hdr,0,sizeof(chk_hdr));
    chunk_addr = sto_chunk_goto(&chk_id,0xFFFFFFFF,1);
    //get_chunk_header
    ret = sto_get_chunk_header(chk_id, &chk_hdr); 
    if (ret > 0 && chunk_addr)
    {
        check_crc = chk_hdr.crc;
        check_len = chk_hdr.len;     
        OTA_PRINTF("[%s:%d] crc = 0x%8x \n",__FUNCTION__, __LINE__,check_crc); 
        if ((check_crc != NCRC1) && (check_crc != NCRC2))
        {
              if ( NULL == ( buffer = ( UINT8 * ) MALLOC ( check_len ) ) )
               {
                   OTA_PRINTF("\t [%s:%d] \n",__FUNCTION__, __LINE__); 
                   return 0;
               }
			  
              sto_get_data ( flash_dev, buffer, chunk_addr + 16, check_len );
              calc_crc = mg_table_driven_crc ( 0xFFFFFFFF, buffer, check_len );

            if(check_crc != calc_crc)
            {
      
				OTA_PRINTF("\t[%s:%d] verify crc failed!, (%x-%x)\n",__FUNCTION__, __LINE__,check_crc,calc_crc); 
                return ver;
            }
            OTA_PRINTF("[%s:%d] chunk(%08x) crc check pass \n",__FUNCTION__, __LINE__,chk_id);
        }

        ver = str2uint32_dec(chk_hdr.version, CODE_VERSION_LEN);
        OTA_PRINTF("\t [%s:%d] code version: 0x%X\n",__FUNCTION__, __LINE__,ver);        
    }
    return ver;
    
}

UINT32 get_see_code_version_m(UINT32 chk_id,UINT8* addr)
{   
    UINT32 ver = 0;    
    UINT32 check_crc = 0;
    UINT32 calc_crc = 0;
    UINT32 check_len = 0;
    UINT32 id = 0;

    id = fetch_long(addr + CHUNK_ID);
    check_len = fetch_long(addr + CHUNK_LENGTH);
    check_crc = fetch_long(addr + CHUNK_CRC);

    if(chk_id != id)
    {
        OTA_PRINTF("ChunkID check failed, %08x-%08x \n",chk_id,id); 
        return ver;
    }    

    OTA_PRINTF("crc = 0x%8x \n",check_crc);    

    if ((check_crc != NCRC1) && (check_crc != NCRC2))
    {
        OTA_PRINTF("[%s:%d] addr = 0x%8x \n",__FUNCTION__, __LINE__,addr); 
        calc_crc = (UINT32)mg_table_driven_crc(0xFFFFFFFF, addr + 0x10, check_len);
        if(check_crc != calc_crc)
        {
            OTA_PRINTF("\tverify crc failed!, (%x-%x)\n",check_crc,calc_crc);
            return ver;
        }
        OTA_PRINTF("chunk(%08x) crc check pass \n",chk_id);
    } 
	
    ver = str2uint32_dec(addr + CHUNK_VERSION, CODE_VERSION_LEN);
    OTA_PRINTF("\tcode version: 0x%X\n", ver);
 
    return ver;
	
}

#ifdef _CAS9_VSC_ENABLE_
UINT32 vsc_version_transfer(UINT8 *buf)
{
    UINT8 i, j;
    UINT32 version=0;

    for (i = 0; i < 12; i++)
    {
        if (0 != buf [i])
        {
            version = 0;
            return version;
        }
    }
    version = 0;
    for (j = 0; j < 4; j++)
    {
        version |= (buf [i + j] << ((3 - j) << 2));
        OTA_PRINTF("%s (%d) 0x%X\n", __FUNCTION__, __LINE__, version);
    }
    return version;
}
#endif

#if defined(_M3503_) && !defined(OTA_NEWFLOW)

static UINT32 ENTRY_ADDR=0;
static UINT32 unzip_length=0;
static UINT8 *g_see_unziped   = NULL;

static UINT8 *expand_ram ( UINT32 addr, UINT32 len,\
             int unzip ( UINT8 *, UINT8 *, UINT8 * ), UINT32 type )
{
    UINT8 *buffer;
    UINT8 *codeentry;

    ENTRY_ADDR=m_ota_info.swap_addr+m_ota_info.swap_len;
    unzip_length = 0;

    codeentry = ( UINT8 * ) ENTRY_ADDR;

    buffer = malloc ( BUFFER_SIZE );
    if ( buffer == NULL )
    {
        FIXED_PRINTF ( "ota : No decompress buffer!\n" );
        return 0;
    }

    if ( unzip ( ( void * ) ( addr ), codeentry, buffer ) != NUM_ZERO )
    {
        codeentry = 0;
        FIXED_PRINTF ( "ota : error to decompress chunk(type %d)!\n", type );
    }
    else
    {
        unzip_length = * ( UINT32* ) buffer;
    }
    osal_cache_flush(codeentry,unzip_length);
    free ( buffer );
    return codeentry;
}

static UINT8* ota_unzip_sw(UINT8 *decrypted, UINT32 mc_len,UINT32 type)
{
    UINT32 ziped_len = 0;
    UINT8 *p=NULL;
    UINT8 *entry=NULL;

    p = decrypted + ( mc_len - 0x200 );
    ziped_len = p[3] | ( p[2] << 8 ) | ( p[1] << 16 ) | ( p[0] << 24 );
    FIXED_PRINTF ( "expand code, ziped len: 0x%x\n", ziped_len );

    entry = expand_ram ( decrypted, ziped_len, un7zip, type );

    return entry;
}

BOOL otaupg_check(struct otaupg_ver_info *ver_info)
{
#ifdef  _BUILD_OTA_E_
    struct sto_device *flash_dev = NULL;
#endif
#ifdef _CAS9_VSC_ENABLE_
    UINT32 block_addr = 0;
    UINT32 block_len = 0;
    UINT8 *temp_buffer=NULL;
    UINT8 *f_temp = NULL;
    UINT32 ret_len=0;
#endif
    UINT8 *addr;
    UINT32 len;
    UINT32 chunk_id = 0;
    UINT8 *data;
    UINT32 data_len;
    UINT8 key_pos;
    int ret;

    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);
    chunk_init(m_ota_info.uncompressed_addr, m_ota_info.uncompressed_len);

    // resume chunk list
    ota_cmd_start_resume(m_ota_info.uncompressed_addr,m_ota_info.uncompressed_len);

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
    sto_chunk_init(0, flash_dev->totol_size);
#endif

    OTA_PRINTF("fetch public key\n");
#ifdef _CAS9_VSC_ENABLE_
    if (decrypt_universal_key(&key_pos, ((DECRPT_KEY_ID & FLASH_CHUCK_ID_MASK)|FLASH_TRI_LADDER_MODE_KEY02)) < NUM_ZERO)
#else
    if (decrypt_universal_key(&key_pos, DECRPT_KEY_ID) < NUM_ZERO)
#endif
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // be compatible with bootloader check module,
    //move this function call into library
    //fetch_sys_pub_key();

    // get the versions of codes in flash
    addr = (UINT8 *)m_ota_info.decrypted_data_addr;

    OTA_PRINTF("check main code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, MAIN_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO)&&(ota_unzip_sw(addr, len,MAIN_CODE)!=NUM_ZERO) )
        ver_info->m_f_ver = get_code_version(ENTRY_ADDR, unzip_length);

#ifndef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check see code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO)&&(ota_unzip_sw(addr, len,SEE_CODE)!=NUM_ZERO) )
        ver_info->s_f_ver =  get_code_version(ENTRY_ADDR, unzip_length);
#endif

#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check VSC code in flash\n");

    ret = sto_get_chunk_len(CHUNKID_VSC_CODE, &block_addr, &block_len);
    if ((ret == 0) && (block_len != 0))
    {
        OTA_PRINTF("VSC code block_addr=0x%x,block_len=0x%x\n", block_addr,block_len);
        f_temp = (UINT8 *)MALLOC(block_len + 0xf);
        if (NULL == f_temp)
        {
            OTA_PRINTF("MALLOC Failed!\n");
        }
        else
        {
            temp_buffer = (UINT8 *)((0xFFFFFFF8 & (UINT32)f_temp));

            if (flash_dev == NULL)
            {
                OTA_PRINTF("Can't find FLASH device!\n");
            }

            ret_len=sto_get_data(flash_dev, (UINT8 *)temp_buffer, block_addr, block_len);
            if(ret_len!=block_len)
            {
                OTA_PRINTF("Get VSC from flash Failed!ret_len=0x%x,block_len=0x%x\n",ret_len,block_len);
            }
            else
            {
                if(test_vsc_rsa_ram ( ( UINT32 ) temp_buffer, block_len ) == 0)  //check VSC signature
                {
                    // ver_info->vsc_f_ver=vsc_version_transfer(temp_buffer+block_len-(VSC_SIGNATURE_LEN+VSC_VERSION_LEN));
                    MEMCPY ((void *)ver_info->vsc_f_ver, (void *)(temp_buffer + block_len - (VSC_SIGNATURE_LEN + VSC_VERSION_LEN)), VSC_VERSION_LEN);
                }
            }
            FREE(f_temp);
            f_temp = NULL;
        }
    }
#endif

    OTA_PRINTF("check ota loader main code in memory\n");

    chunk_id = OTA_MAIN_CODE_CHUNK_ID;
    m_ota_info.ota_main_code_addr = m_ota_info.ota_bin_addr;

    if (m_ota_info.ota_main_code_addr)
    {
        data = (UINT8 *)m_ota_info.ota_main_code_addr;
        m_ota_info.ota_main_code_size = fetch_long(data + CHUNK_OFFSET);
        m_ota_info.ota_loader_size = m_ota_info.ota_main_code_size;//OTA M+S
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if ((NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos, addr, data, data_len))\
        &&(ota_unzip_sw(addr, data_len,MAIN_CODE)!=NUM_ZERO) )
            ver_info->o_m_ver = get_code_version(ENTRY_ADDR, unzip_length);
    }
    OTA_PRINTF("ota loader main code: 0x%X, 0x%X\n",\
     m_ota_info.ota_main_code_addr, m_ota_info.ota_main_code_size);
    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        data = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if ((NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos, addr, data, data_len) )\
    &&(ota_unzip_sw(addr, data_len,MAIN_CODE)!=NUM_ZERO) )
            ver_info->m_m_ver =get_code_version(ENTRY_ADDR, unzip_length);
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n",  m_ota_info.main_code_addr, m_ota_info.main_code_size);

#ifdef _CAS9_VSC_ENABLE_
    if (decrypt_universal_key(&key_pos, ((DECRPT_KEY_ID_SEE & FLASH_CHUCK_ID_MASK)|FLASH_TRI_LADDER_MODE_KEY02)) < NUM_ZERO)
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    OTA_PRINTF("check see code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO)&&(ota_unzip_sw(addr, len,SEE_CODE)!=NUM_ZERO) )
    {
        ver_info->s_f_ver =  get_code_version(ENTRY_ADDR, unzip_length);
    }
#endif

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        data = (UINT8 *)m_ota_info.see_code_addr;
        m_ota_info.see_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH)- CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if ((NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos, addr, data, data_len) )\
        &&(ota_unzip_sw(addr, data_len,SEE_CODE)!=NUM_ZERO) )
            ver_info->s_m_ver = get_code_version(ENTRY_ADDR, unzip_length);
    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n",\
    m_ota_info.see_code_addr, m_ota_info.see_code_size);

#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check vsc code in memory\n");
    chunk_id = CHUNKID_VSC_CODE;
    m_ota_info.vsc_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.vsc_code_addr)
    {
        data = (UINT8 *)m_ota_info.vsc_code_addr;
        m_ota_info.vsc_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if(test_vsc_rsa_ram ( ( UINT32 ) data, data_len) == 0 )
        {
            // ver_info->vsc_m_ver=vsc_version_transfer(data+data_len-(VSC_SIGNATURE_LEN+VSC_VERSION_LEN));
            MEMCPY ((void *)ver_info->vsc_m_ver, (void *)(data + data_len - (VSC_SIGNATURE_LEN + VSC_VERSION_LEN), VSC_VERSION_LEN));
        }
    }
    OTA_PRINTF("vsc code: 0x%X, 0x%X\n", m_ota_info.vsc_code_addr, m_ota_info.vsc_code_size);
    // OTA_PRINTF("vsc flash ver=0x%X memory version=0x%X\n",ver_info->vsc_f_ver,ver_info->vsc_m_ver);
#endif

    OTA_PRINTF("version info: (0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver,ver_info->o_b_ver, ver_info->m_f_ver,\
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
    return TRUE;
}
#elif defined(OTA_NEWFLOW)
// ota new flow
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

UINT32 get_chunk_in_flash(UINT32 ck_id,UINT8 *buf)
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
    chunk_hdr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFF0000, 1); 
    if ((ERR_PARA == (INT32)chunk_hdr) || (ERR_FAILUE == (INT32)chunk_hdr) )
    {
        return 0;
    }
    // ret = sto_get_data(test_flash_dev, buf, chunk_hdr, CHUNK_HEADER_SIZE);
    // if (CHUNK_HEADER_SIZE != ret)
    // {
        // return 0;
    // }
    // data_len = fetch_long(buf + CHUNK_OFFSET) - CHUNK_HEADER_SIZE;
    // ret = sto_get_data(test_flash_dev, &buf[CHUNK_HEADER_SIZE], (chunk_hdr + CHUNK_HEADER_SIZE), data_len);
    data_len = sto_fetch_long(chunk_hdr + CHUNK_OFFSET) - CHUNK_HEADER_SIZE;
    ret = sto_get_data(test_flash_dev, buf, (chunk_hdr + CHUNK_HEADER_SIZE), data_len);
    if ((INT32)data_len != ret)
    {
        return 0;
    }
    // return (data_len + CHUNK_HEADER_SIZE);
    return data_len;
}

UINT32 get_ram_chunk_version(UINT8* data,UINT8 key_pos,UINT8* tmp_buf)
{
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 ck_ver = 0;
    INT32 ret = 0;
    
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(tmp_buf,data,CHUNK_HEADER_SIZE);
    data += CHUNK_HEADER_SIZE;
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
    INT32 ret = 0;
    
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(tmp_buf,data,CHUNK_HEADER_SIZE);
    data += CHUNK_HEADER_SIZE;
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
    RET_CODE ret = RET_SUCCESS;
    UINT32 key_id = 0;  

    if(pos == NULL)
    {
        return RET_FAILURE;
    }

    if((ALI_C3505 ==sys_ic_get_chip_id())||(ALI_C3711C ==sys_ic_get_chip_id()))
    {
        key_id = (NEW_HEAD_ID & NEW_HEAD_MASK);
        #ifdef _CAS9_CA_ENABLE_
        if(app_key_idx == E_KEY_MAIN)
        {
            key_id |= FIRST_FLASH_KEY;
        }
        else if(app_key_idx == E_KEY_SEE)
        {
            key_id |= THIRD_FLASH_KEY;
        }

        #ifdef _CAS9_VSC_ENABLE_
            key_id |= FLASH_TRI_LADDER_MODE_KEY02;
        #endif
        #else
        //for gca
        key_id |= FIRST_FLASH_KEY;
        #endif

        ret = decrypt_universal_key(pos, key_id) ;
    }
    else
    {   
        #ifdef _CAS9_VSC_ENABLE_
        if(ALI_S3821==sys_ic_get_chip_id())
        {
            key_id = (NEW_HEAD_ID & NEW_HEAD_MASK) |FLASH_TRI_LADDER_MODE_KEY02;
        }
        else
        {
            key_id = (DECRPT_KEY_ID & FLASH_CHUCK_ID_MASK) | FLASH_TRI_LADDER_MODE_KEY02;
        }
        #ifdef _CAS9_VSC_API_ENABLE_
        ret = decrypt_vsc_universal_key(pos, key_id) ;
        #else
        ret = decrypt_universal_key(pos, key_id) ;
        #endif
        
        #else
        if((ALI_S3821==sys_ic_get_chip_id())||(ALI_C3702==sys_ic_get_chip_id()))
        {
            key_id = (NEW_HEAD_ID&NEW_HEAD_MASK)|FIRST_FLASH_KEY;
        }
        else
        {
            key_id = DECRPT_KEY_ID;
        }
        ret = decrypt_universal_key(pos, key_id) ;    
        #endif
    }

    return ret;
}

UINT8 get_rsa_key_id_by_type(UINT8 key_type)
{
    UINT8 rsa_key_id = 0;

    #ifdef _CAS9_CA_ENABLE_
    // cas9 use another RSA key to sign see/ota see.
    if(key_type == E_KEY_SEE)
    {
        rsa_key_id = 1;
    }
    #endif        
    return rsa_key_id;
}

//----using now -------
BOOL otaupg_check(struct otaupg_ver_info *ver_info)
{
#ifdef  _BUILD_OTA_E_
    struct sto_device *flash_dev = NULL;
#endif
#ifdef _CAS9_VSC_ENABLE_
#ifndef _CAS9_VSC_API_ENABLE_
    UINT32 block_addr = 0;
    UINT32 block_len = 0;
#endif
#endif
    UINT8 *addr = NULL ;
    UINT32 len = 0;
    UINT32 chunk_id = 0;
    UINT8 *data = NULL ;
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT8 key_pos = 0;
    int ret = 0;
    UINT32 boot_total_area_len = 0;
    __MAYBE_UNUSED__ UINT32 key_id = 0;
#ifdef _OUC_LOADER_IN_FLASH_    
    UINT8 *temp_buffer = NULL;
#endif
    UINT8 rsa_key_id __MAYBE_UNUSED__ = 0;

    get_boot_total_area_len(&boot_total_area_len);

    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);

    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len, m_ota_info.uncompressed_len-boot_total_area_len);

    // resume chunk list
    ota_cmd_start_resume(m_ota_info.uncompressed_addr+boot_total_area_len,m_ota_info.uncompressed_len-boot_total_area_len);

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

    ret = decrypt_universal_key_wrapped(&key_pos,E_KEY_MAIN);
    if (ret < NUM_ZERO)
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // fetch main rsa key id
    rsa_key_id = get_rsa_key_id_by_type(E_KEY_MAIN);

    // get the versions of codes in flash
    addr = (UINT8 *)m_ota_info.decrypted_data_addr;    

    OTA_PRINTF("check main code in flash\n");
    len = 0;
    get_chunk_header_in_flash(MAIN_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk(key_pos, MAIN_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->m_f_ver = get_code_version(addr, len);
    }

#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check VSC code in flash\n");
// bing 20150326 patch 3281C to NEW_FLOW ========= start
#ifdef _CAS9_VSC_API_ENABLE_
    if (TRUE == get_chunk_header_in_flash (CHUNKID_VSC_CODE, addr))
    {
        data_len = CHUNK_HEADER_SIZE;
        data_len += get_chunk_in_flash(CHUNKID_VSC_CODE, (UINT8 *)(addr + data_len));
        //check VSC signature
        ret = test_vsc_rsa_ram ((UINT32)addr, data_len);
        if(NUM_ZERO == ret)
        {
            data_len = fetch_long(addr + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
            addr = addr + CHUNK_HEADER_SIZE;
            MEMCPY ((void *)ver_info->vsc_f_ver, (void *)(addr + data_len - VSC_VERSION_LEN), VSC_VERSION_LEN);
            OTA_PRINTF ("check VSC outer signature pass\n");
        }
        else
        {
            OTA_PRINTF ("check VSC outer signature fail\n");
        }
    }
#else
    data_len = 0;
    data_len = get_chunk_in_flash(CHUNKID_VSC_CODE, (UINT8 *)(addr));
    //check VSC signature
	ret = sto_get_chunk_len(CHUNKID_VSC_CODE, &block_addr, &block_len);
    if ((ret == NUM_ZERO) && (block_len != NUM_ZERO))
    {
        ret = test_vsc_rsa_ram ((UINT32)addr, block_len);
        if(NUM_ZERO == ret)
        {
            MEMCPY ((void *)ver_info->vsc_f_ver, (void *)(addr + block_len - VSC_SIGNATURE_LEN - VSC_VERSION_LEN), VSC_VERSION_LEN);
            OTA_PRINTF ("check VSC outer signature pass\n");
        }
        else
        {
            OTA_PRINTF ("check VSC outer signature fail\n");
        }
    }
#endif
// bing 20150326 patch 3281C to NEW_FLOW ========= end
#endif

#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("check ouc loader main in flash\n");
    len = 0;
    temp_buffer = (UINT8 *)m_ota_info.cipher_buf_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {        
        UINT8* ouc_main_addr = temp_buffer;
  		if ((*ouc_main_addr) + (*(ouc_main_addr+1)) == 0xFF)
		{
            ver_info->o_f_ver = get_ram_chunk_version(ouc_main_addr,key_pos,addr);
		}
    } 
#endif    

    OTA_PRINTF("check ota loader main code in memory\n");
    #ifdef _S3281_
    //3281
    chunk_id = OTA_LOADER_CHUNK_ID;
    m_ota_info.ota_loader_addr = m_ota_info.ota_bin_addr;
    if (m_ota_info.ota_loader_addr)
    {
        data = (UINT8 *)m_ota_info.ota_loader_addr;
        m_ota_info.ota_loader_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
            ver_info->o_m_ver = get_code_version(addr, sig_len);
    }
    OTA_PRINTF("ota loader: 0x%X, 0x%X\n", m_ota_info.ota_loader_addr, m_ota_info.ota_loader_size);
    #else
    //35xx
    chunk_id = OTA_MAIN_CODE_CHUNK_ID;
    m_ota_info.ota_loader_size = m_ota_info.ota_bin_size;//for ota upgrade all size calculate
    m_ota_info.ota_main_code_addr = m_ota_info.ota_bin_addr;
    if (m_ota_info.ota_main_code_addr)
    {
        data = (UINT8 *)m_ota_info.ota_main_code_addr;
        m_ota_info.ota_main_code_size = fetch_long(data + CHUNK_OFFSET);
        ver_info->o_m_ver = get_ram_chunk_version(data,key_pos,addr); 
    }
    OTA_PRINTF("ota loader main code: 0x%X, 0x%X\n",\
     m_ota_info.ota_main_code_addr, m_ota_info.ota_main_code_size);
    #endif
    
    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        data = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
            ver_info->m_m_ver = get_code_version_ext(addr, sig_len,0);
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n", m_ota_info.main_code_addr, m_ota_info.main_code_size);

/*******************************************************************/
#if (defined( _M3702_) || defined( _M3711C_)) 

    ver_info->s_f_ver = get_see_code_version_f(SEE_CODE_CHUNK_ID);
  
  #ifdef _OUC_LOADER_IN_FLASH_
    temp_buffer = (UINT8 *)m_ota_info.cipher_buf_addr ;
    chunk_id = OUC_CHUNK_ID;
    ret = get_chunk_in_flash(chunk_id,temp_buffer);
	temp_buffer = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);
	ver_info->os_f_ver = get_see_code_version_m(OUC_SEECODE_ID,temp_buffer);
  #endif

    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    ver_info->s_m_ver = get_see_code_version_m(chunk_id,(UINT8 *)(m_ota_info.see_code_addr));
    m_ota_info.see_code_size = fetch_long((unsigned char *)(m_ota_info.see_code_addr + CHUNK_OFFSET));
  #ifdef _OUC_LOADER_IN_FLASH_
    chunk_id = OUC_SEECODE_ID;
	m_ota_info.ota_see_code_addr =  m_ota_info.ota_bin_addr + fetch_long(((UINT8 *)m_ota_info.ota_bin_addr) + CHUNK_OFFSET);
    if (m_ota_info.ota_see_code_addr)
    {       
        ver_info->os_m_ver = get_see_code_version_m(chunk_id,((UINT8 *)m_ota_info.ota_see_code_addr));
    }    
  #endif
#else
	////////////////
		ret = decrypt_universal_key_wrapped(&key_pos,E_KEY_SEE);
		if (ret < NUM_ZERO)
		{
			libc_printf("Decrypt universal key failed!\n");
			return FALSE;
		}
    // fetch see rsa key id
    rsa_key_id = get_rsa_key_id_by_type(E_KEY_SEE);
   
    OTA_PRINTF("check see code in flash\n");
    len = 0;
    get_chunk_header_in_flash(SEE_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->s_f_ver = get_code_version_ext(addr, len,rsa_key_id);
    }

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        data = (UINT8 *)m_ota_info.see_code_addr;
        m_ota_info.see_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH)- CHUNK_HEADER_SIZE + CHUNK_NAME;
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
            ver_info->s_m_ver = get_code_version_ext(addr, sig_len,rsa_key_id);
    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n",\
    m_ota_info.see_code_addr, m_ota_info.see_code_size);

#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("check ouc loader see in flash\n");
    len = 0;
    temp_buffer = (UINT8 *)m_ota_info.cipher_buf_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {     
        UINT8* ouc_see_addr = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);  
		if ((*ouc_see_addr) + (*(ouc_see_addr+1)) == 0xFF)
	    {
            ver_info->os_f_ver = get_ram_chunk_version_ext(ouc_see_addr,key_pos,addr,rsa_key_id);
	    }
    } 
#endif    

    OTA_PRINTF("check ota loader see code in memory\n");
    chunk_id = OTA_MAIN_CODE_CHUNK_ID;
    m_ota_info.ota_loader_size = m_ota_info.ota_bin_size;//for ota upgrade all size calculate
    m_ota_info.ota_main_code_addr = m_ota_info.ota_bin_addr;
    if (m_ota_info.ota_main_code_addr)
    {
        data = (UINT8 *)m_ota_info.ota_main_code_addr;     
        #ifdef _OUC_LOADER_IN_FLASH_
        data = data + fetch_long(data + CHUNK_OFFSET);
        ver_info->os_m_ver = get_ram_chunk_version_ext(data,key_pos,addr,rsa_key_id);
        #endif
    }
#endif

#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check vsc code in memory\n");
    chunk_id = CHUNKID_VSC_CODE;
    m_ota_info.vsc_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.vsc_code_addr)
    {
// bing 20150326 patch 3281C to NEW_FLOW ========= start
#ifdef _CAS9_VSC_API_ENABLE_
        data = (UINT8 *)m_ota_info.vsc_code_addr;
        m_ota_info.vsc_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_OFFSET);
		ret = test_vsc_rsa_ram ((UINT32)data, data_len);
        if(NUM_ZERO == ret)
        {
            data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
            data = data + CHUNK_HEADER_SIZE;
            // ver_info->vsc_m_ver=vsc_version_transfer(data + data_len - VSC_VERSION_LEN);
            MEMCPY ((void *)ver_info->vsc_m_ver, (void *)(data + data_len - VSC_VERSION_LEN), VSC_VERSION_LEN);
        }
#else
        data = (UINT8 *)m_ota_info.vsc_code_addr;
        m_ota_info.vsc_code_size = fetch_long(data + CHUNK_OFFSET);
		data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
		data += CHUNK_HEADER_SIZE;
		ret = test_vsc_rsa_ram ((UINT32)data, data_len);
        if(NUM_ZERO == ret)
        {
            MEMCPY ((void *)ver_info->vsc_m_ver, (void *)(data + data_len - (VSC_SIGNATURE_LEN + VSC_VERSION_LEN)), VSC_VERSION_LEN);
        }
#endif
// bing 20150326 patch 3281C to NEW_FLOW ========= end
    }
    OTA_PRINTF("vsc code: 0x%X, 0x%X\n", m_ota_info.vsc_code_addr, m_ota_info.vsc_code_size);
    // OTA_PRINTF("vsc flash ver=0x%X memory version=0x%X\n",ver_info->vsc_f_ver,ver_info->vsc_m_ver);
#endif

#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("version info: (0x%X,0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->os_f_ver,ver_info->o_m_ver,ver_info->os_m_ver,\
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#else
    OTA_PRINTF("version info: (0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver,ver_info->o_b_ver, ver_info->m_f_ver,\
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#endif
    ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);

#ifdef _MOD_DYNAMIC_LOAD_
    OTA_PRINTF("check dyn code in flash\n");
    CHUNK_HEADER chk_hdr;
    UINT32 data_crc = 0;
    UINT32 crc = 0;
    UINT32 crc_len = 0;

    ver_info->dyn_f_ver = 0;
    addr = (UINT8 *)find_chunk_by_sector(boot_total_area_len,MOD_PLUGIN_CHUNK_ID,0xFFFFFFFF,1);

    if(addr != 0)
    {
        if(1 == sto_get_chunk_header(MOD_PLUGIN_CHUNK_ID, &chk_hdr)) 
            ver_info->dyn_f_ver = str2uint32_dec(chk_hdr.version, CODE_VERSION_LEN);     
    }
        
    OTA_PRINTF("check dyn code in memory\n");
    ver_info->dyn_m_ver = 0;
    chunk_id = MOD_PLUGIN_CHUNK_ID;
    m_ota_info.dyn_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.dyn_code_addr)
    {
        data = (UINT8 *)m_ota_info.dyn_code_addr;
        m_ota_info.dyn_code_size = fetch_long(data + CHUNK_OFFSET);
        crc_len = fetch_long(data + CHUNK_LENGTH);
        data_crc = fetch_long(data + CHUNK_CRC);

        mg_setup_crc_table();
        crc = mg_table_driven_crc ( 0xFFFFFFFF, data + 16, crc_len );
        if(crc != data_crc)
            OTA_PRINTF("dyn code in memory CRC fail\n");
        else
        {
            ver_info->dyn_m_ver = str2uint32_dec(data + CHUNK_VERSION, CODE_VERSION_LEN);
        }

    }
    OTA_PRINTF("dyn version info:  (0x%X, 0x%X)\n", ver_info->dyn_f_ver, ver_info->dyn_m_ver);
#endif        
    return TRUE;
}

#else
//3281 old flow.
BOOL otaupg_check(struct otaupg_ver_info *ver_info)
{
#ifdef _CAS9_VSC_ENABLE_
    UINT32 block_addr = 0;
    UINT32 block_len = 0;
    UINT8 *temp_buffer=NULL;
    UINT8 *f_temp = NULL;
    UINT32 ret_len=0;
#endif
    UINT8 *addr=NULL;
    UINT32 len=0;
    UINT32 chunk_id = 0;
    UINT8 *data=NULL;
    UINT32 data_len=0;
    UINT8 key_pos=0;
    int ret=0;
    INT32 func_return_value=0;
    RET_CODE func_ret=RET_FAILURE;
    struct sto_device *flash_dev = NULL;

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size ((UINT8 *)m_ota_info.compressed_addr);
    chunk_init(m_ota_info.uncompressed_addr, m_ota_info.uncompressed_len);

    // resume chunk list
    func_return_value=ota_cmd_start_resume(m_ota_info.uncompressed_addr,m_ota_info.uncompressed_len);

    // decrypt universal key, fetch public key from flash
#ifdef  _BUILD_OTA_E_
    //because the function of ota_load_parameter
    //will call sto_chunk_init to find ota parameter.
    //so cann't find the key chunk,get the wrong key.
    if (NULL == flash_dev)
    {
        OTA_PRINTF("Can't find FLASH device!\n");
    }
    func_return_value=sto_open(flash_dev);
    sto_chunk_init(0, flash_dev->totol_size);
#endif

    OTA_PRINTF("fetch public key\n");
#ifdef _CAS9_VSC_ENABLE_
    if (decrypt_universal_key(&key_pos, ((DECRPT_KEY_ID & FLASH_CHUCK_ID_MASK)|FLASH_TRI_LADDER_MODE_KEY02)) < NUM_ZERO)
#else
    if (decrypt_universal_key(&key_pos, DECRPT_KEY_ID) < NUM_ZERO)
#endif
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // be compatible with bootloader check module,
    //move this function call into library
    //fetch_sys_pub_key();

    // get the versions of codes in flash
    addr = (UINT8 *)m_ota_info.decrypted_data_addr;

#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_)
    OTA_PRINTF("check ota loader in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, OTA_LOADER_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO))
    {
        ver_info->o_f_ver = get_code_version(addr, len/*, &public_key*/);
    }
#endif

    OTA_PRINTF("check main code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, MAIN_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO))
    {
        ver_info->m_f_ver = get_code_version(addr, len/*, &public_key*/);
    }

#ifndef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check see code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO))
    {
        ver_info->s_f_ver = get_code_version(addr, len/*, &public_key*/);
    }
#endif

#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check VSC code in flash\n");

    ret = sto_get_chunk_len(CHUNKID_VSC_CODE, &block_addr, &block_len);
    if ((ret == 0) && (block_len != 0))
    {
        OTA_PRINTF("VSC code block_addr=0x%x,block_len=0x%x\n", block_addr,block_len);
        f_temp = (UINT8 *)MALLOC(block_len + 0xf);
        if (NULL == f_temp)
        {
            OTA_PRINTF("MALLOC Failed!\n");
        }
        else
        {
            temp_buffer = (UINT8 *)((0xFFFFFFF8 & (UINT32)f_temp));

            if (flash_dev == NULL)
            {
                OTA_PRINTF("Can't find FLASH device!\n");
            }

            ret_len=sto_get_data(flash_dev, (UINT8 *)temp_buffer, block_addr, block_len);
            if(ret_len!=block_len)
            {
                OTA_PRINTF("Get VSC from flash Failed!ret_len=0x%x,block_len=0x%x\n",ret_len,block_len);
            }
            else
            {
                if(test_vsc_rsa_ram ( ( UINT32 ) temp_buffer, block_len ) == 0)  //check VSC signature
                {
                    // ver_info->vsc_f_ver=vsc_version_transfer(temp_buffer+block_len-(VSC_SIGNATURE_LEN+VSC_VERSION_LEN));
                    MEMCPY ((void *)ver_info->vsc_f_ver, (void *)(temp_buffer + block_len - (VSC_SIGNATURE_LEN + VSC_VERSION_LEN)), VSC_VERSION_LEN);
                }
            }
            FREE(f_temp);
            f_temp = NULL;
        }
    }
#endif
#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_)
    // get the version of backup ota loader
    OTA_PRINTF("check backup ota loader in flash\n");
    if (OTA_LOADER_BACKUP_LEN == sto_get_data(flash_dev, (UINT8 *)m_ota_info.cipher_buf_addr,
            OTA_LOADER_BACKUP_ADDR, OTA_LOADER_BACKUP_LEN))
    {
        data = (UINT8 *)m_ota_info.cipher_buf_addr;
        if (fetch_long(data + CHUNK_ID) != OTA_LOADER_CHUNK_ID)
        {
            ver_info->o_b_ver = 0; // set to invalid version
        }
        else
        {
            data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
            data += CHUNK_HEADER_SIZE;
            if (NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos, addr, data, data_len))
            {
                ver_info->o_b_ver = get_code_version(addr, data_len/*, &public_key*/);
            }
        }
    }
#endif

    OTA_PRINTF("check ota loader in memory\n");
#if !defined (_M3383_SABBAT_) &&  !defined (_ENABLE_4M_FLASH_)
    chunk_id = OTA_LOADER_CHUNK_ID;
    m_ota_info.ota_loader_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
#else
    m_ota_info.ota_loader_addr = m_ota_info.ota_bin_addr;
#endif
    if (m_ota_info.ota_loader_addr)
    {
        data = (UINT8 *)m_ota_info.ota_loader_addr;
        m_ota_info.ota_loader_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH)- CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if (NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos, addr, data, data_len))
        {
            ver_info->o_m_ver =get_code_version(addr, data_len/*, &public_key*/);
        }
    }
    OTA_PRINTF("ota loader: 0x%X, 0x%X\n",  m_ota_info.ota_loader_addr, m_ota_info.ota_loader_size);

    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        data = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH)- CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if (NUM_ZERO == aes_cbc_decrypt_ram_chunk(key_pos,addr, data, data_len))
        {
            ver_info->m_m_ver =get_code_version(addr, data_len);
        }
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n",m_ota_info.main_code_addr, m_ota_info.main_code_size);

#ifdef _CAS9_VSC_ENABLE_
    if (decrypt_universal_key(&key_pos, ((DECRPT_KEY_ID_SEE & FLASH_CHUCK_ID_MASK)|FLASH_TRI_LADDER_MODE_KEY02)) < NUM_ZERO)
    {
        OTA_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    OTA_PRINTF("check see code in flash\n");
    len = 0;
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, addr, &len);
    if ((NUM_ZERO == ret) && (len != NUM_ZERO))
    {
        ver_info->s_f_ver = get_code_version(addr, len/*, &public_key*/);
    }
#endif

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        data = (UINT8 *)m_ota_info.see_code_addr;
        m_ota_info.see_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH)- CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if (NUM_ZERO ==aes_cbc_decrypt_ram_chunk(key_pos, addr,data, data_len))
        {
            ver_info->s_m_ver = get_code_version(addr, data_len);
        }
    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n", m_ota_info.see_code_addr, m_ota_info.see_code_size);
#ifdef _CAS9_VSC_ENABLE_
    OTA_PRINTF("check vsc code in memory\n");
    chunk_id = CHUNKID_VSC_CODE;
    m_ota_info.vsc_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.vsc_code_addr)
    {
        data = (UINT8 *)m_ota_info.vsc_code_addr;
        m_ota_info.vsc_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        data += CHUNK_HEADER_SIZE;
        if(test_vsc_rsa_ram ( ( UINT32 ) data, data_len) == 0 )
        {
            // ver_info->vsc_m_ver=vsc_version_transfer(data+data_len-(VSC_SIGNATURE_LEN+VSC_VERSION_LEN));
            MEMCPY ((void *)ver_info->vsc_m_ver, (void *)(data + data_len - (VSC_SIGNATURE_LEN + VSC_VERSION_LEN)), VSC_VERSION_LEN);
        }
    }
    OTA_PRINTF("vsc code: 0x%X, 0x%X\n", m_ota_info.vsc_code_addr, m_ota_info.vsc_code_size);
    OTA_PRINTF("vsc flash ver=0x%X memory version=0x%X\n",ver_info->vsc_f_ver,ver_info->vsc_m_ver);
#endif
    OTA_PRINTF("version info: (0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver, ver_info->o_b_ver, ver_info->m_f_ver,
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    func_ret=ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
    return TRUE;
}
#endif /* _M3503_ */
#endif /* _CAS9_CA_ENABLE_ */


INT32 ota_upg_backup_database(void)
{
    struct sto_device *sto_dev = otaupg_sto_device;


    if(sto_lseek(sto_dev, (INT32)m_ota_info.user_db_addr_offset,STO_LSEEK_SET) \
        !=  (INT32)m_ota_info.user_db_addr_offset)
    {
        return ERR_FAILED;
    }
    if(sto_read(sto_dev, (UINT8 *)m_ota_info.backup_db_addr,(INT32)m_ota_info.user_db_len) \
       != (INT32)m_ota_info.user_db_len)
     {
        return ERR_FAILED;
     }
    return SUCCESS;
}


/*Bool_flash_mem(): add by colin to judge wether the third block id(ciplus chunk id)
 of the flash and the memory is identical*/
UINT16 bool_flash_mem(void)
{
      UINT16 flag=0;
          UINT8 *addr=NULL;
      UINT32 bl_sector=0;
      UINT32 offset=0 ;
      //struct sto_device *flash_dev;
      UINT32 ci_chid=0;
      //add by colin:used for restore the ciplus chunk_id
      UINT32 *pciplus=NULL;
       //add by colin:used for reading the ciplus chunk_id
      unsigned char *data=NULL;

      addr = (UINT8 *)SYS_FLASH_BASE_ADDR;
      offset = ((UINT32)(*(addr + 8)) << 24) | ((UINT32)(*(addr + 9)) << 16) \
            | ((UINT32)(*(addr + 10)) << 8) | ((UINT32)(*(addr + 11)) << 0);
           bl_sector = offset / C_SECTOR_SIZE;
       if (NUM_ZERO != offset % C_SECTOR_SIZE)
       {
            bl_sector++;
       }
       addr = addr+bl_sector *C_SECTOR_SIZE;
       pciplus = (UINT32 *)addr;
       data = (unsigned char *)pciplus;
       ci_chid = fetch_long(data + CHUNK_ID);
       libc_printf("ci's chunk id is %d",ci_chid);
       if(0x09F60101==ci_chid)
       {
          flag=1;
       }
       else
       {
          flag=0;
       }
       libc_printf("ciFlag is %d\n",flag);
       return flag;

}
INT32 burn_flash(void)
{
    INT32 er=0;
    UINT32 offset=0;
    UINT32 bl_sector=0;
    UINT32 bl_count=0;
    //add by colin :used for count the block number of ciplus key
    UINT8 *addr=NULL;
    RET_CODE  func_ret=RET_FAILURE;

	if(RET_FAILURE == func_ret)
	{
		;
	}
	sys_data_set_factory_reset(TRUE);
    sys_data_save(0);
    func_ret=ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_POOLING);
    osal_task_dispatch_off();
    //osal_interrupt_disable();
    do
    {
        addr = (UINT8 *)SYS_FLASH_BASE_ADDR;
        offset = ((UINT32)(*(addr + 8)) << 24) | ((UINT32)(*(addr + 9)) << 16) \
            | ((UINT32)(*(addr + 10)) << 8) | ((UINT32)(*(addr + 11)) << 0);
        bl_sector = offset / C_SECTOR_SIZE;
        if (NUM_ZERO != offset % C_SECTOR_SIZE)
        {
            bl_sector++;  //  Colin>>This sentence has already skipped the HDCP key
        }

        #if defined(_BUILD_OTA_E_) && defined(_OUC_LOADER_IN_FLASH_)
            bl_sector += m_ota_info.ota_bin_sectors;
        #endif

        if(ci_flag)
    //Colin>>this "if...else.." is used for compatible
    //because we changed the block scripts
        {
            //skip ciplus   add by colin
            addr=addr+bl_sector *C_SECTOR_SIZE;
            offset = ((UINT32)(*(addr + 8)) << 24) | ((UINT32)(*(addr + 9)) << 16)|((UINT32)(*(addr + 10)) << 8)
                    |((UINT32)(*(addr + 11)) << 0);
            bl_count= offset / C_SECTOR_SIZE;
            if (NUM_ZERO != offset % C_SECTOR_SIZE)
            {
                bl_count++;
            }
            bl_sector+=bl_count;
            //end skip ciplus add by colin
        }

#ifndef _BUILD_OTA_E_

#ifdef BACKUP_TEMP_INFO
        er=save_swap_data(&system_config,sizeof(system_config));
#endif
#ifndef _OUC_LOADER_IN_FLASH_
        er = ota_upg_backup_database();
        if(SUCCESS != er)
        {
            break;
        }     

        // burn ota.bin
        //change_flash_protect(C_OTACODE_OFFSET,
        //C_OTACODE_OFFSET+C_OTACODE_SIZE);
        er = ota_upg_burn_block((UINT8*)m_ota_info.ota_bin_addr, m_ota_info.user_db_start_sector,
            m_ota_info.ota_bin_sectors);
        if(SUCCESS != er)
        {
            break;
        }
#endif           
        // burn firmware.bin
        //change_flash_protect(C_MAINCODE_OFFSET,
    //C_MAINCODE_OFFSET+C_MAINCODE_SIZE);
        er = ota_upg_burn_block((UINT8*)m_ota_info.ota_upg_addr,bl_sector, m_ota_info.ota_upg_sectors);
        if(SUCCESS != er)
        {
            break;
        }

#ifndef _OUC_LOADER_IN_FLASH_
        //change_flash_protect(C_DATABASE_OFFSET,
    //C_DATABASE_OFFSET+C_DATABASE_SIZE);
        er = ota_upg_burn_block((UINT8*)m_ota_info.backup_db_addr,\
         m_ota_info.user_db_start_sector, m_ota_info.user_db_sectors);
        if(SUCCESS != er)
        {
            break;
        }
#endif        
#else
        er = ota_upg_burn_block((UINT8*)m_ota_info.ota_upg_addr, bl_sector, m_ota_info.ota_upg_sectors);
        if(SUCCESS != er)
        {
            break;
        }
#ifndef _OUC_LOADER_IN_FLASH_
        //UpdateMessage("Restore database...");
        er = ota_upg_burn_block((UINT8*)NULL, m_ota_info.user_db_start_sector,m_ota_info.user_db_sectors);
        if(SUCCESS != er)
        {
            break;
        }
#endif        
#endif
    }while(0);
    //osal_interrupt_enable();
    osal_task_dispatch_on();
    func_ret=ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_INTERRUPT);
    sys_data_set_factory_reset(FALSE);
    sys_data_save(0);
    return er;
}

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/

static VACTION otaupg_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_ENTER:
    #if defined(NEW_MANUAL_OTA)|| defined(_NV_PROJECT_SUPPORT_ )
        if ((OTA_STEP_DOWNLOADING ==ota_proc_step)&& (ota_proc_ret != PROC_SUCCESS))
            act = VACT_PASS;
        else
            act = VACT_ENTER;
    #else
        act = VACT_ENTER;
    #endif
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT otaupg_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact=0;
    INT32 btn_state=0;
    LPVSCR apvscr=0;
    INT32 func_ret=0;
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
    //PRESULT obj_ret = PROC_PASS;
#endif
    BOOL func_flag=FALSE;

	if(FALSE == func_flag)
	{
		;
	}
	if(0 == func_ret)
	{
		;
	}
	#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
        TEXT_FIELD* txt_btn = &otaupg_txt_btn;
    #endif

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            /* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
            btn_state = win_otaup_get_btn_state();
            apvscr = osd_get_task_vscr(osal_task_get_current_id());
            switch(btn_state)
            {
            case 0:
                if(!ota_user_stop)
                {
                    func_flag=ota_stop_service();
                    ota_user_stop = 1;
                }
                break;
            case 1:
                ret = PROC_LEAVE;
                break;
            case 2:
                func_ret=win_otaupg_download();
                win_otaup_set_btn_msg_display(TRUE);
                osd_update_vscr(apvscr);
                break;
            case 3:
            {
            #if(!defined(_BUILD_OTA_E_) && !defined(_CAS9_CA_ENABLE_)  && !defined(NEW_MANUAL_OTA) \
            && !defined(_BC_CA_STD_ENABLE_) && !defined(_BC_CA_ENABLE_) && !defined(FTA_ONLY))

                win_popup_choice_t choice=WIN_POP_CHOICE_NO;
                UINT8 back_saved=0;

                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("Are you sure to burn the flash?", NULL, 0);
                win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                choice = win_compopup_open_ext(&back_saved);
                //win_compopup_smsg_restoreback();
                if(WIN_POP_CHOICE_YES == choice)
            #endif
                {
                    #if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                    osd_set_text_field_content (txt_btn, STRING_ID, RS_OTA_BURNFLASH);
                    osd_track_object( (POBJECT_HEAD)txt_btn, C_UPDATE_ALL);
                #endif


                    #ifdef FLASH_SOFTWARE_PROTECT 
					ap_set_flash_lock_len(OTA_LOADER_END_ADDR);
                    #endif
                    
                    if (NUM_ZERO != win_otaupg_burnflash())
                    {
                        ota_proc_ret = PROC_FAILURE;
                        ret = PROC_LEAVE;

                        #ifdef FLASH_SOFTWARE_PROTECT 
		                ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
                        #endif
                        break;
                    }

                    #ifdef FLASH_SOFTWARE_PROTECT 
		            ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
                    #endif
                    
                    win_otaup_set_btn_msg_display(TRUE);
                    osd_update_vscr(apvscr);
					#ifdef _BUILD_LOADER_COMBO_
					if((loader_check_osm_parameter()&0x01)==1)	
					{
		   				ap_send_key(V_KEY_ENTER, TRUE);
					}	
		 			loader_check_run_parameter();
		 			loader_set_run_parameter(0,0);
		 			loader_check_run_parameter();
					#endif
                }
                break;
            }
            case 4:
                func_ret=win_otaupg_reboot();
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    return ret;
}


static VACTION otaupg_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    case V_KEY_POWER:
        act = VACT_PASS + 1;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

//extern void ap_send_epg_PF_update();
//extern void ap_send_epg_sch_update();
//extern void ap_send_epg_detail_update();
//extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);

static PRESULT otaupg_callback(POBJECT_HEAD pobj,VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
#if defined(_S3281_) && defined(_USE_32M_MEM_) || defined(_M3383_SABBAT_)
    RET_CODE dmx_ret=RET_SUCCESS;
#endif
    INT32 func_ret=0;
    BOOL func_flag=FALSE;

	if(FALSE == func_flag)
	{
		;
	}
	if(0 == func_ret)
	{
		;
	}
    switch(event)
    {
    case EVN_PRE_OPEN:
        api_set_system_state(SYS_STATE_OTA_UPG);
        //p_otaosd_buff = MALLOC(OSD_VSRC_MEM_MAX_SIZE);
#ifndef _BUILD_OTA_E_
        func_ret=epg_release();
#endif
#ifdef OTA_BUF_SHARE_FB
        vdec_stop((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV), 1, 0);
#endif
        win_otaupg_set_memory();
#ifdef SUPPORT_CAS9
        #ifdef MULTI_DESCRAMBLE
        UINT8 i = 0;
        for(i = 0;i < 3;i++)  //stop filter all the TP
        {
            func_ret=api_mcas_stop_transponder_multi_des(i);
        }
        #else
            func_ret=api_mcas_stop_transponder();
        #endif
#endif
#if defined(SUPPORT_BC_STD)
        func_ret=api_mcas_stop_transponder();
        bc_cas_run_bc_task(FALSE);
#elif defined(SUPPORT_BC)
        UINT8 i = 0;
        for(i = 0; i < 3; i++)  //stop filter all the TP
        {
            func_ret=api_mcas_stop_transponder_multi_des(i);
        }
        bc_cas_run_bc_task(FALSE);
#endif

        wincom_open_title((POBJECT_HEAD)&g_win_otaupg,RS_TOOLS_SW_UPGRADE_BY_OTA, 0);
        func_ret=win_otaupg_get_info();
        win_otaup_set_btn_msg_display(FALSE);
        win_otaup_set_swinfo_display(FALSE);
        win_otaupg_init();
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        prev_ota_step=OTA_STEP_BURNFLASH+1;
#endif
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        if((OTA_STEP_BURNFLASH == ota_proc_step) && (ota_proc_ret != PROC_FAILURE))
        {
            ret = PROC_LOOP;
            break;
        }
        else
        {
            if(NUM_ZERO == ota_proc_ret)
            {
                func_flag=ota_stop_service();
                //ota_user_stop = 1;
            }
        }
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

        break;

    case EVN_POST_CLOSE:
        if(p_otaosd_buff != NULL)
        {
            osd_task_buffer_free(OSAL_INVALID_ID,p_otaosd_buff);
            //FREE(p_otaosd_buff);
            //p_otaosd_buff = NULL;
        }
#ifdef OTA_BUF_SHARE_FB
		api_show_menu_logo(); 
#endif
#ifndef _BUILD_OTA_E_
        func_ret=epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START/*buffer*/,\
     __MM_EPG_BUFFER_LEN, ap_epg_call_back);
#endif
#ifdef SUPPORT_CAS9
        #ifdef MULTI_DESCRAMBLE
        func_ret=api_mcas_start_transponder_multi_des(0);//ts_route.dmx_id-1);
        #else
        func_ret=api_mcas_start_transponder();
        #endif
#endif
#if defined(SUPPORT_BC_STD)
    func_ret=api_mcas_start_transponder();
    bc_cas_run_bc_task(TRUE);
#elif defined(SUPPORT_BC)
    func_ret=api_mcas_start_transponder_multi_des(0);
    bc_cas_run_bc_task(TRUE);
#endif
#if defined(_S3281_) && defined(_USE_32M_MEM_) || defined(_M3383_SABBAT_)
        func_ret=nim_open((struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0));
        dmx_ret=dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
        dmx_ret=dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#endif

        api_set_system_state(SYS_STATE_NORMAL);
        break;
    case EVN_UNKNOWN_ACTION:
        ret = PROC_LOOP;
        break;
    case EVN_MSG_GOT:
        ret = win_otaupg_message_proc(param1,param2);
        break;
    default:
        break;
    }
    return ret;
}


static PRESULT  win_otaupg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_PASS;
    UINT flgptn = 0;

    if(CTRL_MSG_SUBTYPE_STATUS_OTA_PROCESS_MSG == msg_type)
    {
        win_ota_upgrade_process_draw(msg_code);
    }

    if(NUM_ZERO==ota_proc_ret)
    {
        //osal_flag_wait(&flgptn,g_ota_flg_id,OTA_FLAG_PTN,OSAL_TWF_ORW,OTA_FLAG_TIMEOUT);
        if(E_FAILURE == osal_flag_wait(&flgptn,g_ota_flg_id, OTA_FLAG_PTN, OSAL_TWF_ORW,OTA_FLAG_TIMEOUT))
        {
            OTA_PRINTF("osal_flag_wait() failed!\n");
        }

        switch(ota_proc_step)
        {
        case OTA_STEP_GET_INFO:
        case OTA_STEP_DOWNLOADING:
            if(ota_user_stop &&  (flgptn & PROC_STOPPED ) )
            {
                ota_proc_ret = PROC_STOPPED;
            }

            if(!ota_user_stop && (flgptn & PROC_FAILURE) )
            {
                ota_proc_ret = PROC_FAILURE;
            }

            if(flgptn & PROC_SUCCESS)
            {
                if(ota_user_stop)
                {
                   ota_proc_ret = PROC_STOPPED;
                }
                else
                {
                   ota_proc_ret = PROC_SUCCESS;
                }
            }
            break;

        case OTA_STEP_BURNFLASH:
            if(flgptn & PROC_FAILURE)
            {
                ota_proc_ret = PROC_FAILURE;
            }
            if(flgptn & PROC_SUCCESS)
            {
                ota_proc_ret = PROC_SUCCESS;
            }
            break;
        default:
            break;
        }

        if(ota_proc_ret!= NUM_ZERO)
        {
            win_otaup_set_btn_msg_display(TRUE);
            if(OTA_STEP_GET_INFO == ota_proc_step)
            {
                win_otaup_set_swinfo_display(TRUE);
            }
            #if defined(NEW_MANUAL_OTA)|| defined(_NV_PROJECT_SUPPORT_ )
            if ((OTA_STEP_BURNFLASH==ota_proc_step ) && (PROC_SUCCESS==ota_proc_ret ))
            {
                osal_task_sleep(5000);
            }
            ap_send_key(V_KEY_ENTER, TRUE);
            #endif
			#ifdef _BUILD_LOADER_COMBO_
			if((loader_check_osm_parameter()&0x01)==1)
			{
		   		ap_send_key(V_KEY_ENTER, TRUE);
			}	
			#endif
        }
    }

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if(FALSE==api_osm_get_confirm())
    {
        UINT32 hkey = 0;

        if(prev_ota_step!=ota_proc_step)
        {
            g_otastart_time=osal_get_time();
            prev_ota_step=ota_proc_step;
            b_send_key=FALSE;
        }
        g_otacurrent_time=osal_get_time();

        if(OTA_STEP_GET_INFO == ota_proc_step)
        {
            if((g_otacurrent_time-g_otastart_time > OTA_STEP_TIMEOUT) &&(FALSE == b_send_key))
            {
                  libc_printf("[!] OTA_STEP_TIMEOUT(%d) occurs!!\n", OTA_STEP_TIMEOUT);
                ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);b_send_key=TRUE;
            }

            if((PROC_SUCCESS == ota_proc_ret) &&(FALSE == b_send_key))
            {
                if(g_otacurrent_time-g_otastart_time > OTA_SEND_KEY_TIME)
                {
                    libc_printf("-OTA_STEP_GET_INFO(PROC_SUCCESS)\n");
                    ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                    b_send_key=TRUE;
                }
            }
        }
        else if(OTA_STEP_DOWNLOADING == ota_proc_step)
        {
            if((PROC_SUCCESS == ota_proc_ret)&&(FALSE == b_send_key))
            {
                if(g_otacurrent_time-g_otastart_time > OTA_SEND_KEY_TIME)
                {
                    libc_printf("-OTA_STEP_DOWNLOADING(PROC_SUCCESS)\n");
                    ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                    b_send_key=TRUE;
                }
            }
            else if((PROC_FAILURE == ota_proc_ret)&&(FALSE == b_send_key))
            {
                if(FALSE == b_proc_failure_send_key)
                {
                    //ready for send exit key
                    libc_printf("-OTA_STEP_DOWNLOADING(PROC_FAILURE):ready for send exit key\n");
                    g_otastart_time=osal_get_time();
                    b_proc_failure_send_key=TRUE;
                }
                else
                {
                    if(g_otacurrent_time-g_otastart_time > OTA_SEND_KEY_TIME)
                    {
                        libc_printf("-OTA_STEP_DOWNLOADING(PROC_FAILURE)\n");
                        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                        b_send_key=TRUE;
                    }
                }
            }
        }
        else if(OTA_STEP_BURNFLASH == ota_proc_step)
        {
            if((PROC_SUCCESS == ota_proc_ret)&&(FALSE == b_send_key))
            {
                libc_printf("-OTA_STEP_BURNFLASH(PROC_SUCCESS)\n");
                ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                b_send_key=TRUE;
            }
            else if(( PROC_FAILURE == ota_proc_ret)&&(FALSE == b_send_key))
            {
                if(FALSE == b_proc_failure_send_key)
                {
                    //ready for send exit key
                    libc_printf("-OTA_STEP_BURNFLASH(PROC_FAILURE):ready for send exit key\n");
                    g_otastart_time=osal_get_time();
                    b_proc_failure_send_key=TRUE;
                }
                else
                {
                    if(g_otacurrent_time-g_otastart_time > OTA_SEND_KEY_TIME)
                    {
                        libc_printf("-OTA_STEP_BURNFLASH(PROC_SUCCESS)\n");
                        ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
                        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
                        b_send_key=TRUE;
                    }
                }
            }
        }
    }
#endif

    return ret;
}

#if 0
#if (defined(_BUILD_OTA_E_))
static UINT32 g_ota_addr = 0;
static UINT8 find_ota_loader()
{
    #define C_SECTOR_SIZE   0x10000

    UINT8 *buffer;
    UINT32 data_id;
    UINT32 data_len;
    UINT32 data_off;
    UINT32 data_crc;
    UINT32 crc;
    UINT32 pointer = 0;
    struct sto_device *flash_dev=NULL;

    flash_dev = (struct sto_device *)\
    dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if (NULL == flash_dev)
    {
        return 0;
    }

    while(pointer < flash_dev->totol_size)
    {
        data_id  = sto_fetch_long((UINT32)pointer + CHUNK_ID);
        data_len = sto_fetch_long((UINT32)pointer + CHUNK_LENGTH);
        data_off = sto_fetch_long((UINT32)pointer + CHUNK_OFFSET);
        data_crc = sto_fetch_long((UINT32)pointer + CHUNK_CRC);
    #ifdef _BC_CA_STD_ENABLE_
        //actually, no one invoke win_otaupg.c: find_ota_loader().
        if((data_len )&& (data_len < data_off)&& (CHUNKID_SECLOADER == (data_id & CHUNKID_SECLOADER_MASK)))
        {
            g_ota_addr = pointer;
            return 1;
        }
        #else
        if(data_len && (data_len < data_off) && (data_crc != NO_CRC) \
    && (CHUNKID_SECLOADER == (data_id & CHUNKID_SECLOADER_MASK)))
        {
            buffer = (UINT8 *)malloc(data_len);
            if(buffer != NULL)
            {
                sto_get_data(flash_dev, buffer, pointer + 16, data_len);
                crc = mg_table_driven_crc(0xFFFFFFFF, buffer, data_len);
                free(buffer);
                if(data_crc == crc)
                {
                    g_ota_addr = pointer;
                    return 1;
                }
            }
        }
        #endif
        pointer += C_SECTOR_SIZE;
    }
    return 0;
}
#endif
#endif




static INT32 win_otaupg_set_memory(void)
{

    #ifndef _USE_32M_MEM_
    #ifdef _M3383_SABBAT_
    //__MM_DMX_AVP_START_ADDR is low address.
    // followed by  __MM_PVR_VOB_BUFFER_ADDR.
    //because we use dmx buffer,so after downloading,we must stop dmx.
    m_ota_info.uncompressed_addr =__MM_DMX_AVP_START_ADDR;
    //= // __MM_PVR_VOB_BUFFER_ADDR + 0x250000;
    m_ota_info.uncompressed_len = 0x520000; //0x500000
    m_ota_info.compressed_addr = __MM_PVR_VOB_BUFFER_ADDR+0x400000;
    //m_ota_info.uncompressed_addr+m_ota_info.uncompressed_len;
    m_ota_info.compressed_len = 0x340000;//0x200000;
    m_ota_info.swap_addr = m_ota_info.compressed_addr + m_ota_info.compressed_len;
    m_ota_info.swap_len = 256*1024;
    m_ota_info.backup_db_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr+m_ota_info.swap_len;
    m_ota_info.backup_db_size = 0x200000;
    #elif defined(_USE_64M_MEM_)    
    m_ota_info.compressed_addr = __MM_PVR_VOB_BUFFER_ADDR;
    m_ota_info.compressed_len = 0x400000;//0x200000;
    m_ota_info.uncompressed_addr = m_ota_info.compressed_addr+ m_ota_info.compressed_len;
    m_ota_info.uncompressed_len = 0x600000; //0x500000
    m_ota_info.swap_addr = m_ota_info.uncompressed_addr+ m_ota_info.uncompressed_len;
    m_ota_info.swap_len = 256*1024;
    m_ota_info.backup_db_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr+m_ota_info.swap_len;
    m_ota_info.backup_db_size = 0x200000;
    #else
    m_ota_info.uncompressed_addr = __MM_PVR_VOB_BUFFER_ADDR;
    #ifdef _SUPPORT_64M_MEM
    m_ota_info.uncompressed_len = 0x700000;
    #else
	#if defined( _BUILD_LOADER_COMBO_) || defined(_OUC_LOADER_IN_FLASH_)
	m_ota_info.uncompressed_len = 0x800000;
	#else
    m_ota_info.uncompressed_len = 0x600000; //0x500000
    #endif
    #endif
    m_ota_info.compressed_addr = m_ota_info.uncompressed_addr+m_ota_info.uncompressed_len;
    #ifdef _SUPPORT_64M_MEM
    m_ota_info.compressed_len = 0x400000;
    #else
    #ifdef _OUC_LOADER_IN_FLASH_
    m_ota_info.compressed_len = 0x600000;//0x200000;
    #else
    m_ota_info.compressed_len = 0x500000;//0x200000;
    #endif
    #endif
    m_ota_info.swap_addr = m_ota_info.compressed_addr+ m_ota_info.compressed_len;
    m_ota_info.swap_len = 256*1024;
    m_ota_info.backup_db_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr+m_ota_info.swap_len;
    m_ota_info.backup_db_size = 0x200000;
    #endif
    #else
    m_ota_info.uncompressed_addr = __MM_NIM_BUFFER_ADDR;
    #ifdef _BC_CA_ENABLE_
    m_ota_info.uncompressed_len = 0x520000; //0x500000
    #else
    m_ota_info.uncompressed_len = 0x500000; //0x500000
    #endif
    m_ota_info.compressed_addr = m_ota_info.uncompressed_addr+m_ota_info.uncompressed_len;
    m_ota_info.compressed_len = 0x300000;//0x200000;
    m_ota_info.swap_addr = m_ota_info.compressed_addr + m_ota_info.compressed_len;
    m_ota_info.swap_len = 256*1024;
    m_ota_info.backup_db_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr+m_ota_info.swap_len;
    m_ota_info.backup_db_size = 0x200000;
    #endif

    p_otaosd_buff = (UINT8 *)((m_ota_info.backup_db_addr+m_ota_info.backup_db_size)|0xa0000000);

    ota_mem_config(
        m_ota_info.compressed_addr,
        m_ota_info.compressed_len,
        m_ota_info.uncompressed_addr,
        m_ota_info.uncompressed_len,
        m_ota_info.swap_addr,
        m_ota_info.swap_len);
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
        #ifdef _SECURE1_5_SUPPORT_
        //for Pure VMX SC1.5
        m_ota_info.tmp_buf_addr=0;
        m_ota_info.tmp_buf_size=0;
        //set decrypt buffer for decrypt api, such as aes_cbc_decrypt_flash_data        
        m_ota_info.cipher_buf_addr = m_ota_info.compressed_addr;
        //m_ota_info.swap_addr + m_ota_info.swap_len;
        m_ota_info.cipher_buf_size = 0x300000;
        m_ota_info.decrypted_data_addr = m_ota_info.cipher_buf_addr + m_ota_info.cipher_buf_size;
        m_ota_info.decrypted_data_size = 0x300000;
        flash_cipher_buf_init((UINT8 *)m_ota_info.cipher_buf_addr, m_ota_info.cipher_buf_size);
        #endif
        //for SMI
        m_ota_info.tmp_buf_addr=m_ota_info.compressed_addr;
        m_ota_info.tmp_buf_size=0x300000;        
#endif

#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
    #ifndef _USE_32M_MEM_
    #ifdef _M3383_SABBAT_
    // when start Decrypt/Signatur Test, compressed_addr could be re-used!!!
    m_ota_info.cipher_buf_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr + m_ota_info.swap_len;
    m_ota_info.cipher_buf_size = 0x1A0000;//=compressed_len/2
    m_ota_info.decrypted_data_addr = m_ota_info.cipher_buf_addr + m_ota_info.cipher_buf_size;
    m_ota_info.decrypted_data_size = 0x1A0000;
    flash_cipher_buf_init((UINT8 *)m_ota_info.cipher_buf_addr, m_ota_info.cipher_buf_size);
    #else
    // when start Decrypt/Signatur Test, compressed_addr could be re-used!!!
    m_ota_info.cipher_buf_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr + m_ota_info.swap_len;
    m_ota_info.cipher_buf_size = 0x200000;
    m_ota_info.decrypted_data_addr = m_ota_info.cipher_buf_addr + m_ota_info.cipher_buf_size;
    m_ota_info.decrypted_data_size = 0x200000;
    flash_cipher_buf_init((UINT8 *)m_ota_info.cipher_buf_addr, m_ota_info.cipher_buf_size);
    #endif
    #else
    // when start Decrypt/Signatur Test, compressed_addr could be re-used!!!
    m_ota_info.cipher_buf_addr = m_ota_info.compressed_addr;
    //m_ota_info.swap_addr + m_ota_info.swap_len;
    m_ota_info.cipher_buf_size = 0x150000;
    m_ota_info.decrypted_data_addr = m_ota_info.cipher_buf_addr + m_ota_info.cipher_buf_size;
    m_ota_info.decrypted_data_size = 0x150000;
    flash_cipher_buf_init((UINT8 *)m_ota_info.cipher_buf_addr, m_ota_info.cipher_buf_size);
    #endif
#endif
    return 0;
}


static INT32 win_otaupg_get_info(void)
{

    BOOL ota_get_info_ret=FALSE;

	if(FALSE == ota_get_info_ret)
	{
		;
	}
	ota_proc_ret= 0;
    ota_user_stop= 0;
    ota_proc_step= OTA_STEP_GET_INFO;
    MEMSET(&ota_dl_info,0,sizeof(struct dl_info));
    osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);
    ota_get_info_ret = ota_get_download_info(ota_pid,&ota_dl_info);


    return SUCCESS;
}

static INT32 win_otaupg_download(void)
{

    BOOL ota_download_ret=FALSE;

	if(FALSE == ota_download_ret)
	{
		;
	}
    ota_proc_ret = 0;
    ota_user_stop = 0;
    ota_proc_step = OTA_STEP_DOWNLOADING;

    osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);
    ota_download_ret=ota_start_download(ota_pid,win_ota_upgrade_process_update_ex);

    return SUCCESS;
}


static void win_ota_upgrade_process_draw(UINT32 process)
{
    PROGRESS_BAR *bar = &otaupg_progress_bar;
    TEXT_FIELD  *txt = &otaupg_progress_txt;
    TEXT_CONTENT tcont;
    MULTI_TEXT *mtxt_msg = &otaupg_mtxt_msg;
    UINT16 str_buf[50]={0};
    char    progress_strs[30]={0};
    LPVSCR apvscr = NULL;

    //PRESULT obj_ret=PROC_PASS;
    int sprintf_ret=0;
    UINT32 str_ret=0;

	if(0 == str_ret)
	{
		;
	}
	if(0 == sprintf_ret)
	{
		;
	}
	MEMSET(&tcont,0,sizeof(TEXT_CONTENT));
    MEMSET(&apvscr,0,sizeof(LPVSCR));
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
//      libc_printf("process:%d\n",process);
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


static void win_otaupg_init(void)
{
    PTEXT_FIELD ptxt = NULL;
    MEMSET(&ptxt,0,sizeof(PTEXT_FIELD));
    ptxt = &otaupg_progress_txt;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"0%");
    PROGRESS_BAR *bar = &otaupg_progress_bar;

    osd_set_progress_bar_pos(bar,0);
}

/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
INT32  win_otaup_get_btn_state(void)
{
    INT32 ret = 0;

    if(PROC_SUCCESS == ota_proc_ret)
    {
        ret = 0;
    }
    else if(PROC_STOPPED == ota_proc_ret)
    {
        ret = 1;
    }
    else if(PROC_FAILURE == ota_proc_ret)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    switch(ota_proc_step)
    {
    case OTA_STEP_GET_INFO:
        if(PROC_SUCCESS == ota_proc_ret)
        {
            ret = 2;
        }
        break;
    case OTA_STEP_DOWNLOADING:
        if(PROC_SUCCESS == ota_proc_ret)
        {
            ret = 3;
        }
        break;
    case OTA_STEP_BURNFLASH:
        if(PROC_SUCCESS == ota_proc_ret)
        {
            ret = 4;
        }
        else if(0 == ota_proc_ret)
        {
            ret = 3;//5;
        }
        break;
    default:
        break;
    }

    return ret;

}

UINT16 otaupg_get_btn_strid(INT32 btn_state)
{
    UINT16 btn_strid = 0;

    #ifdef NEW_MANUAL_OTA
    switch(btn_state)
    {
        case 0:
        case 1:
        case 2:
            btn_strid = RS_OTA_DOWNLOAD;
            break;
        case 3:
            btn_strid = RS_OTA_BURNFLASH;
            break;
        case 4:
            btn_strid = RS_OTA_UPGRADE_SUCCESSFUL;
            break;
        default:
            btn_strid = 0;
            break;
    }
#elif defined(_NV_PROJECT_SUPPORT_ )
    switch(btn_state)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            btn_strid = RS_DISPLAY_PLEASE_WAIT;
            break;
        case 4:
            btn_strid = RS_OTA_UPGRADE_SUCCESSFUL;
            break;
        default:
            btn_strid = 0;
            break;
    }
#else
    switch(btn_state)
    {
        case 0:
            btn_strid = RS_DISEQC12_MOVEMENT_STOP;
            break;
        case 1:
            btn_strid = RS_HELP_EXIT;
            break;
        case 2:
            #if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                btn_strid = RS_BC_OTA_PRESS_OK_TO_DOWNLOAD;
            #else
                btn_strid = RS_DISPLAY_DOWNLOADING;
            #endif
            break;
        case 3:
            #if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                btn_strid = RS_BC_OTA_PRESS_OK_TO_BURNFLASH;
            #else
                btn_strid = RS_OTA_BURNFLASH;
            #endif
            break;
        case 4:
            #if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                btn_strid = RS_BC_OTA_PRESS_OK_TO_REBOOT;
            #else
                btn_strid = RS_MSG_UPGRADE_REBOOT;
            #endif
            break;
        default:
            btn_strid = 0;
            break;
    }
    #endif
    return btn_strid;
}

#ifdef FLASH_SOFTWARE_PROTECT 
void flash_protect_onoff(UINT8 mode)
{
    struct sto_device *flash_dev = NULL;      
    UINT32 protect_len = 0;
    UINT32 db_chunk_id = 0x04FB0100;
    UINT32 db_addr = 0;
    UINT32 db_len = 0;
    INT32 ret = 0;
    
    flash_dev = ( struct sto_device * ) dev_get_by_type ( NULL, HLD_DEV_TYPE_STO );    				 
    if( flash_dev == NULL )
    {
        libc_printf( "Can't find FLASH device!\n" );
        return ;
    }

    if (api_get_chunk_add_len(db_chunk_id, &db_addr, &db_len)) 
    {
        protect_len = align_protect_len(db_addr); 
    }
    else
    {
        libc_printf("find DB failed!\n");
        protect_len = 0x10000;
    }    

    if(mode == 0)
    {
        ret = sto_unlock(flash_dev,0, protect_len);
        libc_printf("%s unlock [0 - %x], ret= %x\n", __FUNCTION__, protect_len,ret);
        while(ret <0 && protect_len)
        {
            protect_len = protect_len>>1;
            ret = sto_unlock(flash_dev,0, protect_len);
            libc_printf("%s unlock [0 - %x], ret= %x\n", __FUNCTION__, protect_len,ret);
        }
    }
    else
    {         
        ret = sto_lock(flash_dev,0, protect_len);
        libc_printf("%s lock [0 - %x], ret= %x\n", __FUNCTION__,protect_len,ret);
    }
}
#endif


#ifdef _GEN_CA_ENABLE_
static UINT8 sw_key[16];
static UINT8 sw_iv[32];

void set_key_iv(UINT8 *p_key,UINT8 *p_iv)
{
    if(p_key && p_iv)
    {
        MEMCPY(sw_key,p_key,16);
        MEMCPY(sw_iv,p_iv,32);
    }
}

static RET_CODE aes_decrypt_flash_data(UINT8 key_pos, UINT32 block_addr,
                            UINT32 block_len, UINT8 *out)
{
    UINT8 *temp_buffer = NULL;
    UINT8 *f_temp = NULL;
    RET_CODE ret = RET_FAILURE;
    struct sto_device *flash_dev = NULL;	

    OTA_PRINTF("enter %s (%x,%x,%x)\n", __FUNCTION__, key_pos, block_addr, block_len);

    if ((NULL == out) || (0 == block_addr) || (0 == block_len))
    {
        OTA_PRINTF("Invalid parameter!\n");
        return -1;
    }

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        OTA_PRINTF("Cannot find flash device!\n");
        return -1;
    }

    if(NULL == temp_buffer)
    {
        f_temp = (UINT8 *)MALLOC(block_len + 0xf);
        if (NULL == f_temp)
        {
            OTA_PRINTF("MALLOC Failed!\n");
            return -1;
        }
        temp_buffer = (UINT8 *)((0xFFFFFFF8 & (UINT32)f_temp));
    }
    ret = sto_get_data(flash_dev, temp_buffer, block_addr, block_len);
    if(block_len != (UINT32)ret)
    {
        OTA_PRINTF("%s get data from flash error! %x\n", __FUNCTION__, ret);
        if(f_temp != NULL)
        {
            FREE(f_temp);
            f_temp = NULL;
            temp_buffer = NULL;
        }
        return -1;
    }
    ret = aes_nbits_pure_cbc_crypt_keysram(sw_key, (UINT8 *)sw_iv, temp_buffer, out, block_len, DSC_DECRYPT, 128, RESIDUE_BLOCK_IS_NO_HANDLE);
	if(f_temp != NULL)
    {
        FREE(f_temp);
        f_temp = NULL;
        temp_buffer = NULL;
    }
    if(RET_SUCCESS != ret)
    {
        OTA_PRINTF("%s decrypt data error! %x\n", __FUNCTION__, ret);
        return -1;
    }
    OTA_PRINTF("exit %s \n", __FUNCTION__);
    return RET_SUCCESS;
}

RET_CODE aes_cbc_decrypt_chunk_with_host_key(UINT8 key_pos, UINT32 blockid,
                UINT8 *out, UINT32 *len)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 block_addr = 0;
    UINT32 block_len = 0;
    UINT32 enc_len = 0;

    OTA_PRINTF("enter %s (%x,%x,%x)\n", __FUNCTION__, key_pos, blockid, out);

    if ((NULL == out) || (0 == blockid )||(NULL == out) )
    {
        OTA_PRINTF("Invalid parameter!\n");
        return -1;
    }

    if (RET_SUCCESS != sto_get_chunk_len(blockid, &block_addr, &block_len))
    {
        OTA_PRINTF("Cannot find chunk id 0x%08x.\n", blockid);
        return -1;
    }

    enc_len = (block_len + SIG_ALIGN_LEN - 1)/SIG_ALIGN_LEN*SIG_ALIGN_LEN + SIG_LEN;
    ret = aes_decrypt_flash_data(key_pos, block_addr, enc_len, out);
    *len = enc_len;
    OTA_PRINTF("exit %s \n", __FUNCTION__);
    return ret;
}

RET_CODE aes_cbc_decrypt_ram_data_with_host_key(UINT8 key_pos, UINT8 *out,
                    UINT8 *data, UINT32 data_len)
{
    RET_CODE ret = RET_FAILURE;
    ret = aes_nbits_pure_cbc_crypt_keysram(sw_key, (UINT8 *)sw_iv, data, out, data_len, DSC_DECRYPT, 128, RESIDUE_BLOCK_IS_NO_HANDLE);
    return ret;
}

UINT32 get_ram_chunk_version_fk(UINT8* data,UINT8 key_pos,UINT8* tmp_buf)
{
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 ck_ver = 0;
    INT32 ret = 0;
    
    data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME; 
    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
    sig_len= enc_len + CHUNK_HEADER_SIZE;
    MEMCPY(tmp_buf,data,CHUNK_HEADER_SIZE);
    data += CHUNK_HEADER_SIZE;
    ret = aes_cbc_decrypt_ram_data_with_host_key(key_pos, &tmp_buf[CHUNK_HEADER_SIZE], data, enc_len);
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


BOOL otaupg_check_fixed_key(struct otaupg_ver_info *ver_info)
{
#ifdef  _BUILD_OTA_E_
    struct sto_device *flash_dev = NULL;
#endif
    UINT8 *addr = NULL ;
    UINT32 len = 0;
    UINT32 chunk_id = 0;
    UINT8 *data = NULL ;

    UINT8 key_pos = 0;
    int ret = 0;
    UINT32 boot_total_area_len = 0;
    __MAYBE_UNUSED__ UINT32 key_id = 0;

    get_boot_total_area_len(&boot_total_area_len);

    // get unzip size
    m_ota_info.uncompressed_len = (UINT32)otaupg_get_unzip_size((UINT8 *)m_ota_info.compressed_addr);

    chunk_init(m_ota_info.uncompressed_addr+boot_total_area_len, m_ota_info.uncompressed_len-boot_total_area_len);

    // resume chunk list
    ota_cmd_start_resume(m_ota_info.uncompressed_addr+boot_total_area_len,m_ota_info.uncompressed_len-boot_total_area_len);

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

    // get the versions of codes in flash
    addr = (UINT8 *)m_ota_info.decrypted_data_addr;    

    OTA_PRINTF("check main code in flash\n");
    len = 0;
    get_chunk_header_in_flash(MAIN_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk_with_host_key(key_pos, MAIN_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->m_f_ver = get_code_version(addr, len);
    }


#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("check ouc loader in flash\n");
    len = 0;
    UINT8 *temp_buffer = (UINT8 *)m_ota_info.cipher_buf_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {
        //UINT32 ouc_main_ver = 0;
        //UINT32 ouc_see_ver = 0;
        UINT8* ouc_main_addr = temp_buffer;
        UINT8* ouc_see_addr = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);        

        if ((*ouc_main_addr) + (*(ouc_main_addr+1)) == 0xFF)
        {
            ver_info->o_f_ver = get_ram_chunk_version_fk(ouc_main_addr,key_pos,addr);
        }
        if ((*ouc_see_addr) + (*(ouc_see_addr+1)) == 0xFF)
        {
            ver_info->os_f_ver = get_ram_chunk_version_fk(ouc_see_addr,key_pos,addr);
        }
    } 
#endif    

    OTA_PRINTF("check ota loader main code in memory\n");
    //35xx
    chunk_id = OTA_MAIN_CODE_CHUNK_ID;
    m_ota_info.ota_loader_size = m_ota_info.ota_bin_size;//for ota upgrade all size calculate
    m_ota_info.ota_main_code_addr = m_ota_info.ota_bin_addr;
    if (m_ota_info.ota_main_code_addr)
    {
        data = (UINT8 *)m_ota_info.ota_main_code_addr;
        m_ota_info.ota_main_code_size = fetch_long(data + CHUNK_OFFSET);

        ver_info->o_m_ver = get_ram_chunk_version_fk(data,key_pos,addr);

        #ifdef _OUC_LOADER_IN_FLASH_
        data = data + fetch_long(data + CHUNK_OFFSET);
        ver_info->os_m_ver = get_ram_chunk_version_fk(data,key_pos,addr);
        #endif
    }
    OTA_PRINTF("ota loader main code: 0x%X, 0x%X\n",\
     m_ota_info.ota_main_code_addr, m_ota_info.ota_main_code_size);
    
    OTA_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    m_ota_info.main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.main_code_addr)
    {
        data = (UINT8 *)m_ota_info.main_code_addr;
        m_ota_info.main_code_size = fetch_long(data + CHUNK_OFFSET);        
        ver_info->m_m_ver = get_ram_chunk_version_fk(data,key_pos,addr);
    }
    OTA_PRINTF("main code: 0x%X, 0x%X\n", m_ota_info.main_code_addr, m_ota_info.main_code_size);

    OTA_PRINTF("check see code in flash\n");
    len = 0;
    get_chunk_header_in_flash(SEE_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk_with_host_key(key_pos, SEE_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->s_f_ver = get_code_version(addr, len);
    }

    OTA_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    m_ota_info.see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (m_ota_info.see_code_addr)
    {
        data = (UINT8 *)m_ota_info.see_code_addr;
        m_ota_info.see_code_size = fetch_long(data + CHUNK_OFFSET); 
        ver_info->s_m_ver = get_ram_chunk_version_fk(data,key_pos,addr);
    }
    OTA_PRINTF("see code: 0x%X, 0x%X\n",\
    m_ota_info.see_code_addr, m_ota_info.see_code_size);


#ifdef _OUC_LOADER_IN_FLASH_
    OTA_PRINTF("version info: (0x%X,0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->os_f_ver,ver_info->o_m_ver,ver_info->os_m_ver,\
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#else
    OTA_PRINTF("version info: (0x%X, 0x%X, 0x%X), (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->o_f_ver, ver_info->o_m_ver,ver_info->o_b_ver, ver_info->m_f_ver,\
        ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#endif
    ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
    return TRUE;
}
#endif
#endif
