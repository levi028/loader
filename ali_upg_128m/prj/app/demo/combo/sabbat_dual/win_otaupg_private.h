/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg_private.h
*
*    Description:   The internal function of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#if 0//ndef _WIN_OTAUPG_PRIVATE_H_//new or change add by yuj
#define _WIN_OTAUPG_PRIVATE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define C_SECTOR_SIZE       0x10000
//#define OTA_PRINTF(...)
#define OTA_PRINTF  libc_printf
#define OTA_UPDATE_PROGRESS   100

#ifdef DUAL_ENABLE
#define SIGN_MAX_VAUE   256
#endif

#define OTA_FLAG_PTN    (PROC_SUCCESS | PROC_FAILURE | PROC_STOPPED)
#define OTA_FLAG_TIMEOUT    10

#ifdef    _BUILD_OTA_E_
#define WIN_SH_IDX          WSTL_WINSEARCH_03_HD
#else
#define WIN_SH_IDX          WSTL_WIN_BODYRIGHT_01_HD
#endif


#define MTXT_MSG_SH_IDX        WSTL_TEXT_08_HD
#define MTXT_SWINFO_SH_IDX    WSTL_TEXT_09_HD

#define MTXT_SH_IDX    WSTL_TEXT_08_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_HD
#define TXT_HL_IDX   WSTL_BUTTON_05_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_PERCENT_SH_IDX    WSTL_TEXT_09_HD

#ifndef SD_UI
#ifdef    _BUILD_OTA_E_
#define    W_W     692//493
#define    W_H     488//320
#define    W_L     GET_MID_L(W_W)//60
#define    W_T     98//GET_MID_T(W_H)//60
#define MTXT_MSG_L    (W_L + 30)

#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 60)
#define MTXT_MSG_H     140

#define BTN_L        (W_L + 200)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W        300
#define BTN_H        40

#define MTXT_SWINFO_L    MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     160

#define BAR_L         (W_L + 100)
#define BAR_T         (W_T + W_H - 80)
#define BAR_W         480
#define BAR_H         24

#define TXTP_L        (BAR_L + BAR_W)
#define TXTP_T         (BAR_T - 8)
#define TXTP_W         90
#define TXTP_H         40

#else

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define MTXT_MSG_L    (W_L + 20)
#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 40)
#define MTXT_MSG_H     140

#define BTN_L        (W_L + 120)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W        300
#define BTN_H        40

#define MTXT_SWINFO_L    MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     160

#define BAR_L         (W_L + 10)
#define BAR_T         (W_T + W_H - 80)
#define BAR_W         480
#define BAR_H         24

#define TXTP_L        (BAR_L + BAR_W)
#define TXTP_T         (BAR_T - 8)
#define TXTP_W         100//sharon 90
#define TXTP_H         40

#endif
#else
#ifdef    _BUILD_OTA_E_
#define    W_W     482//493
#define    W_H     360//320
#define    W_L     GET_MID_L(W_W)//60
#define    W_T     60//GET_MID_T(W_H)//60
#define MTXT_MSG_L    (W_L + 30)

#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 40)
#define MTXT_MSG_H     90

#define BTN_L        (W_L + 120)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W        250
#define BTN_H        30

#define MTXT_SWINFO_L    MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     150

#define BAR_L         (W_L + 30)
#define BAR_T         (W_T + W_H - 80)
#define BAR_W         320
#define BAR_H         16

#define TXTP_L        (BAR_L + BAR_W)
#define TXTP_T         (BAR_T - 8)
#define TXTP_W         90
#define TXTP_H         30

#else

#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define MTXT_MSG_L    (W_L + 20)
#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 40)
#define MTXT_MSG_H     90

#define BTN_L        (W_L + 120)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W        250
#define BTN_H        30

#define MTXT_SWINFO_L    MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     150

#define BAR_L         (W_L + 20)
#define BAR_T         (W_T + W_H - 80)
#define BAR_W         300
#define BAR_H         16

#define TXTP_L        (BAR_L + BAR_W)
#define TXTP_T         (BAR_T - 8)
#define TXTP_W         90//sharon 90
#define TXTP_H         30
#endif
#endif

#define B_BG_SH_IDX        WSTL_BARBG_01_HD
#define B_MID_SH_IDX    WSTL_NOSHOW_IDX
#define B_SH_IDX        WSTL_BAR_01_HD

#define LDEF_MTXT(root,var_mtxt,next_obj,l,t,w,h,sh,cnt,content)    \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,NULL,content)

#define LDEF_TXT_BTN(root,var_txt,next_obj,ID,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,next_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,idu,idd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    otaupg_btn_keymap,otaupg_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,res_id,NULL)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh)    \
    DEF_PROGRESSBAR(var_bar, &root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, \
    B_BG_SH_IDX, B_BG_SH_IDX, \
    B_BG_SH_IDX, B_BG_SH_IDX,\
        NULL, NULL, style, 0, 0, \
    B_MID_SH_IDX, B_SH_IDX, \
        rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_TXT_PROGRESS(root,var_txt,next_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,next_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_PERCENT_SH_IDX,\
    TXT_PERCENT_SH_IDX,TXT_PERCENT_SH_IDX,\
    TXT_PERCENT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    otaupg_keymap,otaupg_callback,  \
    nxt_obj, focus_id,0)

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#define     OTA_STEP_TIMEOUT    60
#define     OTA_SEND_KEY_TIME    2
#endif

#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
#define MAX_CHUNK_NUM        48
#define CHUNK_ARRAY_SIZE    (sizeof(CHUNK_HEADER) * MAX_CHUNK_NUM)
#define CODE_LENGTH_OFFSET                  0x200
// negative offset from the end of the data
#define CODE_VERSION_OFFSET                 0x10
// negative offset from the end of the code
#define CODE_VERSION_LEN                    8
#define CODE_VERSION_PADDED_LEN_MIN            0x0
#define CODE_VERSION_PADDED_LEN_MAX            (0x100-1)
#define CODE_APPENDED_LEN                    0x300

#define OTA_LOADER_CHUNK_ID        0x00FF0100
#define MAIN_CODE_CHUNK_ID        0x01FE0101
#define SEE_CODE_CHUNK_ID        0x06F90101

#define    DECRPT_KEY_ID            0x22DD0100
#define    DECRPT_KEY_ID_SEE        0x23DC0100
#define DECRPT_KEY_MASK         0xFFFF0000

#define OTA_LOADER_BACKUP_LEN   0xF0000
#define OTA_LOADER_BACKUP_ADDR  (0x800000 - OTA_LOADER_BACKUP_LEN)

#define DECRYPT_KEY_BACKUP_LEN  0x10000
#define DECRYPT_KEY_BACKUP_ADDR \
        (OTA_LOADER_BACKUP_ADDR - DECRYPT_KEY_BACKUP_LEN)

struct otaupg_ver_info
{
    UINT32 o_f_ver;    // ota loader in flash version
    UINT32 o_m_ver;    // ota loader in memory version
    UINT32 m_f_ver;    // main code in flash version
    UINT32 m_m_ver;    // main code in memory version
    UINT32 s_f_ver;    // see code in flash version
    UINT32 s_m_ver;    // see code in memory version
    UINT32 o_b_ver; // backup ota loader in flash version
#ifdef _CAS9_VSC_ENABLE_
    UINT8 vsc_f_ver [16]; //version of vsc code in flash
    UINT8 vsc_m_ver [16]; //version of vsc code in memory
#endif
#ifdef _OUC_LOADER_IN_FLASH_
    UINT32 os_f_ver;    // ouc loader see in flash version
    UINT32 os_m_ver;    // ouc loader see in memory version
#endif
#ifdef _MOD_DYNAMIC_LOAD_
    UINT32 dyn_f_ver;  //dyn code in flash version
    UINT32 dyn_m_ver;  //dyn code in memory version
#endif
};


#ifdef _M3503_
#define OTA_MAIN_CODE_CHUNK_ID    0x00FF0000
#define OTA_SEE_CODE_CHUNK_ID        0x07F80000
/* Calcuate InternalBufferSize for 7-zip */
#define LZMA_BASE_SIZE  1846
#define LZMA_LIT_SIZE   768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) \
                         * sizeof(UINT16))
#define SEE_CODE  1
#define MAIN_CODE 0
#else
//for other 
#define OTA_MAIN_CODE_CHUNK_ID    0x00FF0000
#define OTA_SEE_CODE_CHUNK_ID        0x07F80000
#endif /* _M3503_ */

#else /* _CAS9_CA_ENABLE_ */

#ifdef _BC_CA_STD_ENABLE_
struct otaupg_ver_info
{
    UINT32 o_f_ver;    // ota loader in flash version
    UINT32 o_m_ver;    // ota loader in memory version
    UINT32 m_f_ver;    // main code in flash version
    UINT32 m_m_ver;    // main code in memory version
    UINT32 s_f_ver;    // see code in flash version
    UINT32 s_m_ver;    // see code in memory version
};
#define SAVE_MAIN           0x1
#define SAVE_SEE            0x2
#define SAVE_OTA_LOADER     0x4

#else

#ifdef _BC_CA_ENABLE_
#ifdef _SMI_ENABLE_
struct otaupg_ver_info
{
	UINT32 o_f_ver;	// ota loader in flash version
	UINT32 o_m_ver;	// ota loader in memory version
	UINT32 o_m_f_ver;	// ota main code in flash version
	UINT32 o_m_m_ver;	// ota main code in memory version		
	UINT32 o_s_f_ver;	// ota see code in flash version
	UINT32 o_s_m_ver;	// ota see code in memory version	
	UINT32 s_bl_f_ver;	// see boot loader code in flash version
	UINT32 s_bl_m_ver;	// see boot loader code in memory version		
	UINT32 m_f_ver;	// main code in flash version
	UINT32 m_m_ver;	// main code in memory version
	UINT32 s_f_ver;	// see code in flash version
	UINT32 s_m_ver;	// see code in memory version
	UINT32 rsa_f_ver; // rsa key in flash version
	UINT32 rsa_m_ver; // rsa key in flash version
	UINT32 ecgk_f_ver; // ecgk in flash version
	UINT32 ecgk_m_ver; // ecgk in flash version
};

#define OTA_MAIN_CODE_CHUNK_ID		0x00FF0000
#define OTA_SEE_CODE_CHUNK_ID		0x07F80000
#define SEE_BL_CODE_CHUNK_ID		0x08F70101
#define MAIN_CODE_CHUNK_ID		0x01FE0101
#define SEE_CODE_CHUNK_ID		0x06F90101
#define RSA_KEY_CHUNK_ID		0x926D0100
#define ECGK_CHUNK_ID			0x926D0200

#else

#define OTA_MAIN_CODE_CHUNK_ID		0x00FF0000
#define OTA_SEE_CODE_CHUNK_ID		0x07F80000
#define CODE_VERSION_LEN                    8

struct otaupg_ver_info
{
    UINT32 o_f_ver;    // ota loader in flash version
    UINT32 o_m_ver;    // ota loader in memory version
    UINT32 m_f_ver;    // main code in flash version
    UINT32 m_m_ver;    // main code in memory version
    UINT32 s_f_ver;    // see code in flash version
    UINT32 s_m_ver;    // see code in memory version
#ifdef _SECURE1_5_SUPPORT_  
    UINT32 o_m_f_ver;	// ota main code in flash version
	UINT32 o_m_m_ver;	// ota main code in memory version		
	UINT32 o_s_f_ver;	// ota see code in flash version
	UINT32 o_s_m_ver;	// ota see code in memory version
#endif
};
#ifdef FAKE_VERITY_SIGNATURE
#define BC_VerifySignature  FakeBC_VerifySignature
#define MAGIC_CODE_LEN    8
#endif //FAKE_VERITY_SIGNATURE
#define MAIN_CODE_CHUNK_ID      0x01FE0101
#define SEE_CODE_CHUNK_ID       0x06F90101
#define SIGNATURE_SIZE      256
#define SAVE_MAIN           0x1
#define SAVE_SEE            0x2
#define SAVE_OTA_LOADER     0x4
#endif
#else
#ifdef _NV_PROJECT_SUPPORT_ 
struct otaupg_ver_info
{
    UINT32 o_f_ver;    // ota loader in flash version
    UINT32 o_m_ver;    // ota loader in memory version
    UINT32 m_f_ver;    // main code in flash version
    UINT32 m_m_ver;    // main code in memory version
    UINT32 s_f_ver;    // see code in flash version
    UINT32 s_m_ver;    // see code in memory version
    UINT32 o_b_ver; // backup ota loader in flash version
};
#endif //_NV_PROJECT_SUPPORT_ 
#endif //_BC_CA_ENABLE_
#endif //_BC_CA_STD_ENABLE_
#endif /* _CAS9_CA_ENABLE_ */


typedef struct
{
    UINT32 compressed_addr;
    UINT32 compressed_len;
    UINT32 uncompressed_addr;
    UINT32 uncompressed_len;
    UINT32 swap_addr;
    UINT32 swap_len;

    UINT32 user_db_start_sector;
    UINT32 user_db_addr;
    UINT32 user_db_addr_offset;
    UINT32 user_db_len;
    UINT32 user_db_sectors;

    UINT32 ota_bin_addr;
    // ota.bin address, write to userdb as second loader.
    UINT32 ota_bin_size;    // ota.bin size.
    UINT32 ota_bin_sectors;    // ota.bin sectors.
    UINT32 ota_bin_cfg_addr;

    UINT32 ota_fw_addr;        // received firmware address.
    UINT32 ota_fw_size;     // received firmware size.
    UINT32 ota_upg_addr;    // write to flash address.
    UINT32 ota_upg_size;     // write to flash size.
    UINT32 ota_upg_sectors; // write to flash sectors.

    UINT32 backup_db_addr;    // backup current database address
    UINT32 backup_db_size;    // backup current database size;
    UINT32 update_total_sectors;
    UINT32 update_secotrs_index;
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)

    UINT32 ota_main_code_addr;       
    UINT32 ota_main_code_size;
    UINT32 ota_see_code_addr;       
    UINT32 ota_see_code_size;	
    UINT32 see_bl_code_addr;
    UINT32 see_bl_code_size;
    UINT32 main_code_addr;
    UINT32 main_code_size;
    UINT32 see_code_addr;
    UINT32 see_code_size;
    UINT32 tmp_buf_addr;
    UINT32 tmp_buf_size;

    UINT32 cipher_buf_addr;
    UINT32 cipher_buf_size;
    UINT32 decrypted_data_addr;
    UINT32 decrypted_data_size; 
    UINT32 ota_loader_addr;
    UINT32 ota_loader_size;
#endif
#if defined(_SMI_ENABLE_)
	UINT32 rsa_key_addr;
	UINT32 rsa_key_size;
	UINT32 ecgk_key_addr;
	UINT32 ecgk_key_size;
    UINT32 backup_key_addr;
#endif
#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY || _NV_PROJECT_SUPPORT_ )
    UINT32 ota_loader_addr;
    UINT32 ota_loader_size;
//#ifdef _M3503_
    UINT32 ota_main_code_addr;
    UINT32 ota_main_code_size;
    UINT32 ota_see_code_addr;
    UINT32 ota_see_code_size;
//#endif
    UINT32 main_code_addr;
    UINT32 main_code_size;
    UINT32 see_code_addr;
    UINT32 see_code_size;

    UINT32 cipher_buf_addr;
    UINT32 cipher_buf_size;
    UINT32 decrypted_data_addr;
    UINT32 decrypted_data_size;
#endif
#ifdef _CAS9_VSC_ENABLE_
    UINT32 vsc_code_addr;
    UINT32 vsc_code_size;
#endif
#ifdef _OUC_LOADER_IN_FLASH_
    UINT32 ouc_para_f_offset;
    UINT32 ouc_para_m_offset;
#endif
#ifdef _MOD_DYNAMIC_LOAD_
    UINT32 dyn_code_addr;
    UINT32 dyn_code_size;
#endif
}OTA_INFO, *POTA_INFO;

extern CONTAINER g_win_otaupg;
extern MULTI_TEXT     otaupg_mtxt_msg;
extern MULTI_TEXT     otaupg_mtxt_swinfo;
extern TEXT_FIELD     otaupg_txt_btn;
extern PROGRESS_BAR otaupg_progress_bar;
extern TEXT_FIELD   otaupg_progress_txt;//
extern UINT32 ota_proc_ret;  /* PROC_SUCCESS / PROC_FAILURE / PROC_STOPPED */
extern UINT8  ota_user_stop;                    /* */
extern OTA_STEP ota_proc_step;                 /* */
extern UINT16 ci_flag;                                   /* */
extern struct dl_info ota_dl_info;                  /* */
extern OTA_INFO m_ota_info;                         /* */
extern struct sto_device *otaupg_sto_device; /* */
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
extern int backup_ota_type;     /* */
#endif
#ifdef _SMI_ENABLE_
extern UINT8 g_cust_public_key[516];
extern UINT8 g_ecgk_key[32];
extern UINT8 ca_switch_mode;
extern UINT64 main_vendor_id;
enum VENDOR_ID_NAME{
    VID_CHOOSE_SMI_VENDORID,
    VID_CHOOSE_VMX_VENDORID,
};
BOOL verify_code(UINT8 *addr, UINT32 len, UINT8 *public_key, UINT8 *ecgk);
BOOL verify_see_code(UINT8 *addr, UINT32 len, UINT8 *public_key);
BOOL bootrom_sig_enable(void);
void config_check_temp_buffer(UINT32 addr, UINT32 size);
void get_main_version_in_flash(UINT32 ck_addr,UINT32 *pversion);
void get_see_version_in_flash(UINT32 ck_addr,UINT32 *pversion);
BOOL get_main_code_in_mem(UINT8 *ck_addr,UINT32 *pversion);
BOOL get_see_code_in_mem(UINT8 *ck_addr,UINT32 *pversion);
void vendor_id_select(int vid);
#endif
void win_ota_upgrade_process_update_old(UINT32 process);
INT32  win_otaup_get_btn_state(void);
INT32 ota_upg_burn_block(UINT8 *pbuffer, UINT32 pos, UINT8 numsectors);
INT32 win_otaupg_reboot(void);
void win_otaup_set_swinfo_display(BOOL update);
void win_ota_upgrade_process_update_ex(UINT32 process);
#ifdef _NV_PROJECT_SUPPORT_ 
BOOL otaupg_check(void);
#elif (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_ENABLE_) \
    ||defined(_BC_CA_STD_ENABLE_) ||defined(FTA_ONLY)) 
BOOL otaupg_check(struct otaupg_ver_info *ver_info);
#endif
INT32 ota_upg_backup_database(void);
unsigned int otaupg_get_unzip_size(unsigned char *in);
UINT16 otaupg_get_btn_strid(INT32 btn_state);
INT32 win_otaupg_burnflash(void);
void win_otaup_set_btn_msg_display(BOOL update);

#ifdef _GEN_CA_ENABLE_
void set_key_iv(UINT8 *p_key,UINT8 *p_iv);
BOOL otaupg_check_fixed_key(struct otaupg_ver_info *ver_info);
#endif

extern UINT32 str2uint32_dec(UINT8 *str, UINT8 len);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif //#ifndef _WIN_OTAUPG_PRIVATE_H_

