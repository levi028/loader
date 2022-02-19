/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_usbupg.c
*
*    Description: The realize of USB upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#if 0//new or change add by yuj
#include <sys_config.h>
#include <sys_parameters.h>
#if defined (USB_MP_SUPPORT) || defined (_BUILD_LOADER_COMBO_)
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>

#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/usb_upgrade.h>
#include <hld/dsc/dsc.h>
#include <api/librsa/flash_cipher.h>
#include <hld/decv/decv.h>

#ifdef DISK_MANAGER_SUPPORT
#include <api/libfs2/statvfs.h>
#endif
#include <api/libfs2/unistd.h>
#include <boot/boot_common.h>
#include "osdobjs_def.h"
#include "win_com_popup.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "control.h"
#include "win_com.h"
#include "win_usbupg.h"

#ifdef _CAS9_CA_ENABLE_
#include <hld/crypto/crypto.h>
#include <api/libchunk/chunk_hdr.h>
#endif
#include <api/libc/fast_crc.h>

#ifdef _OUC_LOADER_IN_FLASH_
#include "win_ouc.h"
#endif

#include "menus_root.h" //by_blushark
//#include "win_com_menu_define.h"
#include <api/librsa/rsa_verify.h>

#ifdef _BC_CA_ENABLE_
#include <hld/crypto/crypto.h>
//#include <api/libcas/bc/bc_types.h>
#include "bc_ap/bc_cas.h"
#ifdef FAKE_VERITY_SIGNATURE
    #define BC_VerifySignature  FakeBC_VerifySignature
#endif
#ifdef _BC_CA_NEW_
#define _SIG_SEE                0x4     /**<Signature verification flag */
#define _DECRYPT_SEE            0x1     /**<Decryption flag */
#define _UZIP_SEE               0x2     /**<Unzip flag */
#define DUS_SEE    ((_DECRYPT_SEE <<28) | (_UZIP_SEE << 24)| \
                     (0xf <<20) | (_SIG_SEE <<16 ))
#define SEE_ENTRY               (__MM_PRIVATE_ADDR + 0x200)
static UINT8 see_rsa_pub_key[516]={0};
#define RSASSA_PKCS1_V1_5       0       
extern UINT32 str2uint32(UINT8 *str, UINT8 len);
extern BOOL loader_set_run_parameter(BOOL set,BOOL type);
extern BOOL loader_check_run_parameter();
extern RET_CODE generate_bc_see_key(struct UKEY_INFO *ukey);
extern RET_CODE get_see_root_pub_key(UINT8 *pk,UINT32 len);
extern RET_CODE test_cust_rsa_ram(const UINT32 addr, const UINT32 len, UINT8 *public_key);
extern void mg_setup_crc_table(void);
extern unsigned int mg_table_driven_crc(register unsigned int crc,
                register unsigned char *bufptr,
                register int len);
#endif
#endif

#ifdef _BC_CA_STD_ENABLE_
#include "bc_ap_std/bc_upg.h"
#endif

#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_block.h>
#include <udi/nv/ca_upg.h>
#include <udi/nv/ca_mmap.h>
#ifdef SSD_DEBUG
#include <udi/nv/ssassdd.h>
#else
#include <udi/nv/ssassd.h>
#endif
#endif
#if defined(_SMI_ENABLE_) || defined(_BC_CA_ENABLE_)
#include "win_otaupg.h"
#include "win_otaupg_private.h"
#endif
#include "power.h"
#include "key.h"
#include "win_otaupg.h"

//#define USBUPG_ONLY_ALLCODE
#define BACKUP_USB_LOADER

#if 0
#define UU_PRINTF(...)
#else
#define UU_PRINTF libc_printf
#define UU_DUMP(data, len) \
    do { \
        int i, l = (len); \
        for (i = 0; i < l; i++) \
            UU_PRINTF(" 0x%02x", *((data) + i)); \
        UU_PRINTF("\n"); \
    } while (0)
#endif
#define ZIP7_HEAD_EASY              0x8000006c
#define MAINCODE_ZIP_HEAD           ZIP7_HEAD_EASY
/*******************************************************************************
* Objects declaration
*******************************************************************************/
#define USBUPG_FILE_BUFFER_ADDR 0x80800000
#define USBUPG_CHUNK_NAME_LEN (16 +1)
#define USBUPG_MSG_MAX_LINE 5


static VACTION usbupg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT usbupg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usbupg_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT usbupg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usbupg_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT usbupg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

BOOL get_chunk_header_in_flash(UINT32 ck_id,UINT8* buf);

#if 0
#ifdef FLASH_SOFTWARE_PROTECT 
extern void flash_protect_onoff(UINT8 mode);
#endif
#endif

static UINT16 usbupg_blocks_cnt= 0;
static UINT16 usbupg_files_cnt= 0;
//static UINT16 *usbupg_msg_buf[USBUPG_MSG_MAX_LINE+1];
static UINT16 usbupg_msg_buf[USBUPG_MSG_MAX_LINE+1];
static UINT32 usbupg_msg_line= 0 ;

#ifdef _NV_PROJECT_SUPPORT_
static struct sto_device *m_StoDevice = NULL;
static UINT32 upg_total_sectors = 0;;
static UINT32 upg_secotrs_index = 0;;
static UINT32 upgrade_size = 0;;
static struct help_item_resource  usbupg_helpinfo[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};
#define HELP_CNT    (sizeof(usbupg_helpinfo)/sizeof(struct help_item_resource))
#endif

static PRESULT win_usbupg_init(void);
#if defined(_BUILD_USB_LOADER_)
#define WIN_SH_IDX      WSTL_WINSEARCH_03_HD
#else
#define WIN_SH_IDX   WSTL_WIN_BODYRIGHT_01_HD
#endif
#define CON_SH_IDX  WSTL_BUTTON_01_HD
#define CON_HL_IDX  WSTL_BUTTON_05_HD
#define CON_HL1_IDX WSTL_BUTTON_05_HD//WSTL_BUTTON_02
#define CON_SL_IDX  WSTL_BUTTON_01_HD
#define CON_GRY_IDX WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_HD //WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#define USBUPG_INFO_SH_IDX  WSTL_TEXT_09_HD


#define PROGRESSBAR_SH_IDX          WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX      WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX WSTL_BAR_04_HD

#define B_MID_SH_IDX                WSTL_NOSHOW_IDX

#define PROGRESS_TXT_SH_IDX WSTL_BUTTON_01_HD

#define RET_UPDATE_FAIL 1
#define RET_ERROR_HANDLER 2
#define ASCII_UPPERCASE_A  65
#define ASCII_UPPERCASE_F  70
#define ASCII_LOWERCASE_A  97
#define ASCII_LOWERCASE_F  102
#define ASCII_ZERO  0x30
#define ASCII_NINE  0x39

#undef BAR_L
#undef BAR_T
#undef BAR_W
#undef W_H

#ifdef SD_UI
#if defined( _BUILD_USB_LOADER_)
#define W_L     GET_MID_L(W_W)
#define W_T     60//80
#define W_W     482//500
#define W_H     380
#else
#define W_L         105//248//384
#define W_T         57//98//138
#define W_W         482//692
#define W_H         370//488
#endif
#define CON_L       (W_L+20)
#define CON_T       (W_T + 8)
#define CON_W       (W_W - 40)
#define CON_H       30//40
#define CON_GAP     8//12

#define TXTN_L_OF   10//4
#define TXTN_W      180//240//200
#define TXTN_H      30//40
#define TXTN_T_OF   ((CON_H - TXTN_H)/2)

#define TXTS_L_OF   (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W - TXTN_W - 20)
#define TXTS_H      30//40//28
#define TXTS_T_OF   ((CON_H - TXTS_H)/2)

#define MSG_SH_IDX WSTL_TEXT_09_HD//sharon WSTL_TEXT_04_HD

#define BAR_L   (CON_L+5)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*3+10)
#define BAR_W   (CON_W-BAR_TXT_W-10)
#define BAR_H   16//24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W   100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36//24

#define MSG_L   (CON_L+5)
#define MSG_T   (BAR_T + BAR_H+20)
#define MSG_W   (CON_W-10)
#define MSG_H   30//40
#define MSG_GAP 4

#define UPG_MODE_ID 1
#define UPG_FILE_ID 2
#define START_ID    3

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#else
#if defined(_BUILD_USB_LOADER_)
#define W_W     692//493
#define W_H     488//320
#define W_L     GET_MID_L(W_W)//60
#define W_T     98//GET_MID_T(W_H)//60
#else
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 60)
#define CON_H       40
#define CON_GAP     12

#define TXTN_L_OF   4
#define TXTN_W      240//200
#define TXTN_H      40
#define TXTN_T_OF   ((CON_H - TXTN_H)/2)

#define TXTS_L_OF   (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W - TXTN_W - 20)
#define TXTS_H      40//28
#define TXTS_T_OF   ((CON_H - TXTS_H)/2)

#define MSG_SH_IDX WSTL_TEXT_09_HD//sharon WSTL_TEXT_04_HD

#define BAR_L   (CON_L+5)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*3+14)
#define BAR_W   (CON_W-BAR_TXT_W-10)
#define BAR_H   24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W   100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36//24

#define MSG_L   (CON_L+5)
#define MSG_T   (BAR_T + BAR_H+20)
#define MSG_W   (CON_W-10)
#define MSG_H   40
#define MSG_GAP 4

#define UPG_MODE_ID 1
#define UPG_FILE_ID 2
#define START_ID    3

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#endif

#ifdef USBUPG_ONLY_ALLCODE
#define WINUSB_DFT_FOCUS_ID         START_ID
#define WINUSB_FILE_ID_UP           START_ID
#define WINUSB_START_ID_DOWN        UPG_FILE_ID
#else
#define WINUSB_DFT_FOCUS_ID         UPG_MODE_ID
#define WINUSB_FILE_ID_UP           UPG_MODE_ID
#define WINUSB_START_ID_DOWN        UPG_MODE_ID
#endif

#if defined(_BUILD_OUC_LOADER_) || defined(_OUC_LOADER_IN_FLASH_)
extern UINT32 ouc_get_param_f_addr(void);
#endif
extern UINT32 get_chunk_in_flash(UINT32 ck_id,UINT8 *buf);
extern UINT32 get_ram_chunk_version(UINT8* data,UINT8 key_pos,UINT8* tmp_buf);

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    usbupg_item_keymap,usbupg_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT1(root,var_txt,nxt_obj,l,t,w,h,res_id)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,CON_HL1_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    usbupg_item_sel_keymap,usbupg_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#undef LDEF_PROGRESS_BAR
#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, &root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, \
        NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
        rl,rt , rw, rh, 1, 100, 100, 0)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, USBUPG_INFO_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 5,0,0,display_strs[1])

#define LDEF_TXT_MSG(root,var_txt)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, \
    MSG_SH_IDX,MSG_SH_IDX,MSG_SH_IDX,MSG_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,NULL/*display_strs[0]*/)

#undef LDEF_WIN
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    usbupg_con_keymap,usbupg_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_INFOCON(var_con,nxt_obj,l,t,w,h)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,   \
    NULL,NULL,  \
    &nxt_obj, 0,0)


#define LDEF_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        hl,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,TXTS_H,style,cur,cnt,ptbl) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,\
                        hl,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT1(&var_con,var_txt,NULL/*&varLine*/,l,t + TXTN_T_OF,w,TXTN_H,res_id)    \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#if(defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_GEN_CA_ENABLE_) )
#define USB_LOADER_CHUNK_ID    0x00FF0000 
#define MAIN_CODE_CHUNK_ID      0x01FE0101
#define SEE_CODE_CHUNK_ID       0x06F90101
#define DECRPT_KEY_ID           0x22DD0100
#define DECRPT_KEY_MASK         0xFFFF0000
#define C_SECTOR_SIZE       0x10000
#endif
#ifdef _NV_PROJECT_SUPPORT_
#define C_SECTOR_SIZE       0x10000
#endif
#if (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_NV_PROJECT_SUPPORT_) || defined(_GEN_CA_ENABLE_))
#define BC_REENCRYPTION_DONE   2
#ifdef _BUILD_LOADER_COMBO_
#define READ_USBUPG_BUFFER_SIZE 0x780000	 
#else
#ifdef _OUC_LOADER_IN_FLASH_
#define READ_USBUPG_BUFFER_SIZE 0x780000	/* 10M */
#else
#ifdef _BUILD_USB_LOADER_
#define READ_USBUPG_BUFFER_SIZE 0x600000	/* 10M */
#else
#define READ_USBUPG_BUFFER_SIZE 0x520000
#endif
#endif
#endif
#define SIGNATURE_SIZE      256
#define SAVE_MAIN           0x1
#define SAVE_SEE            0x2
#define SAVE_USB_LOADER     0x4
#endif
#ifdef _SMI_ENABLE_
#define RSA_KEY_CHUNK_ID		0x926D0100
#define ECGK_CHUNK_ID			0x926D0200
#endif
#define INVALID_CK_ADDR 0xFFFFFFFF

//////////////////////////////////////////////////////

LDEF_ITEM_SEL(win_usbupg_con,usbupg_item_con1,&usbupg_item_con2,usbupg_txt_upgrade_type,usbupg_sel_upgrade_type,\
    usbupg_item_line0,1,3,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,CON_HL_IDX,RS_TOOLS_UPGRADE_MODE,\
    STRING_PROC,0,0,NULL)

LDEF_ITEM_SEL(win_usbupg_con,usbupg_item_con2,&usbupg_item_con3,usbupg_txt_file_select,usbupg_filelist_sel,\
    usbupg_item_line1,2,WINUSB_FILE_ID_UP,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,CON_HL_IDX,\
    RS_USBUPG_UPGRADE_FILE,STRING_PROC,0,0,NULL)

#ifdef _NV_PROJECT_SUPPORT_
LDEF_ITEM_TXT(win_usbupg_con,usbupg_item_con3,&usbupg_info_con,usbupg_txt_start,usbupg_item_line2,\
    3,2,WINUSB_START_ID_DOWN,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL1_IDX,RS_COMMON_OK)
#else
LDEF_ITEM_TXT(win_usbupg_con,usbupg_item_con3,&usbupg_info_con,usbupg_txt_start,usbupg_item_line2,\
    3,2,WINUSB_START_ID_DOWN,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL1_IDX,RS_COMMON_START)
#endif

LDEF_INFOCON(usbupg_info_con, usbupg_bar_progress,CON_L,CON_T + (CON_H + CON_GAP)*3, CON_W, 196)

LDEF_PROGRESS_BAR(usbupg_info_con,usbupg_bar_progress,&usbupg_txt_progress, \
    BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W,BAR_H-10)

LDEF_PROGRESS_TXT(usbupg_info_con, usbupg_txt_progress, NULL, \
        BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT_MSG(usbupg_info_con,usbupg_txt_msg)

LDEF_WIN(win_usbupg_con,&usbupg_item_con1,W_L,W_T,W_W,W_H,WINUSB_DFT_FOCUS_ID)

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/


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
	
	UINT32 usb_bin_addr;	// usb.bin address, write to userdb as second loader.
	UINT32 usb_bin_size;	// usb.bin size.
	UINT32 usb_bin_sectors;	// usb.bin sectors.
	UINT32 usb_bin_cfg_addr;
	
	UINT32 usb_fw_addr;		// received firmware address.
	UINT32 usb_fw_size; 	// received firmware size.	
	UINT32 usb_upg_addr;	// write to flash address.
	UINT32 usb_upg_size; 	// write to flash size.	
	UINT32 usb_upg_sectors; // write to flash sectors.

	UINT32 backup_db_addr;	// backup current database address
	UINT32 backup_db_size;	// backup current database size;
	UINT32 update_total_sectors;
	UINT32 update_secotrs_index;
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
	UINT32 main_code_addr;
	UINT32 main_code_size;
	UINT32 see_code_addr;
	UINT32 see_code_size;
	UINT32 tmp_buf_addr;
	UINT32 tmp_buf_size;	
#endif
#if (defined _CAS9_CA_ENABLE_ || defined FTA_ONLY)
	UINT32 ota_loader_addr;
	UINT32 ota_loader_size;
	UINT32 usb_loader_addr;
	UINT32 usb_loader_size;
#ifdef _M3503_
	UINT32 ota_main_code_addr;
	UINT32 ota_main_code_size;
	UINT32 ota_see_code_addr;
	UINT32 ota_see_code_size;
#endif
	UINT32 main_code_addr;
	UINT32 main_code_size;
	UINT32 see_code_addr;
	UINT32 see_code_size;
	
	UINT32 cipher_buf_addr;
	UINT32 cipher_buf_size;	
	UINT32 decrypted_data_addr;
	UINT32 decrypted_data_size;
#endif
}USB_INFO;

#ifndef _BUILD_LOADER_COMBO_
static USB_INFO m_usb_info __MAYBE_UNUSED__;
#endif

#if defined(_GEN_CA_ENABLE_)
    extern void set_key_iv(UINT8 *p_key,UINT8 *p_iv);
    extern RET_CODE aes_cbc_decrypt_chunk_with_host_key(UINT8 key_pos, UINT32 blockid,
                UINT8 *out, UINT32 *len);
    extern UINT32 get_ram_chunk_version_fk(UINT8* data,UINT8 key_pos,UINT8* tmp_buf);
#endif
static void win_usbupg_process_update(UINT32 process)
{

    if(process > PERCENT_MAX)
    {
        process = 0;
    }
    osd_set_progress_bar_pos(&usbupg_bar_progress,(INT16)process);
    osd_set_text_field_content(&usbupg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);

    osd_draw_object( (POBJECT_HEAD)&usbupg_bar_progress, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&usbupg_txt_progress, C_UPDATE_ALL);

#ifdef WATCH_DOG_SUPPORT
    dog_set_time(0, 0);
#endif

}

static void win_usbupg_msg_update(UINT16 str_id)
{
    UINT32 i= 0;
    OSD_RECT rect;

    usbupg_msg_buf[usbupg_msg_line % USBUPG_MSG_MAX_LINE] = str_id;

        //draw msg infos
    for(i=0; i<USBUPG_MSG_MAX_LINE; i++)
    {
        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&usbupg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&usbupg_txt_msg, STRING_ID, (UINT32)usbupg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&usbupg_txt_msg, C_UPDATE_ALL);
    }

    usbupg_msg_line++;
}

static void win_usbupg_msg_clear(void)
{
    UINT8 i= 0;
    OSD_RECT rect;

    for(i=0;i<USBUPG_MSG_MAX_LINE;i++)
    {
        usbupg_msg_buf[i] = 0;//display_strs[10+i];

        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&usbupg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&usbupg_txt_msg, STRING_ID, (UINT32)usbupg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&usbupg_txt_msg, C_UPDATE_ALL);
    }

    usbupg_msg_line = 0;
}

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION usbupg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;

}

static UINT32 string_to_lower(UINT8 *str)
{
    UINT32 i= 0;
    UINT32 len= 0;

    if ( NULL == str)
    {
        return 0;
    }

    len = strlen((const char *)str);

    for (i = 0; i<len; i++)
    {
        if ((str[i] >= 'A') && (str[i] <= 'Z'))
        {
            str[i] += 'a' - 'A';
        }
    }
    return i;
}

static PRESULT usbupg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT8 bid= 0;
    UINT16 sel= 0;
    UINT16 block_idx= 0;
    UINT16 *uni_str= 0;
    UINT32 block_len= 0;
    char str_buf[20] = {0};
    char str_extend[4] = {0};
    char str_filter[4] = {0};
    PRESULT ret = PROC_PASS;
    USBUPG_FILENODE filenode;
    MULTISEL *pmsel = NULL;

    MEMSET(&filenode, 0x0, sizeof(USBUPG_FILENODE));
    bid = osd_get_obj_id(pobj);
    switch(event)
    {
        case EVN_REQUEST_STRING:
            sel = param1;
            uni_str= (UINT16*)param2; // NOTICE: uni_str size is 32 (#define MAX_STR_LEN 32) in obj_multisel.c

            switch(bid)
            {
                case UPG_MODE_ID:
                    switch(sel)
                    {
                        case 0:
                            block_idx = 0;
                            break;
                        default:
                            block_idx = sel + 1;
                            break;
                    }

#ifdef _BUILD_UPG_LOADER_
                    sprintf(str_buf,"%s","AllCode");
#else
                    usbupg_get_blockinfo(block_idx,str_buf,&block_len);
#endif
                    com_asc_str2uni((UINT8 *)str_buf, uni_str);
                    break;
                case UPG_FILE_ID:
                    if(0 == usbupg_files_cnt)
                    {
                        pmsel = &usbupg_sel_upgrade_type;
                        switch(osd_get_multisel_sel(pmsel))
                        {
                            case 0:
                                block_idx = 0;
                                break;
                            default:
                                block_idx = osd_get_multisel_sel(pmsel)+ 1;
                                break;
                        }
                        usbupg_get_file_filter(block_idx, (UINT8 *)str_extend, (UINT8 *)str_filter);
                        string_to_lower((UINT8 *)str_extend);
#ifdef USBUPG_UPGFILE_FILTER
                        string_to_lower((UINT8 *)str_filter);
#else
                        str_filter[0] = 0;
#endif
                        snprintf(str_buf, 20, "No File(%s*.%s)", str_filter, str_extend);
                        com_asc_str2uni((UINT8 *)str_buf, uni_str);
                    }
                    else
                    {
                        usbupg_get_filenode(&filenode,sel);
                        filenode.upg_file_name[31] = 0;
                        com_asc_str2uni((UINT8 *)filenode.upg_file_name, uni_str);
                    }
                    break;
                default:
                    uni_str[0] = 0;//ComAscStr2Uni("", uni_str);
                    break;
            }
            break;
        case EVN_POST_CHANGE:
            if(1 == bid )
            {
                sel = param1;
                switch(sel)
                {
                    case 0:
                        block_idx = 0;
                        break;
                    default:
                        block_idx = sel + 1;
                        break;
                }
                usbupg_create_filelist(block_idx,&usbupg_files_cnt);

                pmsel = &usbupg_filelist_sel;
                osd_set_multisel_sel(pmsel, 0);
                osd_set_multisel_count(pmsel, (usbupg_files_cnt));
                osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
            }
            break;
        default:
            break;
    }
    return ret;
}

static VACTION usbupg_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

#if defined(_CAS9_CA_ENABLE_) || defined(_GEN_CA_ENABLE_)
static UINT32 upg_total_sectors= 0;
static UINT32 upg_secotrs_index= 0;
static UINT32 main_code_addr= 0;
static UINT32 main_code_size= 0;
static UINT32 see_code_addr= 0;
static UINT32 see_code_size= 0;
static UINT32 usb_loader_code_addr;
static UINT32 usb_loader_code_size;
#ifdef _MOD_DYNAMIC_LOAD_
static UINT32 dyn_code_addr;
static UINT32 dyn_code_size;
#endif

#ifdef _OUC_LOADER_IN_FLASH_ 
static UINT32 usb_loader_f_code_addr;
static UINT32 usb_loader_f_code_size;
static UINT32 usb_loader_main_code_addr = 0;
static UINT32 usb_loader_see_code_addr = 0;
static UINT32 usb_loader_f_param_addr = 0;;
static UINT32 usb_loader_param_addr = 0;
#endif
static UINT32 boot_len = 0;
static UINT32 usb_loader_m_main_offset = 0;

static UINT32 usb_loader_main_code_size;
static UINT32 usb_loader_see_code_size;
#ifndef _OUC_LOADER_IN_FLASH_ 
static UINT32 usb_loader_parameter_size;
static UINT32 userdb_code_addr;
static UINT32 userdb_code_size;
#endif
static UINT32 usb_loader_bin_size __MAYBE_UNUSED__;
static UINT32 usb_loader_bin_sectors __MAYBE_UNUSED__;
static UINT32 backup_db_addr __MAYBE_UNUSED__;
static UINT32 backup_db_size __MAYBE_UNUSED__;
static UINT32 user_db_start_sector __MAYBE_UNUSED__;
static UINT32 user_db_sectors __MAYBE_UNUSED__;
static UINT32 user_db_addr_offset __MAYBE_UNUSED__;
static UINT32 user_db_len __MAYBE_UNUSED__;
static UINT32 usb_tmp_buf_addr __MAYBE_UNUSED__;
static UINT32 usb_tmp_buf_size __MAYBE_UNUSED__;
static struct sto_device *m_sto_device = NULL;
struct usbupg_ver_info
{
    UINT32 u_f_ver;    // usb loader in flash version
    UINT32 u_m_ver;    // usb loader in memory version
    UINT32 m_f_ver;    // main code in flash version
    UINT32 m_m_ver;    // main code in memory version
    UINT32 s_f_ver;    // see code in flash version
    UINT32 s_m_ver;    // see code in memory version
    UINT32 u_b_ver; // backup ota loader in flash version
#ifdef _OUC_LOADER_IN_FLASH_ 
    UINT32 us_f_ver;   // usb loader (see) in flash version
    UINT32 us_m_ver;   // usb loader (see) in memory version
#endif
#ifdef _MOD_DYNAMIC_LOAD_
    UINT32 dyn_f_ver;  //dyn code in flash version
    UINT32 dyn_m_ver;  //dyn code in memory version
#endif
};

static INT32 win_usbupg_set_memory(void)
{
	UINT32 addr;
	UINT32 len;
			
	m_usb_info.uncompressed_addr = __MM_PVR_VOB_BUFFER_ADDR;
#ifdef _SUPPORT_64M_MEM
    m_usb_info.uncompressed_len = 0x700000;//0x600000;
#else
	m_usb_info.uncompressed_len = 0x1000000;//0x600000;
#endif	
	m_usb_info.compressed_addr = m_usb_info.uncompressed_addr+m_usb_info.uncompressed_len;
#ifdef _SUPPORT_64M_MEM
    m_usb_info.compressed_len = 0x400000;//0x200000;
#else
	m_usb_info.compressed_len = 0x800000;//0x200000;
#endif	
	m_usb_info.swap_addr = m_usb_info.compressed_addr + m_usb_info.compressed_len;
	m_usb_info.swap_len = 256*1024;
	m_usb_info.backup_db_addr = m_usb_info.compressed_addr;//m_ota_info.swap_addr+m_ota_info.swap_len;
	m_usb_info.backup_db_size = 0x200000;

	// user db address
	if(!api_get_chunk_add_len(USER_DB_ID, &addr, &len))
		return FALSE;
	m_usb_info.user_db_addr_offset = addr;//addr-m_usb_info.uncompressed_addr;
	m_usb_info.user_db_len = len;
	m_usb_info.user_db_start_sector = m_usb_info.user_db_addr_offset/C_SECTOR_SIZE;
	m_usb_info.user_db_sectors = m_usb_info.user_db_len/C_SECTOR_SIZE;

	if(m_usb_info.user_db_len%C_SECTOR_SIZE)
	{
	//	ASSERT(0);
		m_usb_info.user_db_sectors++;
	}
	
	// find the firmware position	
	m_usb_info.usb_fw_addr = m_usb_info.uncompressed_addr;
	m_usb_info.usb_fw_size = addr + len - m_usb_info.usb_fw_addr;
	//libc_printf("ota_fw_size=%x \n",m_ota_info.ota_fw_size);	
	// find ota.bin position.
	m_usb_info.usb_bin_addr = m_usb_info.usb_fw_addr + m_usb_info.usb_fw_size;
	m_usb_info.usb_bin_size = m_usb_info.uncompressed_len - m_usb_info.usb_fw_size;
	m_usb_info.usb_bin_sectors = m_usb_info.usb_bin_size/C_SECTOR_SIZE;
	if(m_usb_info.usb_bin_size % C_SECTOR_SIZE)
		m_usb_info.usb_bin_sectors++;

    return TRUE;
}


static INT32 init_storage(void)
{

    m_sto_device = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    INT32 er = sto_open(m_sto_device);

    if(NULL == m_sto_device)
    {
        UU_PRINTF("%s: can not get sto device\n", __FUNCTION__);
        return ERR_NO_DEV;
    }

    if(SUCCESS != er)
    {
        UU_PRINTF("%s: can not open sto device\n", __FUNCTION__);
    }
    return er;
}

#ifdef _OUC_LOADER_IN_FLASH_
BOOL save_upgrade_param(UINT8* buf,UINT32 buf_len)
{
    BOOL ret = FALSE;
    UINT32 chunk_id = OUC_CHUNK_ID;
    UINT8* p = NULL;
    OUC_CFG ouc_cfg;
    UINT32 crc=0;
    UINT32 len=0;
    #ifdef _CAS9_CA_ENABLE_
    UINT32 verify_len = 0;   
    #endif
    MEMSET(&ouc_cfg,0,sizeof(ouc_cfg));
    ouc_cfg.magic = OUC_MAGIC ;
    ouc_cfg.upgrade_mode = OUC_TYPE_USB;    
    
    if(usb_loader_code_addr)
    {
        chunk_id = CHUNKID_OTA_PARA;
        //chunk_init(usb_loader_code_addr,usb_loader_code_size);
        chunk_init((UINT32)buf, buf_len);       //flash protection
        p = (UINT8*)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if(p)
        {            
            MEMCPY(p+CHUNK_HEADER_SIZE+OUC_CFG_OFFSET, &ouc_cfg, sizeof(ouc_cfg));
    
            //vicky_OTA_PARA
            #ifdef _CAS9_CA_ENABLE_
            if(p)
            {
                verify_len = OUC_CFG_OFFSET + sizeof(ouc_cfg);
                update_otapara_hmac(p+CHUNK_HEADER_SIZE,verify_len);
            }      
            #endif
    
            mg_setup_crc_table();
            len = fetch_long(p + CHUNK_LENGTH);
            crc = (UINT32)mg_table_driven_crc(0xFFFFFFFF, p + CHUNK_NAME, len);
            store_long((UINT8*)(p + CHUNK_CRC), crc);

            usb_loader_param_addr = (UINT32)p;
           // usb_loader_f_param_addr = ouc_get_param_f_addr();
             usb_loader_f_param_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE; 
            if(0 == usb_loader_f_param_addr)
            {
                usb_loader_f_param_addr = p - buf;
                usb_loader_f_param_addr += boot_len;
            }
            
            ret = TRUE;
        }
    }
    return ret;    
}
#endif

#ifdef _CAS9_CA_ENABLE_
BOOL save_otaloader_hmac(void)
{
    BOOL bret = FALSE;
    //INT32 ret = 0;
    
    #if 1
    bret = TRUE;
    #else
    if(usb_loader_code_addr)
    {
        chunk_init(usb_loader_code_addr,usb_loader_code_size);
        ret = update_otaloader_hmac((UINT8 *)usb_loader_code_addr,usb_loader_code_size);
        if(ret == RET_SUCCESS)
        {
            bret= TRUE;
        }
    }
    #endif
    return bret;
}
#endif

//cas9
static BOOL usbupg_check(struct usbupg_ver_info *ver_info,UINT8 *pbuff, UINT32 buf_len)
{
    UINT8 *cipher_addr= 0;
    UINT32 cipher_len= 0;
    UINT8 *addr= 0;
    UINT32 len= 0;
    UINT32 chunk_id = 0;
    UINT8 *data= NULL;
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 * block_addr __MAYBE_UNUSED__ =NULL ;
    UINT8 key_pos= 0;
    //UINT32 key_id = 0;
    int ret= 0;
    CHUNK_HEADER chunk_header __MAYBE_UNUSED__;
 #ifdef _OUC_LOADER_IN_FLASH_ 
    UINT32 loader_f_addr = 0 ;
    UINT32 loader_f_size = 0 ;
    UINT8 *temp_buffer = NULL;
 #endif
    UINT8 rsa_key_id __MAYBE_UNUSED__ = 0;

    if ((NULL == pbuff) || (0 == buf_len))
    {
        return FALSE;
    }
    //init chunk
    chunk_init((UINT32)pbuff, buf_len);

//////////////////////////MAIN PART///////////////////////////////    
    // decrypt universal key, fetch public key from flash    
    if (decrypt_universal_key_wrapped(&key_pos,E_KEY_MAIN) < 0)
    {
        UU_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // fetch main rsa key id
    rsa_key_id = get_rsa_key_id_by_type(E_KEY_MAIN);

    // get the versions of codes in flash
    cipher_addr = pbuff + buf_len;
    cipher_len = 0x200000;
    flash_cipher_buf_init(cipher_addr, cipher_len);

    addr = cipher_addr + cipher_len;

    UU_PRINTF("check main code in flash\n");
    len = 0;
    get_chunk_header_in_flash(MAIN_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk(key_pos, MAIN_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->m_f_ver = get_code_version(addr, len);
    }
   
#ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check ouc loader main in flash\n");
    len = 0;
    temp_buffer = cipher_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {        
        UINT8* ouc_main_addr = temp_buffer;        
        ver_info->u_f_ver = get_ram_chunk_version(ouc_main_addr,key_pos,addr);            
    } 
#endif 
	
    UU_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    //UU_PRINTF("main addr= %x\n", main_code_addr);	
    main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("main addr= %x\n", main_code_addr);
    if (main_code_addr)
    {
        usb_loader_m_main_offset = main_code_addr - (UINT32)pbuff + boot_len;
        data = (UINT8 *)main_code_addr;
        main_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != enc_len))
            ver_info->m_m_ver = get_code_version(addr, sig_len);
    }
    
#ifdef _OUC_LOADER_IN_FLASH_ 
    UU_PRINTF("check ouc loader main in memory\n");
    chunk_id = OUC_CHUNK_ID;
    usb_loader_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    usb_loader_code_size = fetch_long((unsigned char *)usb_loader_code_addr + CHUNK_OFFSET);
    loader_f_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((((INT32)loader_f_addr) < 0) || (usb_loader_code_size >= buf_len) )
    {
        UU_PRINTF("err,loader_f_addr = 0x%08x,usb_loader_code_size = 0x%08x\n",\
            loader_f_addr,usb_loader_code_size);
        return FALSE;
    }

    loader_f_size = sto_fetch_long(loader_f_addr + CHUNK_OFFSET);
    if(usb_loader_code_size > loader_f_size)
    {
        UU_PRINTF("err,loader_f_size = 0x%08x,usb_loader_code_size = 0x%08x\n",\
            loader_f_size,usb_loader_code_size);
        return FALSE;
    }

    usb_loader_code_addr = usb_loader_code_addr + CHUNK_HEADER_SIZE;
    usb_loader_code_size = usb_loader_code_size - CHUNK_HEADER_SIZE;
    usb_loader_f_code_addr = loader_f_addr + CHUNK_HEADER_SIZE;
    usb_loader_f_code_size = loader_f_size - CHUNK_HEADER_SIZE;

    usb_loader_main_code_addr = usb_loader_code_addr ;
    usb_loader_main_code_size = fetch_long((unsigned char *)usb_loader_main_code_addr + CHUNK_OFFSET);
    if(usb_loader_main_code_size < buf_len)
    {
        ver_info->u_m_ver = get_ram_chunk_version((UINT8 *)usb_loader_main_code_addr,key_pos,addr);
    }
    else
    {
        ver_info->u_m_ver= 0;
    }   
#else
    UU_PRINTF("check otaloader in memory\n");
    //caculate usb loader addr
    chunk_id = 0x04FB0100;
    userdb_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("userdb addr= %x\n", userdb_code_addr);
    data = (UINT8 *)userdb_code_addr;
    userdb_code_size = fetch_long(data + CHUNK_LENGTH) + CHUNK_NAME;
    UU_PRINTF("userdb size= %x\n", userdb_code_size);
 
    usb_loader_code_addr = userdb_code_addr + userdb_code_size;
    UU_PRINTF("loader addr= %x\n", usb_loader_code_addr);
    //check usbloader 
    if (usb_loader_code_addr)
    {
        data = (UINT8 *)usb_loader_code_addr;
        usb_loader_main_code_size = fetch_long(data + CHUNK_OFFSET);
	    UU_PRINTF("loader main code size= %x\n", usb_loader_main_code_size);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
		
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != len))
            ver_info->u_m_ver = get_code_version(addr, sig_len);  
    }
	
	data=(UINT8*)(usb_loader_code_addr+usb_loader_main_code_size);
	UU_PRINTF("block_addr = %x\n", data);
	// Go to the next chunk , get loader see code offset 
	usb_loader_see_code_size = fetch_long(data + CHUNK_OFFSET);
	UU_PRINTF("loader see offset = %x\n", usb_loader_see_code_size);
	
	// Go to the next chunk , get parameter offset 
	data = data + usb_loader_see_code_size;
	//usb_loader_parameter_size = fetch_long(data + CHUNK_OFFSET);
            usb_loader_parameter_size=CHUNK_HEADER_SIZE;
	UU_PRINTF("loader parameter offset = %x\n", usb_loader_parameter_size);
	usb_loader_code_size = usb_loader_main_code_size + usb_loader_see_code_size + usb_loader_parameter_size;
	UU_PRINTF("\nusb_loader_code_size = %x",usb_loader_code_size);
#endif    

//////////////////////////SEE PART///////////////////////////////

#if defined(_M3702_) || defined(_M3711C_)
		ver_info->s_f_ver = get_see_code_version_f(SEE_CODE_CHUNK_ID);
  #ifdef _OUC_LOADER_IN_FLASH_
		temp_buffer = cipher_addr ;
		chunk_id = OUC_CHUNK_ID;
		ret = get_chunk_in_flash(chunk_id,temp_buffer);
		temp_buffer = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);
		ver_info->us_f_ver = get_see_code_version_m(OUC_SEECODE_ID,temp_buffer);
  #endif
		chunk_id = SEE_CODE_CHUNK_ID;
		see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
		ver_info->s_m_ver = get_see_code_version_m(chunk_id,(UINT8* )see_code_addr);
        see_code_size = fetch_long((unsigned char *)(see_code_addr + CHUNK_OFFSET));
  #ifdef _OUC_LOADER_IN_FLASH_
		usb_loader_see_code_addr = usb_loader_main_code_addr + usb_loader_main_code_size;
        usb_loader_see_code_size = fetch_long((unsigned char *)usb_loader_see_code_addr + CHUNK_OFFSET);
		if (usb_loader_see_code_size < buf_len)
		{		
		   ver_info->us_m_ver = get_see_code_version_m(OUC_SEECODE_ID,(UINT8*) usb_loader_see_code_addr);
		}	 
  #endif
#else
    // decrypt universal key, fetch public key from flash    
    if (decrypt_universal_key_wrapped(&key_pos,E_KEY_SEE) < 0)
    {
        UU_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }
    // fetch see rsa key id
    rsa_key_id = get_rsa_key_id_by_type(E_KEY_SEE);
    
    UU_PRINTF("check see code in flash\n");
    len = 0;
    get_chunk_header_in_flash(SEE_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk(key_pos, SEE_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->s_f_ver = get_code_version_ext(addr, len,rsa_key_id);
    }


    UU_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    //UU_PRINTF("see addr= %x\n", see_code_addr);	
    see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("see addr= %x\n", see_code_addr);	
    if (see_code_addr)
    {
        data = (UINT8 *)see_code_addr;
        see_code_size = fetch_long(data + CHUNK_OFFSET);
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
	    enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != enc_len))
            ver_info->s_m_ver = get_code_version_ext(addr, sig_len,rsa_key_id);
    }

#ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check ouc loader see in flash\n");
    len = 0;
    temp_buffer = cipher_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {        
        //UINT8* ouc_main_addr = temp_buffer;
        UINT8* ouc_see_addr = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);         
        ver_info->us_f_ver = get_ram_chunk_version_ext(ouc_see_addr,key_pos,addr,rsa_key_id);     
    } 
#endif 

#ifdef _OUC_LOADER_IN_FLASH_ 
    UU_PRINTF("check ouc loader see in memory\n");
    usb_loader_see_code_addr = usb_loader_main_code_addr + usb_loader_main_code_size;
    usb_loader_see_code_size = fetch_long((unsigned char *)usb_loader_see_code_addr + CHUNK_OFFSET);

    if(usb_loader_see_code_size < buf_len)
    {
        ver_info->us_m_ver = get_ram_chunk_version_ext((UINT8 *)usb_loader_see_code_addr,key_pos,addr,rsa_key_id);
    }
#endif
#endif

#ifdef _OUC_LOADER_IN_FLASH_ 
    UU_PRINTF("version info: (0x%X, 0x%X,0x%X,0x%X),(0x%X, 0x%X), (0x%X, 0x%X)\n",\
        ver_info->u_f_ver,ver_info->us_f_ver,ver_info->u_m_ver,ver_info->us_m_ver,\
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#else
    UU_PRINTF("version info:  (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#endif
#ifdef _CAS9_CA_ENABLE_ 
    ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
#endif

#ifdef _MOD_DYNAMIC_LOAD_
    UU_PRINTF("check dyn code in flash\n");
    CHUNK_HEADER chk_hdr;
    UINT32 data_crc = 0;
    UINT32 crc = 0;
    UINT32 crc_len = 0;
    #define CODE_VERSION_LEN                    8

    ver_info->dyn_f_ver = 0;
    addr = (UINT8 *)find_chunk_by_sector(boot_len,MOD_PLUGIN_CHUNK_ID,0xFFFFFFFF,1);

    if(addr != 0)
    {
        if(1 == sto_get_chunk_header(MOD_PLUGIN_CHUNK_ID, &chk_hdr)) 
            ver_info->dyn_f_ver = str2uint32_dec(chk_hdr.version, CODE_VERSION_LEN);     
    }
        
    UU_PRINTF("check dyn code in memory\n");
    ver_info->dyn_m_ver = 0;
    chunk_id = MOD_PLUGIN_CHUNK_ID;
    dyn_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (dyn_code_addr)
    {
        data = (UINT8 *)dyn_code_addr;
        dyn_code_size = fetch_long(data + CHUNK_OFFSET);
        crc_len = fetch_long(data + CHUNK_LENGTH);
        data_crc = fetch_long(data + CHUNK_CRC);

        mg_setup_crc_table();
        crc = mg_table_driven_crc ( 0xFFFFFFFF, data + 16, crc_len );
        if(crc != data_crc)
            UU_PRINTF("dyn code in memory CRC fail\n");
        else
        {
            ver_info->dyn_m_ver = str2uint32_dec(data + CHUNK_VERSION, CODE_VERSION_LEN);
        }

    }
    UU_PRINTF("dyn version info:  (0x%X, 0x%X)\n", ver_info->dyn_f_ver, ver_info->dyn_m_ver);
#endif        

    return TRUE;
}

#ifdef _GEN_CA_ENABLE_
//fixed key
static BOOL usbupg_check_with_host_key(struct usbupg_ver_info *ver_info,UINT8 *pbuff, UINT32 buf_len)
{    
    UINT8 *cipher_addr= 0;
    UINT32 cipher_len= 0;
    UINT8 *addr= 0;
    UINT32 len= 0;
    UINT32 chunk_id = 0;
    UINT8 *data= NULL;
    UINT32 * block_addr __MAYBE_UNUSED__ =NULL ;
    UINT8 key_pos= 0;
    int ret= 0;
    
    CHUNK_HEADER chunk_header __MAYBE_UNUSED__;
 #ifdef _OUC_LOADER_IN_FLASH_ 
    UINT32 loader_f_addr = 0 ;
    UINT32 loader_f_size = 0 ;
 #endif

    if ((NULL == pbuff) || (0 == buf_len))
    {
        return FALSE;
    }
    //init chunk
    chunk_init((UINT32)pbuff, buf_len);    
    
    //GCA + NOVEL BL Flow.
    UINT8 key[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,};
    UINT8 iv[32];
    MEMSET(iv,0,sizeof(iv));
    set_key_iv(key,iv);

    // get the versions of codes in flash
    cipher_addr = pbuff + buf_len;
    cipher_len = 0x200000;
    flash_cipher_buf_init(cipher_addr, cipher_len);

    addr = cipher_addr + cipher_len;

    UU_PRINTF("check main code in flash\n");
    len = 0;
    get_chunk_header_in_flash(MAIN_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk_with_host_key(key_pos, MAIN_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->m_f_ver = get_code_version(addr, len);
    }
    UU_PRINTF("check see code in flash\n");
    len = 0;
    get_chunk_header_in_flash(SEE_CODE_CHUNK_ID,addr);
    ret = aes_cbc_decrypt_chunk_with_host_key(key_pos, SEE_CODE_CHUNK_ID, &addr[CHUNK_HEADER_SIZE], &len);
    if ((NUM_ZERO == ret) && (NUM_ZERO != len))
    {
        len += CHUNK_HEADER_SIZE;
        ver_info->s_f_ver = get_code_version(addr, len);
    }

#ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check ouc loader in flash\n");
    len = 0;
    UINT8 *temp_buffer = cipher_addr ;
    ret = get_chunk_in_flash(OUC_CHUNK_ID,temp_buffer);

    if(ret)
    {
        //UINT32 ouc_main_ver = 0;
        //UINT32 ouc_see_ver = 0;
        UINT8* ouc_main_addr = temp_buffer;
        UINT8* ouc_see_addr = temp_buffer + fetch_long(temp_buffer + CHUNK_OFFSET);        

        ver_info->u_f_ver = get_ram_chunk_version_fk(ouc_main_addr,key_pos,addr);
        ver_info->us_f_ver = get_ram_chunk_version_fk(ouc_see_addr,key_pos,addr);     
    } 
#endif 
	
    UU_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    //UU_PRINTF("main addr= %x\n", main_code_addr);	
    main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("main addr= %x\n", main_code_addr);
    if (main_code_addr)
    {
        usb_loader_m_main_offset = main_code_addr - (UINT32)pbuff + boot_len;
        data = (UINT8 *)main_code_addr;
        main_code_size = fetch_long(data + CHUNK_OFFSET);
       
        ver_info->m_m_ver = get_ram_chunk_version_fk(data,key_pos,addr); 
    }

    UU_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    //UU_PRINTF("see addr= %x\n", see_code_addr);	
    see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("see addr= %x\n", see_code_addr);	
    if (see_code_addr)
    {
        data = (UINT8 *)see_code_addr;
        see_code_size = fetch_long(data + CHUNK_OFFSET);        
        ver_info->s_m_ver = get_ram_chunk_version_fk(data,key_pos,addr); 
    }

#ifdef _OUC_LOADER_IN_FLASH_ 
    chunk_id = OUC_CHUNK_ID;
    usb_loader_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    usb_loader_code_size = fetch_long((unsigned char *)usb_loader_code_addr + CHUNK_OFFSET);
    loader_f_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((((INT32)loader_f_addr) < 0) || (usb_loader_code_size >= buf_len))
    {
        UU_PRINTF("err,loader_f_addr = 0x%08x,usb_loader_code_size = 0x%08x\n",\
            loader_f_addr,usb_loader_code_size);
        return FALSE;
    }

    loader_f_size = sto_fetch_long(loader_f_addr + CHUNK_OFFSET);
    if(usb_loader_code_size > loader_f_size)
    {
        UU_PRINTF("err,loader_f_size = 0x%08x,usb_loader_code_size = 0x%08x\n",\
            loader_f_size,usb_loader_code_size);
        return FALSE;
    }

    usb_loader_code_addr = usb_loader_code_addr + CHUNK_HEADER_SIZE;
    usb_loader_code_size = usb_loader_code_size - CHUNK_HEADER_SIZE;
    usb_loader_f_code_addr = loader_f_addr + CHUNK_HEADER_SIZE;
    usb_loader_f_code_size = loader_f_size - CHUNK_HEADER_SIZE;

    usb_loader_main_code_addr = usb_loader_code_addr ;
    usb_loader_main_code_size = fetch_long((unsigned char *)usb_loader_main_code_addr + CHUNK_OFFSET);
    if(usb_loader_main_code_size < buf_len)
    {
        ver_info->u_m_ver = get_ram_chunk_version_fk((UINT8 *)usb_loader_main_code_addr,key_pos,addr);
    }
    else
    {
        ver_info->u_m_ver= 0;
    }

    usb_loader_see_code_addr = usb_loader_main_code_addr + usb_loader_main_code_size;
    usb_loader_see_code_size = fetch_long((unsigned char *)usb_loader_see_code_addr + CHUNK_OFFSET);

    if(usb_loader_see_code_size < buf_len)
    {
        ver_info->us_m_ver = get_ram_chunk_version_fk((UINT8 *)usb_loader_see_code_addr,key_pos,addr);
    }
#else
    //caculate usb loader addr
    chunk_id = 0x04FB0100;
    userdb_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("userdb addr= %x\n", userdb_code_addr);
    data = (UINT8 *)userdb_code_addr;
    userdb_code_size = fetch_long(data + CHUNK_LENGTH) + CHUNK_NAME;
    UU_PRINTF("userdb size= %x\n", userdb_code_size);
 
    usb_loader_code_addr = userdb_code_addr + userdb_code_size;
    UU_PRINTF("loader addr= %x\n", usb_loader_code_addr);
    //check usbloader 
    if (usb_loader_code_addr)
    {
        data = (UINT8 *)usb_loader_code_addr;
        usb_loader_main_code_size = fetch_long(data + CHUNK_OFFSET);
	    UU_PRINTF("loader main code size= %x\n", usb_loader_main_code_size);
        
        ver_info->u_m_ver = get_ram_chunk_version_fk(data,key_pos,addr);  
    }
	
	data=(UINT8*)(usb_loader_code_addr+usb_loader_main_code_size);
	UU_PRINTF("block_addr = %x\n", data);
	// Go to the next chunk , get loader see code offset 
	usb_loader_see_code_size = fetch_long(data + CHUNK_OFFSET);
	UU_PRINTF("loader see offset = %x\n", usb_loader_see_code_size);
	
	// Go to the next chunk , get parameter offset 
	data = data + usb_loader_see_code_size;
	//usb_loader_parameter_size = fetch_long(data + CHUNK_OFFSET);
            usb_loader_parameter_size=CHUNK_HEADER_SIZE;
	UU_PRINTF("loader parameter offset = %x\n", usb_loader_parameter_size);
	usb_loader_code_size = usb_loader_main_code_size + usb_loader_see_code_size + usb_loader_parameter_size;
	UU_PRINTF("\nusb_loader_code_size = %x",usb_loader_code_size);
#endif

#ifdef _OUC_LOADER_IN_FLASH_ 
    UU_PRINTF("version info: (0x%X, 0x%X,0x%X,0x%X),(0x%X, 0x%X), (0x%X, 0x%X)\n",\
        ver_info->u_f_ver,ver_info->us_f_ver,ver_info->u_m_ver,ver_info->us_m_ver,\
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#else
    UU_PRINTF("version info:  (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#endif

    return TRUE;
}
#endif

static INT32 burn_block(UINT8 *pbuffer, UINT32 pos, UINT8 numsectors)
{
    struct sto_device *sto_dev = m_sto_device;
    UINT32 param[2];
    INT32 offset= 0;
    UINT32 er = SUCCESS;
    UINT8 i= 0;

    for(i=0; i<numsectors; i++)
    {
        offset = (pos + i) * C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] = C_SECTOR_SIZE >> 10; // length in K bytes
        UU_PRINTF("%s: erase sector %d\n", __FUNCTION__, pos+i);
        er = sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param);
        if(er != SUCCESS)
        {
            UU_PRINTF("%s: erase sector %d failed\n", __FUNCTION__, i);
            break;
        }

        if(pbuffer)
        {
            UU_PRINTF("%s: seek sector %d, offset = %xh\n", __FUNCTION__, pos+i,offset);
            if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
            {
                UU_PRINTF("%s: seek sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }

            UU_PRINTF("%s: write sector %d, src = %xh\n", __FUNCTION__, pos+i,&pbuffer[C_SECTOR_SIZE * i]);
            if(sto_write(sto_dev, &pbuffer[C_SECTOR_SIZE * i], C_SECTOR_SIZE) != C_SECTOR_SIZE)
            {
                UU_PRINTF("%s: write sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }
        }
        else
        {
            UU_PRINTF("%s: Error in Line(%d), Please check !!!\n",__FUNCTION__, __LINE__);
        }
        upg_secotrs_index++;
        win_usbupg_process_update(upg_secotrs_index*100/upg_total_sectors);
    }
    return er;
}

static INT32 BackupDatabase()
{
	struct sto_device *sto_dev = m_sto_device;
    INT32 er __MAYBE_UNUSED__ = 0;

	if(sto_lseek(sto_dev, (INT32)m_usb_info.user_db_addr_offset, STO_LSEEK_SET) !=  (INT32)m_usb_info.user_db_addr_offset)
    	{
    		return ERR_FAILED;
	}
		libc_printf("{%x,%x}\n",sto_lseek(sto_dev, (INT32)m_usb_info.user_db_addr_offset, STO_LSEEK_SET), m_usb_info.user_db_addr_offset);
	if(sto_read(sto_dev, (UINT8 *)m_usb_info.backup_db_addr, (INT32)m_usb_info.user_db_len) != (INT32)m_usb_info.user_db_len)
	{
		libc_printf("{%x,%x}\n",sto_read(sto_dev, (UINT8 *)m_usb_info.backup_db_addr, (INT32)m_usb_info.user_db_len),m_usb_info.user_db_len);
		return ERR_FAILED;
	}

    return SUCCESS;
}

//cas9
static BOOL win_usb_burnflash(struct usbupg_ver_info ver_info)
{
    UINT8 back_saved= 0;
    UINT32 chunk_id= 0;
    UINT32 chunk_addr= 0;
    UINT32 chunk_size= 0;
    UINT32 upgrade_size = 0;
    win_popup_choice_t win_ret = WIN_POP_CHOICE_NULL;

	if(WIN_POP_CHOICE_NULL == win_ret)
	{
		;
	}
	if (ver_info.m_m_ver > ver_info.m_f_ver)
    {
        upgrade_size += main_code_size;
    }
    if (ver_info.s_m_ver > ver_info.s_f_ver)
    {
        upgrade_size += see_code_size;
    }
    
#ifndef _BUILD_OTA_E_
#ifdef _OUC_LOADER_IN_FLASH_
    if (ver_info.u_m_ver > ver_info.u_f_ver\
        || ver_info.us_m_ver > ver_info.us_f_ver)
    {
        upgrade_size += usb_loader_code_size;
    }       
#endif
#endif

#ifdef _MOD_DYNAMIC_LOAD_
    if(ver_info.dyn_m_ver > ver_info.dyn_f_ver)
        upgrade_size += dyn_code_size;
#endif

#if defined( _BUILD_OTA_E_) && defined(_CAS9_CA_ENABLE_)
    if (NUM_ZERO==upgrade_size)
    {
        //update_allhdr_hmac(FALSE,0, NULL, 0);
        update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
        return 0;
    }
#endif

    if (0 == upgrade_size)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Nothing need to be upgrade!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        win_ret = win_compopup_open_ext(&back_saved);
        return FALSE;
    }  

#ifndef _BUILD_OTA_E_
#ifdef _OUC_LOADER_IN_FLASH_
    if(ver_info.u_m_ver == ver_info.u_f_ver\
        && ver_info.us_m_ver == ver_info.us_f_ver)
    {
        //JUST update param.
        upgrade_size += C_SECTOR_SIZE;;
    }        
#endif
#endif

    upg_total_sectors = upgrade_size / C_SECTOR_SIZE;
    upg_secotrs_index = 0;
    init_storage();

    UINT32 flash_pos= 0;
    UINT32 ret= 0;

#ifndef _BUILD_OTA_E_
#ifdef _OUC_LOADER_IN_FLASH_
  	// TODO: step 1: burn USB Loader
    if(0 == usb_loader_f_code_addr)
    {
        UU_PRINTF("Cannot find OUC loader chunk!\n");
        return FALSE;
    } 
    
     if(ver_info.u_m_ver > ver_info.u_f_ver\
        || ver_info.us_m_ver > ver_info.us_f_ver)
    {    
        usb_loader_bin_sectors = usb_loader_code_size/C_SECTOR_SIZE;
        if(usb_loader_code_size%C_SECTOR_SIZE)
        	usb_loader_bin_sectors++;

        flash_pos = usb_loader_f_code_addr / C_SECTOR_SIZE;
        //	libc_printf("\nusb_loader_code_addr = %x, flash_pos = %x ,sectors = %x\n",usb_loader_code_addr,m_usb_info.user_db_start_sector,usb_loader_bin_sectors);
        #ifdef FLASH_SOFTWARE_PROTECT
		ap_set_flash_lock_len(KEY_AREA_END_ADDR);
        #endif
        ret = burn_block((UINT8*)usb_loader_code_addr, flash_pos, usb_loader_bin_sectors);
        if(SUCCESS != ret)
            return FALSE;
     }
     else
     {       
        flash_pos = usb_loader_f_param_addr / C_SECTOR_SIZE;
        ret = burn_block((UINT8*)usb_loader_param_addr, flash_pos, 1);
     }
#else
	// TODO: step 1: backup UserDB
	UU_PRINTF("BackupDataBase cas9!\n");
	ret = BackupDatabase();
	if (SUCCESS != ret)
	{
		libc_printf("BackupDatabase failed\n");
		return FALSE;
	}
	// TODO: step 2: burn OTA to UserDB

	UU_PRINTF("Burn USB LOADER!\n");
	chunk_id = USER_DB_ID;
	chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1); /* We use user_db_start_sector address */
	chunk_addr+= CHUNK_HEADER_SIZE;
	chunk_size = usb_loader_code_size; //+ C_SECTOR_SIZE;
	usb_loader_bin_sectors = chunk_size/C_SECTOR_SIZE;
	
	if(chunk_size%C_SECTOR_SIZE)
		usb_loader_bin_sectors++;
	
	if ((UINT32)ERR_FAILUE == chunk_addr)
	{
		UU_PRINTF("Cannot find userdb's chunk!\n");
		return FALSE;
	}

	flash_pos = chunk_addr / C_SECTOR_SIZE;
//	libc_printf("\nusb_loader_code_addr = %x, flash_pos = %x ,sectors = %x\n",usb_loader_code_addr,m_usb_info.user_db_start_sector,usb_loader_bin_sectors);
	ret = burn_block((UINT8*)usb_loader_code_addr, flash_pos, usb_loader_bin_sectors);
    if(SUCCESS != ret)
        return FALSE;
#endif		
#endif

    chunk_id = MAINCODE_ID;
    chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if ((UINT32)ERR_FAILUE == chunk_addr)
    {
        #ifdef _OUC_LOADER_IN_FLASH_
        //use the addr in the memory.
        chunk_addr = usb_loader_m_main_offset;
        if(chunk_addr <= boot_len || chunk_addr > SYS_FLASH_SIZE)
        {
            UU_PRINTF("Cannot find maincode's chunk!\n");
            return FALSE;
        }
        #else
        UU_PRINTF("Cannot find maincode's chunk!\n");
        return FALSE;
        #endif
    }
    // burn new maincode.bin
    // main code chunk addr and size
    chunk_size = main_code_size;
    if (ver_info.m_m_ver > ver_info.m_f_ver)
    {
        UU_PRINTF("upgrade main code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
	    #if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
            //update_allhdr_hmac(TRUE,MAINCODE_ID,(UINT8*)main_code_addr,CHUNK_HEADER_SIZE);
            update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
        #endif
        ret = burn_block((UINT8 *)main_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
        if (SUCCESS != ret)
        {
            return FALSE;
        }
    }
    // see code chunk addr and size
    chunk_addr += chunk_size;
    chunk_size = see_code_size;
    if (ver_info.s_m_ver > ver_info.s_f_ver)
    {
        UU_PRINTF("upgrade see code\n");
        flash_pos = chunk_addr / C_SECTOR_SIZE;
	    #if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
            //update_allhdr_hmac(TRUE,SEECODE_ID,(UINT8*)see_code_addr,CHUNK_HEADER_SIZE);
            update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
        #endif            

        ret = burn_block((UINT8 *)see_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);	
        if (SUCCESS != ret)
        {
            return FALSE;
        }
    }
    
#ifndef _OUC_LOADER_IN_FLASH_	
#ifndef _BUILD_OTA_E_
		// TODO: step 3 burn back UserDB
		UU_PRINTF("BurnBack UserDB!\n");
	//	libc_printf("m_usb_info.backup_db_addr = %x, m_usb_info.user_db_start_sector=%x, m_usb_info.user_db_start_sector = %d\n",
	//				m_usb_info.backup_db_addr,m_usb_info.user_db_start_sector,m_usb_info.user_db_sectors);
		ret = burn_block((UINT8*)m_usb_info.backup_db_addr, m_usb_info.user_db_start_sector, m_usb_info.user_db_sectors);
        if(SUCCESS != ret)
            return FALSE;
#endif	
#endif

#ifdef _MOD_DYNAMIC_LOAD_
        // dyn code chunk addr and size
        chunk_addr += chunk_size;
        chunk_size = dyn_code_size;
        if (ver_info.dyn_m_ver > ver_info.dyn_f_ver)
        {
            UU_PRINTF("upgrade dyn code\n");
            flash_pos = chunk_addr / C_SECTOR_SIZE;                   
            ret = burn_block((UINT8 *)dyn_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
                return FALSE;
        }
#endif

#if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
    //update_allhdr_hmac(FALSE,0, NULL, 0);
    update_allhdr_hmac(HMAC_DYNAMIC_TOTAL);
#endif	

#ifdef _VERSION_CONTROL_
    if(!version_update(ver_info.m_m_ver,SW_VER))
    {
        UU_PRINTF("update code version fail\n");
        UINT8 back_saved=0;
        __MAYBE_UNUSED__ win_popup_choice_t popup_choice=WIN_POP_CHOICE_NULL;
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Update version fail!", NULL, 0);
        popup_choice=win_compopup_open_ext(&back_saved);
        //while(1);
    }
#endif
    return TRUE;
}
#endif

#ifdef _BC_CA_ENABLE_
static UINT32 upg_total_sectors;
static UINT32 upg_secotrs_index;
static UINT32 code_start_offset = 0;
static UINT32 code_start_addr = 0;
static UINT32 main_code_addr = 0;
static UINT32 main_code_size = 0;
static UINT32 see_code_addr = 0;
static UINT32 see_code_size = 0;
static UINT32 loader_main_code_addr __MAYBE_UNUSED__= 0;
static UINT32 loader_main_code_size __MAYBE_UNUSED__= 0;
static UINT32 loader_see_code_addr __MAYBE_UNUSED__= 0;
static UINT32 loader_see_code_size __MAYBE_UNUSED__= 0;
#if defined(_SMI_ENABLE_)
static UINT32 rsa_key_addr;
static UINT32 rsa_key_size;
static UINT32 ecgk_key_addr;
static UINT32 ecgk_key_size;
#endif
#ifdef BACKUP_USB_LOADER
#ifndef _BUILD_LOADER_COMBO_
static UINT32 usb_loader_code_addr __MAYBE_UNUSED__;
static UINT32 usb_loader_code_size __MAYBE_UNUSED__;
static UINT32 usb_loader_bin_size __MAYBE_UNUSED__;
static UINT32 usb_loader_bin_sectors __MAYBE_UNUSED__;
#endif
static UINT32 backup_db_addr, backup_db_size;
static UINT32 user_db_start_sector, user_db_sectors, user_db_addr_offset,user_db_len;
static UINT32 usb_tmp_buf_addr,usb_tmp_buf_size;
#endif
static struct sto_device *m_sto_device = NULL;
struct usbupg_ver_info
{
    UINT32 m_f_ver; // main code in flash version
    UINT32 m_m_ver; // main code in memory version
    UINT32 s_f_ver; // see code in flash version
    UINT32 s_m_ver; // see code in memory version
#ifdef _SMI_ENABLE_
	UINT32 rsa_f_ver; // rsa key in flash version
	UINT32 rsa_m_ver; // rsa key in flash version
	UINT32 ecgk_f_ver; // ecgk in flash version
	UINT32 ecgk_m_ver; // ecgk in flash version
#endif
#ifdef _OUC_LOADER_IN_FLASH_
    UINT32 u_f_ver; //usb loader(main) in flash version
    UINT32 u_m_ver; //usb loader(main) in memory version
    UINT32 us_f_ver;   // usb loader (see) in flash version
    UINT32 us_m_ver;   // usb loader (see) in memory version
#else    
#ifdef BACKUP_USB_LOADER
    UINT32 u_m_ver; //usb loader in memory version
#endif
#endif
};

#define NEW_USBUPG_FLOW


///////////////////////////////////////////////////

static INT8 char2hex(UINT8 ch)
{
    INT8 ret =  - 1;
    if ((ch <= ASCII_NINE) && (ch >= ASCII_ZERO))
    // '0'~'9'
        ret = ch &0xf;
    else if ((ch <= ASCII_LOWERCASE_F) && (ch >= ASCII_LOWERCASE_A))
    //'a'~'f'
        ret = ch - 97+10;
    else if ((ch <= ASCII_UPPERCASE_F) && (ch >= ASCII_UPPERCASE_A))
    //'A'~'F'
        ret = ch - 65+10;

    return ret;
}

#ifndef _BC_CA_NEW_
static UINT32 str2uint32(UINT8 *str, UINT8 len)
{
    UINT32 ret = 0;
    UINT8 i= 0;
    INT temp= 0;

    if (str == NULL)
        return 0;

    for (i = 0; i < len; i++)
    {
        temp = char2hex(*str++);
        if (temp ==  - 1)
            return 0;

        ret = (ret << 4) | temp;
    }

    return ret;
}
#endif

static INT32 init_storage()
{
    m_sto_device = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if(NULL == m_sto_device)
    {
        UU_PRINTF("%s: can not get sto device\n", __FUNCTION__);
        return ERR_NO_DEV;
    }

    INT32 er = sto_open(m_sto_device);
    if(SUCCESS != er)
    {
        UU_PRINTF("%s: can not open sto device\n", __FUNCTION__);
    }


    return er;
}

static INT32 backup_database()
{
    struct sto_device *sto_dev = m_sto_device;
    //INT32 er= 0;

    if(sto_lseek(sto_dev, (INT32)user_db_addr_offset, STO_LSEEK_SET) !=  (INT32)user_db_addr_offset)
        return ERR_FAILED;
    if(sto_read(sto_dev, (UINT8 *)backup_db_addr, (INT32)user_db_len) != (INT32)user_db_len)
        return ERR_FAILED;
    return SUCCESS;
}

static BOOL usb_upg_set_memory(UINT8 *pbuff,UINT32 length)
{
    UINT32 addr_offset, len;

    if(length>READ_USBUPG_BUFFER_SIZE)
    {
        UU_PRINTF("buffer overlaping\n");
        return FALSE;
    }
    usb_tmp_buf_addr=(UINT32)(pbuff+READ_USBUPG_BUFFER_SIZE);
    usb_tmp_buf_size=0x300000;
    backup_db_addr=usb_tmp_buf_addr+usb_tmp_buf_size;
    backup_db_size = 0x200000;

    // init user_db addrss
    if(!api_get_chunk_add_len(0x04FB0100, &addr_offset, &len))
    {
        UU_PRINTF("Cannot find user db chunk\n");
        return FALSE;
    }

    user_db_addr_offset = addr_offset;
    user_db_len=len;
    user_db_start_sector = user_db_addr_offset/C_SECTOR_SIZE;
    user_db_sectors = user_db_len/C_SECTOR_SIZE;
    if(user_db_len%C_SECTOR_SIZE)
    {
        user_db_sectors++;
    }

    return TRUE;
}

#ifdef _SMI_ENABLE_
extern RET_CODE test_smi_rsa_ram(const UINT32 addr, const UINT32 len);
#define USBUPG_CHECK_SMI    usbupg_check
static BOOL USBUPG_CHECK_SMI(struct usbupg_ver_info *ver_info,UINT8 *pbuff, UINT32 buf_len)
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
	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (flash_dev == NULL)
	{
		UU_PRINTF("Can't find FLASH device!\n");
	}
	sto_open(flash_dev);
	sto_chunk_init(boot_total_area_len, flash_dev->totol_size-boot_total_area_len);
	
	if (NULL == pbuff || 0 == buf_len)
	{
		return FALSE;
	}

    vendor_id_select(VID_CHOOSE_SMI_VENDORID);
	//init chunk
	//chunk_init((unsigned long)(pbuff+boot_total_area_len), buf_len-boot_total_area_len);
	//pbuff have skip the BL.
	chunk_init((UINT32)pbuff,buf_len);
	
	temp_buffer = (UINT8*)usb_tmp_buf_addr;

	chunk_id=RSA_KEY_CHUNK_ID;
	block_addr=(UINT8*)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if((UINT8*)ERR_FAILUE!=block_addr)
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
	block_addr=(UINT8*)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if((UINT8*)ERR_FAILUE!=block_addr)
	{
		data = block_addr+CHUNK_HEADER_SIZE;
		data_len = (UINT32)sto_fetch_long((unsigned long)(block_addr + CHUNK_LENGTH));
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
	
	UU_PRINTF("check rsa key in memory\n");
	chunk_id = RSA_KEY_CHUNK_ID;
	rsa_key_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (rsa_key_addr)
	{
		block_addr = (UINT8 *)rsa_key_addr;
		rsa_key_size = fetch_long(block_addr + CHUNK_OFFSET);
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
	UU_PRINTF("RSA key: 0x%X, 0x%X\n", rsa_key_addr, rsa_key_size);

	if((0==ver_info->rsa_f_ver) && (0==ver_info->rsa_m_ver))
	{
		UU_PRINTF("no valid rsa key in flash or memory\n");
		return FALSE;
	}
	
	if((ver_info->rsa_m_ver > ver_info->rsa_f_ver)||(MEMCMP(g_cust_public_key, data, 516)!=0)) //cstm rsa key diff, need to upgrade rsa key and ecgk
	{
		MEMCPY(g_cust_public_key, data, 516); //use upgraded cust public key to verify main, see and ota loader
		ca_switch_mode = 1;
	}
	else
	{
		ca_switch_mode = 0;
	}

	UU_PRINTF("check ecgk memory\n");
	chunk_id = ECGK_CHUNK_ID;
	ecgk_key_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (ecgk_key_addr)
	{
		block_addr = (UINT8 *)ecgk_key_addr;
		ecgk_key_size = fetch_long(block_addr + CHUNK_OFFSET);
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
	UU_PRINTF("ECGK: 0x%X, 0x%X\n", ecgk_key_addr, ecgk_key_size);

	if((0==ver_info->ecgk_f_ver) && (0==ver_info->ecgk_m_ver))
	{
		UU_PRINTF("no ecgk key in flash or memory\n");
		return FALSE;
	}

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
    config_check_temp_buffer(usb_tmp_buf_addr,usb_tmp_buf_size);
    
    chunk_id = OUC_CHUNK_ID;
	block_addr = (UINT8*)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((UINT32)block_addr != (UINT32)ERR_FAILUE)
    {
        UINT32 chunk_addr = 0;
        //skip OUC header.
        OTA_PRINTF("check loader main code in flash\n");
        chunk_addr = (UINT32)block_addr + CHUNK_HEADER_SIZE; 
        get_main_version_in_flash(chunk_addr,&ver_info->u_f_ver);        

        OTA_PRINTF("check loader see code in flash\n");
        chunk_addr += sto_fetch_long(chunk_addr + CHUNK_OFFSET); 
        get_see_version_in_flash(chunk_addr,&ver_info->us_f_ver);             
    }
#else
#ifdef BACKUP_USB_LOADER
	UU_PRINTF("check usb loader in memory\n");

	if(!get_chunk_add_len(0x04FB0100, &usb_loader_code_addr, &len))
	{
		UU_PRINTF("cannot find user db in memory\n");
		return FALSE;
	}
	usb_loader_code_addr=usb_loader_code_addr+len;
	if (usb_loader_code_addr)
	{		
		block_addr = (UINT8 *)usb_loader_code_addr;
		usb_loader_code_size = fetch_long(block_addr + CHUNK_OFFSET);
		usb_loader_bin_size=usb_loader_code_size+C_SECTOR_SIZE; //add a sector for cfg, backup loader and cfg (match ota structure in bootloader)
		usb_loader_bin_sectors=usb_loader_bin_size/C_SECTOR_SIZE;
		if(usb_loader_bin_size%C_SECTOR_SIZE)
		{
			usb_loader_bin_sectors++;
		}
		UU_PRINTF("usb_loader_bin_size=%d, usb_loader_bin_sectors=%d\n",usb_loader_bin_size, usb_loader_bin_sectors);
		UU_PRINTF("usb_loader_code_addr=0x%x, usb_loader_code_size=0x%x\n",usb_loader_code_addr, usb_loader_code_size);
		UU_PRINTF("usb_loader_id=0x%x 0x%x\n",*((UINT8 *)usb_loader_code_addr),*((UINT8 *)usb_loader_code_addr+1));
		data_len = fetch_long(block_addr + CHUNK_LENGTH);
		data = block_addr+CHUNK_HEADER_SIZE;
		len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
		//verify memory ota loader using flash rsa key
		if(TRUE==verify_code(data, len, g_cust_public_key,g_ecgk_key))
		{
			UU_PRINTF("usb loader verify OK\n");
		}
		else
		{
			return FALSE;
		}
	}
	UU_PRINTF("usb loader: 0x%X, 0x%X\n", usb_loader_code_addr, usb_loader_code_size);
#endif
#endif
	
 #ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check usbloader main code in memory\n");
    
    chunk_id = OUC_CHUNK_ID;
	block_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if(block_addr)
    {
        loader_main_code_addr = (UINT32)block_addr + CHUNK_HEADER_SIZE;
   		block_addr = (UINT8 *)loader_main_code_addr;
		loader_main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_main_code_in_mem(block_addr,&ver_info->u_m_ver))
        {
            UU_PRINTF("Verify loader main code in memory failed \n");
            return FALSE;
        }	
        
        UINT32 main_ck_id = fetch_long((UINT8*)loader_main_code_addr + CHUNK_ID) ;
        if(main_ck_id != OTA_MAIN_CODE_CHUNK_ID)
        {
            UU_PRINTF("Cannot find usbloader main code in memory\n");
            return FALSE;
        }
        
	}
	UU_PRINTF("usbloader main code: 0x%X, 0x%X\n", loader_main_code_addr, loader_main_code_size);	

	UU_PRINTF("check usbloader see code in memory\n");
    if(loader_main_code_addr)
    {        
        UINT32 main_offset = fetch_long((UINT8*)loader_main_code_addr + CHUNK_OFFSET) ;
       
        loader_see_code_addr = loader_main_code_addr + main_offset;
		block_addr = (UINT8 *)loader_see_code_addr;
        loader_see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_see_code_in_mem(block_addr,&ver_info->us_m_ver))
        {
            UU_PRINTF("Verify loader see code in memory failed \n");
            return FALSE;
        } 	
	}
	UU_PRINTF("loader see code: 0x%X, 0x%X\n", loader_see_code_addr, loader_see_code_size);
#endif
    
	UU_PRINTF("check main code in memory\n");
	chunk_id = MAIN_CODE_CHUNK_ID;
	main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (main_code_addr)
	{
		block_addr = (UINT8 *)main_code_addr;
		main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        if(!get_main_code_in_mem(block_addr,&ver_info->m_m_ver))
        {
            UU_PRINTF("Verify main code in memory failed \n");
            return FALSE;
        }		
	}
	UU_PRINTF("main code: 0x%X, 0x%X\n", main_code_addr, main_code_size);

	UU_PRINTF("check see code in memory\n");
	chunk_id = SEE_CODE_CHUNK_ID;
	see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if (see_code_addr)
	{
		block_addr = (UINT8 *)see_code_addr;
		see_code_size = fetch_long(block_addr + CHUNK_OFFSET);        
        if(!get_see_code_in_mem(block_addr,&ver_info->s_m_ver))
        {
            UU_PRINTF("Verify see code in memory failed \n");
            return FALSE;
        }        
	}
	UU_PRINTF("see code: 0x%X, 0x%X\n", see_code_addr, see_code_size);
	UU_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X)\n",
		ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#ifdef _OUC_LOADER_IN_FLASH_  
    UU_PRINTF("loader version info: (0x%X, 0x%X), (0x%X, 0x%X)\n",
		ver_info->u_f_ver,ver_info->u_m_ver, ver_info->us_f_ver, ver_info->us_m_ver);
#endif

    if(!ver_info->m_m_ver || !ver_info->s_m_ver)
    {
        UU_PRINTF("Failed to get Main or SEE version !!!");
        return FALSE;
    }

    if(!ver_info->u_m_ver || !ver_info->us_m_ver)
    {
        UU_PRINTF("Failed to get OUC loader Main or SEE version !!!\n");
        return FALSE;
    }

	return TRUE;	//save apps to flash
}
#undef USBUPG_CHECK_SMI

#else

#ifdef NEW_USBUPG_FLOW
#define USBUPG_CHECK_BC_SC15    usbupg_check
static BOOL USBUPG_CHECK_BC_SC15(struct usbupg_ver_info *ver_info,UINT8 *pbuff, UINT32 buf_len)
{
    UINT8 *cipher_addr= 0;
    UINT32 cipher_len= 0;
    UINT8 *decrypt_addr= 0;
    //UINT32 decrypt_len= 0;
    UINT8 *block_addr=NULL;
    UINT32 chunk_id = 0;
    UINT8 *data= NULL;
    UINT8 key_pos= 0;
    int ret= 0;
#ifdef _OUC_LOADER_IN_FLASH_ 
    //UINT32 loader_f_addr = 0 ;
    //UINT32 loader_f_size = 0 ;
    UINT8 *temp_buffer = NULL;    
#endif

    if ((NULL == pbuff) || (0 == buf_len))
    {
        return FALSE;
    }
    //init chunk
    chunk_init((UINT32)pbuff, buf_len);

//////////////////////////MAIN PART///////////////////////////////    
    // decrypt universal key, fetch public key from flash    
    if (decrypt_universal_key_wrapped(&key_pos,E_KEY_MAIN) < 0)
    {
        UU_PRINTF("Decrypt universal key failed!\n");
        return FALSE;
    }

    // get the versions of codes in flash
    cipher_addr = pbuff + buf_len;
    cipher_len = 0x300000;
    flash_cipher_buf_init(cipher_addr, cipher_len);
    decrypt_addr = cipher_addr + cipher_len;
    //decrypt_len = cipher_len;

    UU_PRINTF("check main code in flash\n");    
    get_chunk_in_flash_ex(MAIN_CODE_CHUNK_ID,cipher_addr,cipher_len);    
    get_mem_chunk_version(cipher_addr,key_pos,decrypt_addr,&ver_info->m_f_ver); 

    UU_PRINTF("check see code in flash\n");
    get_chunk_in_flash_ex(SEE_CODE_CHUNK_ID,cipher_addr,cipher_len); 
    get_see_version_in_mem(cipher_addr,&ver_info->s_f_ver);
   
#ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check ouc loader main in flash\n");
    temp_buffer = cipher_addr ;
    ret = get_chunk_data_in_flash(OUC_CHUNK_ID,temp_buffer,cipher_len);
    if(ret)
    {        
        UINT8* ouc_main_addr = temp_buffer;         
        if(fetch_long(ouc_main_addr) != OTA_MAIN_CODE_CHUNK_ID)
        {
            UU_PRINTF("Error addr(0x%08x) for 0x%08x\n",ouc_main_addr,OTA_MAIN_CODE_CHUNK_ID);  
        }
  		if ((*ouc_main_addr) + (*(ouc_main_addr+1)) == 0xFF)
		{            
            get_mem_chunk_version(ouc_main_addr,key_pos,decrypt_addr,&ver_info->u_f_ver); 
		}

        UU_PRINTF("check ouc loader see in flash\n");  
        UINT8* ouc_see_addr = ouc_main_addr + fetch_long(ouc_main_addr + CHUNK_OFFSET);
        if(fetch_long(ouc_see_addr) != OTA_SEE_CODE_CHUNK_ID)
        {
            UU_PRINTF("Error addr(0x%08x) for 0x%08x\n",ouc_main_addr,OTA_MAIN_CODE_CHUNK_ID);  
        }
        get_mem_chunk_version(ouc_main_addr,key_pos,decrypt_addr,&ver_info->us_f_ver); 
    } 
#endif 
	
    UU_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    //UU_PRINTF("main addr= %x\n", main_code_addr);	
    main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("main addr= %x\n", main_code_addr);
    if (main_code_addr)
    {
        data = (UINT8 *)main_code_addr;
        main_code_size = fetch_long(data + CHUNK_OFFSET);
        get_mem_chunk_version(data,key_pos,decrypt_addr,&ver_info->m_m_ver); 
    }

    UU_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
	see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    UU_PRINTF("see addr= %x\n", see_code_addr);
    if(see_code_addr)
    {
        data = (UINT8 *)see_code_addr;
        see_code_size = fetch_long((unsigned char *)(see_code_addr + CHUNK_OFFSET));
        get_see_version_in_mem(data,&ver_info->s_m_ver);
    }
    
 #ifdef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("check usbloader main code in memory\n");
    
    chunk_id = OUC_CHUNK_ID;
	block_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if(block_addr)
    {
        loader_main_code_addr = (UINT32)block_addr + CHUNK_HEADER_SIZE;
   		block_addr = (UINT8 *)loader_main_code_addr;
		loader_main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        get_mem_chunk_version(block_addr,key_pos,decrypt_addr,&ver_info->u_m_ver); 	
        
        UINT32 main_ck_id = fetch_long((UINT8*)loader_main_code_addr + CHUNK_ID) ;
        if(main_ck_id != OTA_MAIN_CODE_CHUNK_ID)
        {
            UU_PRINTF("Cannot find usbloader main code in memory\n");
            return FALSE;
        }
        
	}
	UU_PRINTF("usbloader main code: 0x%X, 0x%X\n", loader_main_code_addr, loader_main_code_size);	

	UU_PRINTF("check usbloader see code in memory\n");
    if(loader_main_code_addr)
    {        
        UINT32 main_offset = fetch_long((UINT8*)loader_main_code_addr + CHUNK_OFFSET) ;
       
        loader_see_code_addr = loader_main_code_addr + main_offset;
		block_addr = (UINT8 *)loader_see_code_addr;
        loader_see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        get_see_version_in_mem(block_addr,&ver_info->us_m_ver);
	}
	UU_PRINTF("loader see code: 0x%X, 0x%X\n", loader_see_code_addr, loader_see_code_size);
#endif

//////////////////////////SEE PART///////////////////////////////
    UU_PRINTF("version info:  (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);
#ifdef _OUC_LOADER_IN_FLASH_  
    UU_PRINTF("loader version info: (0x%X, 0x%X), (0x%X, 0x%X)\n",
		ver_info->u_f_ver,ver_info->u_m_ver, ver_info->us_f_ver, ver_info->us_m_ver);
#endif

    if(!ver_info->m_m_ver || !ver_info->s_m_ver)
    {
        UU_PRINTF("Failed to get Main or SEE version !!!");
        return FALSE;
    }

    if(!ver_info->u_m_ver || !ver_info->us_m_ver)
    {
        UU_PRINTF("Failed to get OUC loader Main or SEE version !!!\n");
        return FALSE;
    }

    return TRUE;
}
#undef USBUPG_CHECK_BC_SC15

#else

extern int BC_VerifySignature( uint8_t* signature, uint8_t* src, uint8_t* tmp,\
                       uint32_t len, uint32_t max_len, \
               uint8_t mode,uint8_t *error_code );

#define USBUPG_CHECK_BC_OLD    usbupg_check
static BOOL USBUPG_CHECK_BC_OLD(struct usbupg_ver_info *ver_info,UINT8 *pbuff, UINT32 buf_len)
{
    //struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0;
    UINT8 *data= 0;
    UINT32 data_len= 0;
    UINT8 *block_addr= 0;
    int ret= 0;
    uint8_t *signature=NULL;
    uint8_t *tmp_buffer=NULL;
    uint32_t len, max_len= 0;
    uint8_t mode= 0;
    uint8_t error_code= 0;
    unsigned int header_crc=0;
    //UINT32 block_header, version;
    CHUNK_HEADER blk_header;
    UINT8 verify_result=0;

#ifdef _BUILD_LOADER_COMBO_
    struct UKEY_INFO ukey;
    UINT32 boot_type = 0;
    boot_type = sys_ic_get_boot_type();
    MEMSET(&ukey, 0x0, sizeof(UKEY_INFO));
#endif

    if ((NULL == pbuff) ||( 0 == buf_len))
    {
        return FALSE;
    }
    //init chunk
    chunk_init((unsigned long)pbuff, buf_len);

    tmp_buffer = (uint8_t *)usb_tmp_buf_addr;
    mg_setup_crc_table();

#ifdef BACKUP_USB_LOADER
	#ifndef _BUILD_LOADER_COMBO_
    UU_PRINTF("check usb loader in memory\n");

    if(!get_chunk_add_len(0x04FB0100, &usb_loader_code_addr, (UINT32 *)&len))
    {
        UU_PRINTF("cannot find user db in memory\n");
        return FALSE;
    }

    usb_loader_code_addr=usb_loader_code_addr+len;
    if (usb_loader_code_addr)
    {

        block_addr = (UINT8 *)usb_loader_code_addr;
        usb_loader_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        //add a sector for cfg, backup loader and cfg (match ota structure in bootloader)
        usb_loader_bin_size=usb_loader_code_size+C_SECTOR_SIZE;
        usb_loader_bin_sectors=usb_loader_bin_size/C_SECTOR_SIZE;
        if(usb_loader_bin_size%C_SECTOR_SIZE)
        {
            usb_loader_bin_sectors++;
        }
        UU_PRINTF("usb_loader_bin_size=%d, usb_loader_bin_sectors=%d\n",usb_loader_bin_size, usb_loader_bin_sectors);
        UU_PRINTF("usb_loader_code_addr=0x%x, usb_loader_code_size=0x%x\n",usb_loader_code_addr, usb_loader_code_size);
        UU_PRINTF("usb_loader_id=0x%x 0x%x\n",*((UINT8 *)usb_loader_code_addr),*((UINT8 *)usb_loader_code_addr+1));
        data_len = fetch_long(block_addr + CHUNK_LENGTH);
        data = block_addr+CHUNK_HEADER_SIZE;

        len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
        signature=data+len;
        //maxLen=usb_loader_code_size; //Clarify 20120806
        max_len=len;
        mode=1;     //from loader
        ret=BC_VerifySignature(signature, data, tmp_buffer, len, max_len, mode,&error_code);
        if(1 == ret)  //app can be started
        {
            UU_PRINTF("usb loader cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(BC_REENCRYPTION_DONE == error_code)    //store app to flash and reboot later
            {
                ver_info->u_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
                //calculate crc value for chunk header
                header_crc=mg_table_driven_crc(0xFFFFFFFF, block_addr+CHUNK_NAME, data_len);
                *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
                *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
                *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
                *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
                UU_PRINTF("ota block header_crc=0x%x first byte=0x%x\n",header_crc,*(block_addr+CHUNK_CRC));
                UU_PRINTF("usb loader verify ok and re-encrypt\n");
                verify_result|=SAVE_USB_LOADER;
            }
            else if(1 == error_code)
            {
                UU_PRINTF("usb code verify failed\n");
            }
        }

    }


    UU_PRINTF("usb loader: 0x%X, 0x%X\n", usb_loader_code_addr, usb_loader_code_size);
	#else
    verify_result|=SAVE_USB_LOADER;
	#endif
#endif

    UU_PRINTF("check main code in memory\n");
    chunk_id = MAIN_CODE_CHUNK_ID;
    main_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (main_code_addr)
    {
        block_addr = (UINT8 *)main_code_addr;
        main_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        data_len = fetch_long(block_addr + CHUNK_LENGTH);
        data = block_addr+CHUNK_HEADER_SIZE;

        len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
        signature=data+len;
        //maxLen=main_code_size;  //Clarify 20120806
        max_len=len;
        mode=1;     //from loader
        ret=BC_VerifySignature(signature, data, tmp_buffer, len, max_len, mode,&error_code);
        if(1 == ret)  //app can be started
        {
            UU_PRINTF("main code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(BC_REENCRYPTION_DONE == error_code)    //store app to flash and reboot later
            {
                ver_info->m_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
                //calculate crc value for chunk header
                header_crc=mg_table_driven_crc(0xFFFFFFFF, block_addr+CHUNK_NAME, data_len);
                *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
                *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
                *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
                *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
                UU_PRINTF("main block header_crc=0x%x first byte=0x%x\n",header_crc,*(block_addr+CHUNK_CRC));
                UU_PRINTF("main code verify ok and re-encrypt\n");
                verify_result|=SAVE_MAIN;
            }
            else if(1 == error_code)
            {
                UU_PRINTF("main code verify failed\n");
            }
        }
    }
    UU_PRINTF("main code: 0x%X, 0x%X\n", main_code_addr, main_code_size);

    UU_PRINTF("check see code in memory\n");
    chunk_id = SEE_CODE_CHUNK_ID;
    see_code_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (see_code_addr)
    {
        block_addr = (UINT8 *)see_code_addr;
        see_code_size = fetch_long(block_addr + CHUNK_OFFSET);
        data_len = fetch_long(block_addr + CHUNK_LENGTH);
        data = block_addr+CHUNK_HEADER_SIZE;

	    #ifndef _BUILD_LOADER_COMBO_
        len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME)-SIGNATURE_SIZE;
        signature=data+len;
        //maxLen=see_code_size;  //Clarify 20120806
        max_len=len;
        mode=1;     //from loader
        ret=BC_VerifySignature(signature, data, tmp_buffer, len, max_len, mode,&error_code);
	    #else	
	    len=data_len-(CHUNK_HEADER_SIZE-CHUNK_NAME);
	    if(RET_SUCCESS != generate_bc_see_key(&ukey))
        {
        	UU_PRINTF("Error :generate see key failed!\n");
         	return FALSE;
    	}
	    #endif
        if(1 == ret)  //app can be started
        {
            UU_PRINTF("see code cannot be started in loader mode\n");
        }
        else    // don't start app
        {
            if(BC_REENCRYPTION_DONE == error_code)    //store app to flash and reboot later
            {
                #ifndef _BUILD_LOADER_COMBO_
                ver_info->s_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
                //calculate crc value for chunk header
                header_crc=mg_table_driven_crc(0xFFFFFFFF, block_addr+CHUNK_NAME, data_len);
                *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
                *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
                *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
                *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
                UU_PRINTF("see block header_crc=0x%x first byte=0x%x\n",header_crc,*(block_addr+CHUNK_CRC));
                UU_PRINTF("see code verify ok and re-encrypt\n");
                verify_result|=SAVE_SEE;
	            #else
		        ver_info->s_m_ver = str2uint32(block_addr+CHUNK_VERSION, 8);
		        get_see_root_pub_key(see_rsa_pub_key,sizeof(see_rsa_pub_key));
		        if (RET_SUCCESS == test_cust_rsa_ram((UINT32)block_addr+ CHUNK_HEADER_SIZE, len,see_rsa_pub_key))
		        {
			        //encrypte SeeCode
			        UU_PRINTF("enc see\n");
			        tmp_buffer = (UINT8*)( (0xFFFFFFF8&(UINT32)MALLOC(see_code_size+0xf)) );
			
                    ret = aes_crypt_puredata_with_ce_key(block_addr+ CHUNK_HEADER_SIZE, tmp_buffer, len, ukey.kl_key_pos, DSC_ENCRYPT);
			        MEMCPY(block_addr+ CHUNK_HEADER_SIZE, tmp_buffer, len);
			        //calculate crc value for chunk header
	        	    header_crc=mg_table_driven_crc(0xFFFFFFFF, block_addr+CHUNK_NAME, data_len);
	        	    *(block_addr+CHUNK_CRC)=(UINT8)(header_crc>>24);
	        	    *(block_addr+CHUNK_CRC+1)=(UINT8)(header_crc>>16);
	        	    *(block_addr+CHUNK_CRC+2)=(UINT8)(header_crc>>8);
	        	    *(block_addr+CHUNK_CRC+3)=(UINT8)(header_crc);
	        	    UU_PRINTF("SeeCode block header_crc=0x%x first byte=0x%x\n",header_crc,*(block_addr+CHUNK_CRC));

			        FREE(tmp_buffer);
		            tmp_buffer=NULL;
			        verify_result|=SAVE_SEE;
	        	    UU_PRINTF("SeeCode encrypt done\n");
		         }
	             #endif
            }
            else if(1 == error_code)
            {
                UU_PRINTF("see code verify failed\n");
            }
        }

    }
    UU_PRINTF("see code: 0x%X, 0x%X\n", see_code_addr, see_code_size);


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

    UU_PRINTF("version info: (0x%X, 0x%X), (0x%X, 0x%X)\n",
        ver_info->m_f_ver,ver_info->m_m_ver, ver_info->s_f_ver, ver_info->s_m_ver);

    UU_PRINTF("verify result=0x%x\n",verify_result);
    if(verify_result==(SAVE_MAIN|SAVE_SEE|SAVE_USB_LOADER))
    {
        return TRUE;    //save apps to flash
    }
    else
    {
        return FALSE;
    }
}
#undef USBUPG_CHECK_BC_OLD
#endif
#endif


static INT32 burn_block(UINT8 *pbuffer, UINT32 pos, UINT8 numsectors)
{
    struct sto_device *sto_dev = m_sto_device;
    UINT32 param[2];
    INT32 offset, er = SUCCESS;
    UINT8 i= 0;

    for(i=0; i<numsectors; i++)
    {
        offset = (pos + i) * C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] = C_SECTOR_SIZE >> 10; // length in K bytes
        UU_PRINTF("%s: erase sector %d\n", __FUNCTION__, pos+i);
        er = sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param);
        if(er != SUCCESS)
        {
            UU_PRINTF("%s: erase sector %d failed\n", __FUNCTION__, i);
            break;
        }

        if(pbuffer)
        {
            UU_PRINTF("%s: seek sector %d, offset = %xh\n", __FUNCTION__, pos+i,offset);
            if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
            {
                UU_PRINTF("%s: seek sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }

            UU_PRINTF("%s: write sector %d, src = %xh\n", __FUNCTION__, pos+i,&pbuffer[C_SECTOR_SIZE * i]);
            if(sto_write(sto_dev, &pbuffer[C_SECTOR_SIZE * i], C_SECTOR_SIZE) != C_SECTOR_SIZE)
            {
                UU_PRINTF("%s: write sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }
        }
        else
        {
            UU_PRINTF("%s: Error in Line(%d), Please check !!!\n",__FUNCTION__, __LINE__);
        }
        upg_secotrs_index ++;
        win_usbupg_process_update(upg_secotrs_index*100/upg_total_sectors);
        UU_PRINTF("%s: %d -%d ,Percent(%d) \n",__FUNCTION__, \
            upg_secotrs_index,upg_total_sectors,upg_secotrs_index*100/upg_total_sectors);
    }
    return er;
}

#ifdef _OUC_LOADER_IN_FLASH_
BOOL save_upgrade_param(UINT8* buf,UINT32 buf_len)
{
    BOOL ret = FALSE;
    UINT32 chunk_id = OUC_CHUNK_ID;
    UINT8* p = NULL;
    OUC_CFG ouc_cfg;
    UINT32 crc=0;
    UINT32 len=0;
    
    MEMSET(&ouc_cfg,0,sizeof(ouc_cfg));
    ouc_cfg.magic = OUC_MAGIC ;
    ouc_cfg.upgrade_mode = OUC_TYPE_USB;    
    
    if(loader_main_code_addr)
    {
        chunk_id = CHUNKID_OTA_PARA;
        chunk_init(loader_main_code_addr, loader_main_code_size * 3);       
        p = (UINT8*)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
        if(p)
        {            
            MEMCPY(p+CHUNK_HEADER_SIZE+OUC_CFG_OFFSET, &ouc_cfg, sizeof(ouc_cfg));    
    
            mg_setup_crc_table();
            len = fetch_long(p + CHUNK_LENGTH);
            crc = (UINT32)mg_table_driven_crc(0xFFFFFFFF, p + CHUNK_NAME, len);
            store_long((UINT8*)(p + CHUNK_CRC), crc);

            ret = TRUE;
            UU_PRINTF("%s: Save OUC param Done\n",__FUNCTION__);
        }
        else
        {
            UU_PRINTF("%s: Cannot find param chunk\n",__FUNCTION__);
        }
    }
    else
    {
        UU_PRINTF("%s: Cannot find ouc main chunk\n",__FUNCTION__);
    }
    return ret;    
}
#endif


static UINT32 find_chunk_addr(UINT32 ck_id,UINT32 mem_addr)
{
    UINT32 chunk_addr = 0;
    UINT32 chunk_addr_b = 0;
    
    chunk_addr = (UINT32)sto_chunk_goto(&ck_id, 0xFFFFFFFF, 1);
    chunk_addr_b = mem_addr - code_start_addr + code_start_offset;
    if ((UINT32)ERR_FAILUE == chunk_addr)
    {
        if(mem_addr)
            chunk_addr = chunk_addr_b;
        else
        {
            UU_PRINTF("Cannot find Chunk(0x%08x) in flash !\n",ck_id);
            return INVALID_CK_ADDR;
        }
    }
    else if(chunk_addr != chunk_addr_b)
    {
        UU_PRINTF("Error:Different Mapping between addr in flash and addr in memory,Chunk(0x%08x) !\n",ck_id);       
        return INVALID_CK_ADDR;
    }

    return chunk_addr;
}

//VMX or VMX+SMI
#define WIN_USB_BURNFLASH_BC win_usb_burnflash
static BOOL WIN_USB_BURNFLASH_BC(struct usbupg_ver_info ver_info)
{
    //win_popup_choice_t choice=WIN_POP_CHOICE_NULL;
    UINT8 back_saved= 0;
    //struct sto_device *flash_dev = NULL;
    UINT32 chunk_id __MAYBE_UNUSED__= 0;
    UINT32 chunk_addr= 0;
    UINT32 chunk_size= 0;

    UINT32 upgrade_size = 0;
#ifdef _BUILD_LOADER_COMBO_	
    CHUNK_HEADER blk_header;	    
    UINT32 flash_total_code_size=0;
#endif
#ifdef _SMI_ENABLE_
    UINT32 rsakey_backup_addr __MAYBE_UNUSED__= 0;
#endif

    //can upgrade all version in usb upgrade
    //if((ver_info.m_m_ver >= ver_info.m_f_ver)&&(ver_info.s_m_ver >= ver_info.s_f_ver))
    {
        upgrade_size += main_code_size;
        upgrade_size += see_code_size;
    }
#ifdef _SMI_ENABLE_
	//if(ver_info.rsa_m_ver > ver_info.rsa_f_ver) //ca switching, update rsa key and ECGK
	if(ca_switch_mode==1)
	{
        #ifndef _BUILD_USB_LOADER_
		upgrade_size += rsa_key_size; //backup.
		upgrade_size += ecgk_key_size;
        #endif
        upgrade_size += rsa_key_size;//upgrade.
		upgrade_size += ecgk_key_size;
	}
#endif
    if (upgrade_size == 0)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Nothing need to be upgrade!", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        win_compopup_open_ext(&back_saved);
        return FALSE;
    }

#ifndef _BUILD_OTA_E_ 
    if(check_main_see_size(main_code_size + see_code_size))
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

#ifndef _BUILD_USB_LOADER_    
    if((INVALID_CK_ADDR == find_chunk_addr(MAINCODE_ID,main_code_addr)) 
#ifdef _OUC_LOADER_IN_FLASH_         
        ||(INVALID_CK_ADDR ==find_chunk_addr(OUC_CHUNK_ID,loader_main_code_addr - CHUNK_HEADER_SIZE)) 
#endif        
#ifdef _SMI_ENABLE_
        || (INVALID_CK_ADDR == find_chunk_addr(RSA_KEY_CHUNK_ID,rsa_key_addr))
#endif        
    )
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Flash mapping don't match", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        win_compopup_open_ext(&back_saved);
        return FALSE;
    }          
#endif   
    
#ifndef _BUILD_LOADER_COMBO_	
#ifndef _BUILD_USB_LOADER_
#ifdef _OUC_LOADER_IN_FLASH_    
    upgrade_size += loader_main_code_size;
    upgrade_size += loader_see_code_size;
    upgrade_size += 0x10000;//ota param.
#else
#ifdef BACKUP_USB_LOADER
    upgrade_size += usb_loader_bin_size;
    upgrade_size += user_db_len;
#endif
#endif
#else
#ifndef _OUC_LOADER_IN_FLASH_ 
    upgrade_size += user_db_len;
#endif
#endif
#endif
    upg_total_sectors = upgrade_size / C_SECTOR_SIZE;
    upg_secotrs_index = 0;
    init_storage();

    UINT32 flash_pos= 0;
    UINT32 ret= 0;

#ifdef BACKUP_USB_LOADER
#ifndef _BUILD_USB_LOADER_
#ifndef _OUC_LOADER_IN_FLASH_
    UU_PRINTF("BackupDataBase!\n");
    ret = backup_database();
    if (SUCCESS != ret)
        return FALSE;

    UU_PRINTF("Burn USBLOADER!\n");
    UU_PRINTF("burn usb_loader_id=0x%x 0x%x\n",*((UINT8 *)usb_loader_code_addr),*((UINT8 *)usb_loader_code_addr+1));
    ret = burn_block((UINT8 *)usb_loader_code_addr, user_db_start_sector, usb_loader_bin_sectors);
    if(SUCCESS != ret)
        return FALSE;
#else
    UU_PRINTF("Burn USBLOADER!\n");
    //loader main is in the ouc chunk, you need to skip the chunk header.
    chunk_id = OUC_CHUNK_ID;
    chunk_addr = find_chunk_addr(OUC_CHUNK_ID,loader_main_code_addr - CHUNK_HEADER_SIZE); 
    chunk_addr += CHUNK_HEADER_SIZE;
    flash_pos = chunk_addr / C_SECTOR_SIZE ;
    chunk_size = loader_main_code_size + loader_see_code_size + 0x10000 ;
    ret = burn_block((UINT8 *)loader_main_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);    
#endif    

#ifdef _SMI_ENABLE_	
	// / TODO:  burn RSA key and ECGK to UserDB last sector
	#ifndef NEW_USBUPG_FLOW
	UU_PRINTF("Burn RSA key and ECGK  to DB!\n");  
	if( usb_loader_bin_sectors+2 > user_db_sectors)
	{
		UU_PRINTF("Backup RSA key and ECGK Fail!\n");  
		return FALSE;
	}
    rsakey_backup_addr = user_db_start_sector+user_db_sectors-2;
    #else    
    rsakey_backup_addr = SMI_KEY_BACKKUP_ADDR;
    UU_PRINTF("Backup RSA key and ECGK  to Addr (0x%08x)!\n",rsakey_backup_addr );  
    #endif
    flash_pos = rsakey_backup_addr / C_SECTOR_SIZE;
	ret = burn_block((UINT8*)rsa_key_addr, flash_pos , 1);
	if(SUCCESS != ret)
        return FALSE;
	ret = burn_block((UINT8*)ecgk_key_addr, flash_pos + 1 , 1);
	if(SUCCESS != ret)
        return FALSE;
#endif
	
#endif
#endif

#ifdef _SMI_ENABLE_
	// /TODO:  calculate  RSA key and ECGK chunk addr
	//if(ver_info.rsa_m_ver > ver_info.rsa_f_ver) //ca switching, update rsa key
	if(ca_switch_mode == 1)
	{
		//upgrade RSA key
		chunk_id = RSA_KEY_CHUNK_ID;
        chunk_addr = find_chunk_addr(RSA_KEY_CHUNK_ID,rsa_key_addr);		
		// burn new rsa key
		chunk_size = rsa_key_size;
		UU_PRINTF("upgrade rsa key\n");
		flash_pos = chunk_addr / C_SECTOR_SIZE;
		ret = burn_block((UINT8 *)rsa_key_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
		if (SUCCESS != ret)
			return FALSE;

		//upgrade ECGK
		chunk_addr += chunk_size;
		// burn new ecgk
		chunk_size = ecgk_key_size;
		UU_PRINTF("upgrade ecgk\n");
		flash_pos = chunk_addr / C_SECTOR_SIZE;
		ret = burn_block((UINT8 *)ecgk_key_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
		if (SUCCESS != ret)
			return FALSE;
	}
#endif

#ifndef NEW_USBUPG_FLOW
#if defined(_SMI_ENABLE_) 
	if(ca_switch_mode == 0)
	{
		chunk_id = MAINCODE_ID;
		chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
		if ((UINT32)ERR_FAILUE == chunk_addr)
		{
			//find the prev chunk,and the calculate maincode chun addr		
			chunk_id = ECGK_CHUNK_ID;
			chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
			chunk_size = ecgk_key_size;
			chunk_addr += chunk_size;
		}	
	}
	else
	{
		chunk_addr += chunk_size;
	}
#else
    chunk_id = MAINCODE_ID;
    chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if ((UINT32)ERR_FAILUE == chunk_addr)
    {
        UU_PRINTF("Cannot find maincode's chunk!\n");
        return FALSE;
    }
#endif

#ifdef _BUILD_LOADER_COMBO_
    chunk_id = SEECODE_ID;
    chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);

    if(sto_get_chunk_header(chunk_id, &blk_header))
    {
       	flash_total_code_size += blk_header.offset;
		UU_PRINTF("SEE size in flash =0x%x\n",flash_total_code_size);
    }		
#endif

    chunk_id = MAINCODE_ID;
    chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
#ifdef _BUILD_LOADER_COMBO_
    if(sto_get_chunk_header(chunk_id, &blk_header)&&flash_total_code_size!=0)
    {
       	flash_total_code_size += blk_header.offset;
		UU_PRINTF("MAIN+SEE size in flash =0x%x\n",flash_total_code_size);
		UU_PRINTF("Found MAIN/SEE code size in flash\n");

		if(upgrade_size>flash_total_code_size)
		{
             win_compopup_init(WIN_POPUP_TYPE_OK);
        	 win_compopup_set_msg("Upgrade code size is too big, cannot do upgrade", NULL, 0);
        	 win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        	 win_compopup_open_ext(&back_saved);

	 		 loader_check_run_parameter();
	 		 loader_set_run_parameter(0,1);
	 		 loader_check_run_parameter();
			 flash_total_code_size=0;

    		 win_compopup_init(WIN_POPUP_TYPE_OKNO);
    		 win_compopup_set_msg("Press Yes to reboot!", NULL, 0);

    		 if(WIN_POP_CHOICE_YES == win_compopup_open_ext(&back_saved))
    		  	 hw_watchdog_reboot();
			
        	 return FALSE;
		}	
    }		
#endif	
    if ((UINT32)ERR_FAILUE == chunk_addr)
    {
        UU_PRINTF("Cannot find maincode's chunk!\n");
#ifdef _BUILD_LOADER_COMBO_
    	chunk_id = OTA_PARAMETER_ID;
    	chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);

    	if(sto_get_chunk_header(chunk_id, &blk_header))
         	chunk_addr=chunk_addr+blk_header.offset;
      else	    
#endif
        return FALSE;
    }
#else
    //new flow.
    chunk_id = MAINCODE_ID;
    chunk_addr = find_chunk_addr(MAINCODE_ID,main_code_addr);    
#endif
    // burn new maincode.bin
    // main code chunk addr and size
    chunk_size = main_code_size;

    UU_PRINTF("upgrade main code\n");
    flash_pos = chunk_addr / C_SECTOR_SIZE;
    ret = burn_block((UINT8 *)main_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
    if (SUCCESS != ret)
        return FALSE;

    // see code chunk addr and size
    chunk_addr += chunk_size;
    chunk_size = see_code_size;

    UU_PRINTF("upgrade see code\n");
    flash_pos = chunk_addr / C_SECTOR_SIZE;
    ret = burn_block((UINT8 *)see_code_addr, flash_pos, chunk_size/C_SECTOR_SIZE);
    if (SUCCESS != ret)
        return FALSE;
    
#ifndef _OUC_LOADER_IN_FLASH_    
#ifdef BACKUP_USB_LOADER
#ifndef _BUILD_USB_LOADER_
    // TODO: burn back UserDB
    UU_PRINTF("BurnBack UserDB!\n");
    ret = burn_block((UINT8*)backup_db_addr, user_db_start_sector, user_db_sectors);
    if(SUCCESS != ret)
        return FALSE;
#else
#ifndef _BUILD_LOADER_COMBO_
    UU_PRINTF("reset UserDB!\n");
    ret = burn_block((UINT8*)NULL, user_db_start_sector, user_db_sectors);
    if(SUCCESS != ret)
        return FALSE;
#else
#endif
#endif
#endif
#endif

    return TRUE;
}
#undef WIN_USB_BURNFLASH_BC

#endif
#ifdef _BUILD_LOADER_COMBO_
static PRESULT usbupg_switch_to_otaupg(void)
{
    PRESULT ret = PROC_LOOP;
    POBJECT_HEAD cur_menu, new_menu;

    cur_menu = (POBJECT_HEAD)(&win_usbupg_con);
    new_menu = (POBJECT_HEAD)(&win_otaupg_con);

     
    menu_stack_pop();
    if(osd_obj_open(new_menu,(UINT32)(~0)) != PROC_LEAVE)
        menu_stack_push(new_menu);

    return ret;
}

static PRESULT     usbupg_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;

    switch(act)
    {
        case VACT_SWITCH_TO_OTAUPG:
            ret = usbupg_switch_to_otaupg();
            break;
        default:
            break;
    }

    return ret;
}
#endif

#ifdef _NV_PROJECT_SUPPORT_
static PRESULT usbupg_switch_to_otaupg(void)
{
    PRESULT ret = PROC_LOOP;
    POBJECT_HEAD cur_menu, new_menu;

    cur_menu = (POBJECT_HEAD)(&win_usbupg_con);
    new_menu = (POBJECT_HEAD)(&win_otaupg_con);

    osd_obj_close(cur_menu,0);
    menu_stack_pop();
    if(osd_obj_open(new_menu,(UINT32)(~0)) != PROC_LEAVE)
        menu_stack_push(new_menu);

    return ret;
}

PRESULT usbupg_switch_to_dvb(void)
{
    UINT8 back_saved;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg("Switch to DVB", NULL, NULL);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(3000);
    win_compopup_smsg_restoreback();

    dvb_upgloader_switch(SWITCH_TO_DVB, UPGTYPE_USB);

    return SUCCESS;
}

static PRESULT usbupg_switch_to_upgloader(void)
{
    UINT8 back_saved;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext("Switch to UPGLoader", NULL, NULL);

#ifndef SD_UI
    win_compopup_set_frame(290, 250, 448, 100);
#else
    win_compopup_set_frame(150, 200, 300, 80);
#endif

    win_compopup_open_ext(&back_saved);
    osal_task_sleep(3000);
    win_compopup_smsg_restoreback();

    dvb_upgloader_switch(SWITCH_TO_UPGLOADER, UPGTYPE_USB);

    return SUCCESS;
}


static PRESULT     usbupg_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;

    switch(act)
    {
        case VACT_SWITCH_TO_OTAUPG:
            ret = usbupg_switch_to_otaupg();
            break;
        case VACT_SWITCH_TO_DVB:
            ret = usbupg_switch_to_dvb();
            break;
        default:
            break;
    }

    return ret;
}

static void usbupg_end_process(char* popup_str)
{
    UINT8 back_saved;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(popup_str, NULL, NULL);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(3000);
    win_compopup_smsg_restoreback();

    usbupg_switch_to_dvb();
}


static INT32 init_storage()
{
    m_StoDevice = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if(NULL == m_StoDevice)
    {
        UU_PRINTF("%s: can not get sto device\n", __FUNCTION__);
        return ERR_NO_DEV;
    }

    INT32 er = sto_open(m_StoDevice);
    if(SUCCESS != er)
    {
        UU_PRINTF("%s: can not open sto device\n", __FUNCTION__);
    }


    return er;
}

static INT32 burn_block(UINT8 *pBuffer, UINT32 Pos, UINT8 NumSectors)
{
    struct sto_device *sto_dev = m_StoDevice;
    UINT32 param[2];
    INT32 offset, er = SUCCESS;
    UINT8 i;

    for(i=0; i<NumSectors; i++)
    {
        offset = (Pos + i) * C_SECTOR_SIZE;
        param[0] = (UINT32)offset;
        param[1] = C_SECTOR_SIZE >> 10; // length in K bytes
        UU_PRINTF("%s: erase sector %d\n", __FUNCTION__, Pos+i);
        er = sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);
        if(er != SUCCESS)
        {
            UU_PRINTF("%s: erase sector %d failed\n", __FUNCTION__, i);
            break;
        }

        if(pBuffer)
        {
            UU_PRINTF("%s: seek sector %d, offset = %xh\n", __FUNCTION__, Pos+i,offset);
            if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
            {
                UU_PRINTF("%s: seek sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }

            UU_PRINTF("%s: write sector %d, src = %xh\n", __FUNCTION__, Pos+i,&pBuffer[C_SECTOR_SIZE * i]);
            if(sto_write(sto_dev, &pBuffer[C_SECTOR_SIZE * i], C_SECTOR_SIZE) != C_SECTOR_SIZE)
            {
                UU_PRINTF("%s: write sector %d failed\n", __FUNCTION__, i);
                er = ERR_FAILED;
                break;
            }
        }
        else
        {
            UU_PRINTF("%s: Error in Line(%d), Please check !!!\n",__FUNCTION__, __LINE__);
        }
        upg_secotrs_index ++;
        win_usbupg_process_update(upg_secotrs_index*100/upg_total_sectors);
        UU_PRINTF("%s: %d -%d ,Percent(%d) \n",__FUNCTION__, \
            upg_secotrs_index,upg_total_sectors,upg_secotrs_index*100/upg_total_sectors);
    }
    return er;
}

static INT32 verify_block(UINT8 *pBuffer, UINT32 Pos, UINT8 NumSectors)
{
    struct sto_device *sto_dev = m_StoDevice;
    UINT8 *read_buf = NULL;
    INT32 offset = 0;
    UINT8 i;

    read_buf = MALLOC(C_SECTOR_SIZE);
    if(NULL == read_buf)
    {
        goto ERR;
    }
    UU_PRINTF("[%s(%d)] read_buf addr=0x%x\n",__FUNCTION__,__LINE__, read_buf);

    for(i=0; i<NumSectors; i++)
    {
        offset = (Pos + i) * C_SECTOR_SIZE;
        if(sto_lseek(sto_dev, offset, STO_LSEEK_SET) != offset)
        {
            UU_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
            goto ERR;
        }

        if(sto_read(sto_dev, (UINT8 *)read_buf, C_SECTOR_SIZE) != C_SECTOR_SIZE)
        {
            UU_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
            goto ERR;
        }

        if(0 != MEMCMP(read_buf, &pBuffer[C_SECTOR_SIZE * i], C_SECTOR_SIZE))
        {
            UU_PRINTF("[%s(%d)]\n",__FUNCTION__,__LINE__);
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
    UU_PRINTF("[%s(%d)] verify_block fail\n",__FUNCTION__,__LINE__);
    free(read_buf);
    return ERR_FAILED;

}

static BOOL win_usb_burnflash(UINT8 *buffer_adr,UINT32 buffer_size)
{
    win_popup_choice_t choice;
    UINT8 back_saved;
    int id = 0;
    UINT32 chunk_size;
    UINT32 chunk_addr;
    UINT32 flash_pos;
    UINT32 ret;
    int da2_update_retry_cnt = 2;
    int da2_update_done = 0; /* 0:not done; 1:done */
    block_info_t block_infor;
    block_info_t sto_block_infor;

    UU_PRINTF("[%s(%d)] buffer_adr=0x%x, buffer_size=0x%x\n", __FUNCTION__,__LINE__, buffer_adr, buffer_size);

    upg_total_sectors = upgrade_size / C_SECTOR_SIZE;
    upg_secotrs_index = 0;
    UU_PRINTF("[%s(%d)] upg_total_sectors=%d\n", __FUNCTION__,__LINE__, upg_total_sectors);
    init_storage();

    //update DA2
    block_info_get(FLASH_BLK_DA2, &block_infor);
    sto_block_info_get(FLASH_BLK_DA2, &sto_block_infor);
    chunk_size = sto_block_infor.block_len;
    chunk_addr = sto_block_infor.block_start;
    flash_pos = chunk_addr / C_SECTOR_SIZE;
    #if 0
    if(TRUE == is_da2_change(block_infor, sto_block_infor))
    #else
    if(TRUE == is_block_data_change(block_infor, sto_block_infor))
    #endif
    {
        UU_PRINTF("[%s(%d)] chunk_size=0x%x, chunk_addr=0x%x, flash_pos=0x%x\n",
                __FUNCTION__,__LINE__, chunk_size, chunk_addr, flash_pos);
        while(da2_update_retry_cnt--)
        {
            UU_PRINTF("[%s(%d)] update DA2 da2_update_retry_cnt=%d\n", __FUNCTION__,__LINE__, da2_update_retry_cnt);

            if(SUCCESS != burn_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE))
            {
                UU_PRINTF("[%s(%d)] burn DA2 fail\n", __FUNCTION__,__LINE__);
                continue;
            }

            if(SUCCESS != verify_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE))
            {
                UU_PRINTF("[%s(%d)] verify DA2 fail\n", __FUNCTION__,__LINE__);
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
            UU_PRINTF("[%s(%d)] DA2 update fail\n", __FUNCTION__,__LINE__);
            return FALSE;
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
        UU_PRINTF("[%s(%d)] id=%d, block_infor.version=%d, sto_block_infor.version=%d\n",
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
            UU_PRINTF("[%s(%d)] chunk_size=0x%x, chunk_addr=0x%x, flash_pos=0x%x\n",
                __FUNCTION__,__LINE__, chunk_size, chunk_addr, flash_pos);
            ret = burn_block((UINT8 *)block_infor.block_start, flash_pos, chunk_size/C_SECTOR_SIZE);
            if (SUCCESS != ret)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}
#endif

#ifdef CI_PLUS_SUPPORT
static BOOL usbupg_maincode_check(UINT8 *pbuff, UINT32 length)
{
    BOOL ret = TRUE;
    UINT32 data_addr = 0;
    UINT32 data_len = 0;

    if ((NULL == pbuff) ||( 0 == length))
    {
        ASSERT(0);
        return FALSE;
    }

    // Even upgrade app.bin, usb_readfile still add CHUNK_HEADER structure to pbuff
    // and input length is chunk offset
    // & build a total full chunk, so need to fetch raw data & len
    data_addr = (UINT32)pbuff + CHUNK_HEADER_SIZE;
    data_len = fetch_long(pbuff + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
    if (0 != test_rsa_ram(data_addr, data_len))
    {
        ret = FALSE;
        return ret;
    }

    return ret;
}

static BOOL usbupg_seecode_check(UINT8 *pbuff, UINT32 length)
{
    BOOL ret = TRUE;
    UINT32 data_addr = 0;
    UINT32 data_len = 0;

    if ((NULL == pbuff) ||( 0 == length))
    {
        ASSERT(0);
        return FALSE;
    }

    data_addr = (UINT32)pbuff+ CHUNK_HEADER_SIZE;
    data_len = fetch_long(pbuff + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
    if (0 != test_rsa_ram(data_addr, data_len))
    {
        ret = FALSE;
        return ret;
    }

    return ret;
}

static BOOL usbupg_allcode_check(UINT8 *pbuff, UINT32 length)
{
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_offset = 0;
    UINT8 bl_signed[256] = {0xFF};
    UINT32 check_cnt = 0;
    BOOL ret = TRUE; // default TRUE, means check success!
    UINT32 chunk_id = 0;
    UINT8 *data_addr = NULL;
    UINT32 data_len = 0;
    CHUNK_HEADER chunk_header;

    if ((NULL == pbuff) ||( 0 == length))
    {
        ASSERT(0);
        return FALSE;
    }

    // step 1: bootloader check
    chunk_init(pbuff, length);
    // step 2: maincode check
    // get maincode raw data address & length
    chunk_id = MAINCODE_ID;
    if(!get_chunk_header(chunk_id, &chunk_header))
    {
        ret = FALSE;
        return ret;
    }
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    data_len = chunk_header.len;
    if (FALSE == usbupg_maincode_check((UINT32)data_addr, data_len)) // this function operate RAW Data and RAW length
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
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    data_len = chunk_header.len;
    if (FALSE == usbupg_seecode_check((UINT32)data_addr, data_len))
    {
        ret = FALSE;
        return ret;
    }

    return ret;
}
#endif
static BOOL usbupg_pretreat(UINT16* block_idx,USBUPG_BLOCK_TYPE usbupg_type,\
    __MAYBE_UNUSED__ LPVSCR apvscr,UINT16* file_idx)
{
    UINT16 popup_strid =0;
    INT32 vscr_idx= 0;
    PMULTISEL pmsel= NULL;
    UINT8 back_saved= 0;

    win_usbupg_msg_clear();
    win_usbupg_process_update(0);
    vscr_idx = osal_task_get_current_id();
    if( INVALID_ID == vscr_idx)
    {
        return FALSE;
    }
    apvscr = osd_get_task_vscr(vscr_idx);

    if(0 == usbupg_files_cnt)//no file selected to upgrade
    {
        popup_strid = RS_USBUPG_NO_MATCH_FILE;
    }
    else
    {
        popup_strid = RS_USBUPG_ALERT_NOPLUG;
    }
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(NULL,NULL, popup_strid);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(2000);
    win_compopup_smsg_restoreback();

    if(0 == usbupg_files_cnt)
    {
        return FALSE;
    }
    osd_set_attr(&usbupg_item_con1, C_ATTR_INACTIVE);
    osd_set_attr(&usbupg_item_con2, C_ATTR_INACTIVE);
    osd_set_attr(&usbupg_item_con3, C_ATTR_INACTIVE);
    osd_draw_object( (POBJECT_HEAD)&usbupg_item_con1, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&usbupg_item_con2, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&usbupg_item_con3, C_UPDATE_ALL);

    pmsel = &usbupg_sel_upgrade_type;
    if(0 ==osd_get_multisel_sel(pmsel))
    {
        *block_idx = 0;
    }
    else
    {
        *block_idx = osd_get_multisel_sel(pmsel)+ 1;
    }
    pmsel = &usbupg_filelist_sel;
    *file_idx = osd_get_multisel_sel(pmsel);

    usbupg_type = usbupg_get_block_type(*block_idx);
    if((USBUPG_ALLCODE == usbupg_type) || (USBUPG_BOOTLOADER == usbupg_type))
    {
#ifndef CI_PLUS_SUPPORT
        // if we do AllCode/BootLoader Upgrade, we need to upgrade bootloader chunk
        set_upgrade_bloader_flag(1);
#else
        // Oncer: for CI+, do not allow to upgrade bootloader
        set_upgrade_bloader_flag(0);
#endif        
    }
    else
    {
        set_upgrade_bloader_flag(0);
    }
#if (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_GEN_CA_ENABLE_) )
    set_upgrade_bloader_flag(0);
#endif

#ifdef _NV_PROJECT_SUPPORT_
    set_upgrade_bloader_flag(1);
#endif

    return TRUE;
}

#if(!(defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)))
static BOOL usbupg_allcode_pretreat(void *pbuff,INT32 length)
{
#ifdef HDCP_IN_FLASH
    UINT32 chunk_id = 0;//HDCPKEY_CHUNK_ID;
#endif

#ifdef CI_PLUS_SUPPORT
    // CI+ FW upgrade:
    // step 1: we need to do FW signed check
    // step 2: we need to backup CI+ key
    if (FALSE == usbupg_allcode_check((UINT8 *)pbuff, (UINT32)length))
        return FALSE;

    // backup CI+ Key
    chunk_id = CIPLUSKEY_CHUNK_ID;
    if (!backup_chunk_data(pbuff, length, chunk_id))
        return FALSE;
#endif /* End of FW Signed Check & All Code Backup CI Plus Key Chunk */

#ifdef HDCP_IN_FLASH
    // backup HDCP Key which stored in flash
    chunk_id = HDCPKEY_CHUNK_ID;
    if (!backup_chunk_data(pbuff, length, chunk_id))
        return FALSE;
#endif/* End of all code backup HDCP Key chunk */

    return TRUE;
}

static int usbupg_maincode_pretreat(void *pbuff,INT32 length,__MAYBE_UNUSED__ UINT16 popup_strid)
{
// 1:UPDATE_FAIL  2: ERROR_HANDLER 0: OK
#ifdef _COMPRESS_LZO
    UINT32 lzo_len1= 0;
    UINT32 lzo_len2= 0;
#endif

#ifdef CI_PLUS_SUPPORT
    // maincode signature check
    if (FALSE == usbupg_maincode_check((UINT8 *)pbuff, (UINT32)length))
        return 1;
#endif

#ifdef _COMPRESS_LZO
    lzo_len1 = *(UINT32 *)(pbuff + CHUNK_HEADER_SIZE);    //after compress len
    lzo_len2 = *(UINT32 *)(pbuff + CHUNK_HEADER_SIZE+4);    // before compress len

    //lzo: after compress len / before compress len may between 35~49
    if( lzo_len1 == MAINCODE_ZIP_HEAD
        ||lzo_len2 == 0
        ||lzo_len1*100 /lzo_len2  > 49
        || lzo_len1*100 /lzo_len2  < 35)
    {
        UU_PRINTF("%s-%d got RS_MSG_UPGRADE_INVALID_FILE\n",__FUNCTION__,__LINE__);
        popup_strid = RS_MSG_UPGRADE_INVALID_FILE;
        return 2;
    }
#else
    if (*(UINT32 *)(pbuff + CHUNK_HEADER_SIZE) != MAINCODE_ZIP_HEAD)
    {
        UU_PRINTF("%s-%d got RS_MSG_UPGRADE_INVALID_FILE\n",__FUNCTION__,__LINE__);
        popup_strid = RS_MSG_UPGRADE_INVALID_FILE;
        return 2;
    }
#endif
    return 0;
}
#endif

#ifdef HDCP_IN_FLASH
static void usbupg_hdcp_handle(UINT16 block_idx)
{
    // delete programmed HDCP Key file.
    USBUPG_FILENODE filenode;
    char dir_str[USBUPG_FILE_NAME_LENGTH+1]={0};
    UINT16 file_idx= 0;
    PMULTISEL pmsel= NULL;
    int fs_ret = 0;
    
    pmsel = &usbupg_filelist_sel;
    file_idx = osd_get_multisel_sel(pmsel);
    usbupg_get_filenode(&filenode,file_idx);
    snprintf(dir_str,USBUPG_FILE_NAME_LENGTH+1, "/c/%s",filenode.upg_file_name);
    fs_ret = fs_remove(dir_str);
    if(fs_ret < 0)
    {
        return;
    }
    fs_ret = fs_sync("/c/");
    if(fs_ret < 0)
    {
        return;
    }
    usbupg_create_filelist(block_idx,&usbupg_files_cnt);
    pmsel = &usbupg_filelist_sel;
    osd_set_multisel_sel(pmsel, 0);
    osd_set_multisel_count(pmsel, (usbupg_files_cnt));
    osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
    key_pan_display("OFF ",4);
    osal_task_sleep(500);
    power_off_process(0);
    power_on_process();

}
#endif
static void usbupg_error_handle(UINT16 popup_strid,LPVSCR apvscr)
{
    UINT8 back_saved= 0;

    osd_set_attr(&usbupg_item_con1, C_ATTR_ACTIVE);
    osd_set_attr(&usbupg_item_con2, C_ATTR_ACTIVE);
    osd_set_attr(&usbupg_item_con3, C_ATTR_ACTIVE);
    osd_draw_object( (POBJECT_HEAD)&usbupg_item_con1, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&usbupg_item_con2, C_UPDATE_ALL);
    osd_track_object( (POBJECT_HEAD)&usbupg_item_con3, C_UPDATE_ALL);
    osd_update_vscr(apvscr);

    if(popup_strid != 0)
    {
        win_usbupg_msg_update(popup_strid);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL,NULL, popup_strid);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
    }
}

static void usbupg_sub_process(BOOL burnflash_ret,USBUPG_BLOCK_TYPE usbupg_type,\
    __MAYBE_UNUSED__ UINT16 popup_strid,UINT16 block_idx)
{
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
    UINT8 back_saved;
#endif

    if(burnflash_ret == TRUE)
    {
        popup_strid = RS_MSG_UPGRADE_SUCCESS;
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext(NULL, NULL, popup_strid);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
#endif
        if((USBUPG_ALLCODE == usbupg_type)|| (USBUPG_MAINCODE == usbupg_type)
#ifdef USBUPG_MEMCFG_SUPPORT
            || (USBUPG_MEMCFG == usbupg_type)
#endif
            )
        {
#ifdef BACKUP_TEMP_INFO
            save_swap_data(&system_config,sizeof(system_config));
#endif
            key_pan_display("OFF ",4);
            osal_task_sleep(500);
            power_off_process(0); 
#ifndef SUPPORT_BC_STD
            if (USBUPG_ALLCODE == usbupg_type)
            {
                sys_data_factroy_init();
                sys_data_set_factory_reset(TRUE);
                sys_data_set_cur_chan_mode(TV_CHAN);
            }
#endif
            power_on_process();
        }
        else if(USBUPG_LOGO == usbupg_type)
        {
            api_show_menu_logo();
        }
#ifdef HDCP_IN_FLASH
        else if(USBUPG_HDCPKEY == usbupg_type)
            usbupg_hdcp_handle(block_idx);
#endif
    }
    else
    {
#if defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_GEN_CA_ENABLE_)
        popup_strid = 0;
#else
        popup_strid = RS_MSG_UPGRADE_BURN_FLASH_ERROR;
#endif
    }
}

static PRESULT usbupg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT8 bid= 0;
    UINT8 back_saved= 0;
    UINT16 block_idx= 0;
    UINT16 file_idx= 0;
    UINT16 popup_strid= 0;
    INT32 length= 0;
    INT32 fileread_ret= 0;
    char str_buff[20];
    void *pbuff = NULL;
    BOOL burnflash_ret= FALSE;
    VACTION unact =VACT_PASS;
    PRESULT ret =PROC_PASS;
    LPVSCR apvscr= NULL;
    win_popup_choice_t choice =WIN_POP_CHOICE_NULL;
#if (defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_CAS9_CA_ENABLE_) || defined(_GEN_CA_ENABLE_))
    struct usbupg_ver_info ver_info;
#endif
#if(!(defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)))
    int result __MAYBE_UNUSED__=0;
#endif
    USBUPG_BLOCK_TYPE usbupg_type=USBUPG_ALLCODE;

    ret = PROC_PASS;
    bid = osd_get_obj_id(pobj);
    if( EVN_UNKNOWN_ACTION == event)
    {
        unact = (VACTION)(param1>>16);
        if((VACT_ENTER==unact) && (START_ID==bid))
        {
            burnflash_ret = usbupg_pretreat(&block_idx, usbupg_type, apvscr, &file_idx);
            if(!burnflash_ret)
            {
                return ret;
            }
            #if (defined(_CAS9_CA_ENABLE_)|| defined(_GEN_CA_ENABLE_))
            get_boot_total_area_len(&boot_len);
            #endif
            
            UU_PRINTF("[%s(%d)] block_idx=%d, file_idx=%d\n", __FUNCTION__, __LINE__, block_idx, file_idx);
            usbupg_get_blockinfo(block_idx,str_buff,(UINT32 *)&length);//get chunk length

#if (defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_NV_PROJECT_SUPPORT_) || defined(_GEN_CA_ENABLE_))
            pbuff = (void *)(__MM_PVR_VOB_BUFFER_ADDR | 0xa0000000);//should use un cached addr
        #ifdef BACKUP_USB_LOADER
            length=READ_USBUPG_BUFFER_SIZE;
        #endif
#else
            pbuff = (void *)(__MM_PVR_VOB_BUFFER_ADDR & 0x8fffffff);//have included chunk_head info size
#endif
//        if(pbuff != NULL)
            fileread_ret = usbupg_readfile(block_idx,file_idx,pbuff,length,\
                                     (usbupg_prgrs_disp)win_usbupg_process_update);
            UU_PRINTF("[%s(%d)] get 0x%x from usbupg_readfile() to address 0x%x\n",
                        __FUNCTION__,__LINE__,fileread_ret, pbuff);
#ifdef _NV_PROJECT_SUPPORT_
            if((USBUPG_FILEREAD_NO_ERROR == fileread_ret) || (fileread_ret > 0))
            {
                length = fileread_ret;
                
                #if 1
                init_upg_bock((UINT8 *)pbuff, (UINT32)length, &upgrade_size);
                if(0 == upgrade_size)
                {
                    usbupg_end_process("no data need upgrade!");
                    return ret;
                }
                #endif

                #if 1
                if(SSD_CHECK_VALID_NO_ERROR != upg_ssd_check((UINT8*)pbuff, (UINT32)length, \
                                    NULL, NULL))
                {
                    usbupg_end_process("invalid file!");
                    return ret;
                }
                #endif

                win_usbupg_process_update(0);
                UU_PRINTF("[%s(%d)] going to burn flash\n", __FUNCTION__,__LINE__);
                burnflash_ret = win_usb_burnflash((UINT8 *)pbuff, (UINT32)length);
                if(TRUE == burnflash_ret)
                {
                    usbupg_end_process("upgrade success!");
                }
                else
                {
                    usbupg_end_process("upgrade fail!");
                }
                return ret;
            }
            else
            {
                 usbupg_end_process("invalid file!");
            }
            return ret;

#else
            if(USBUPG_FILEREAD_NO_ERROR == fileread_ret)
            {
#if (defined(_CAS9_CA_ENABLE_) || defined(_GEN_CA_ENABLE_))
                MEMSET(&ver_info, 0, sizeof(struct usbupg_ver_info));
		        win_usbupg_set_memory();
                #ifdef _NOVEL_ENABLE_
                burnflash_ret = usbupg_check_with_host_key(&ver_info,(UINT8 *)pbuff, (UINT32)length);
                #else
                burnflash_ret = usbupg_check(&ver_info,(UINT8 *)pbuff, (UINT32)length);
                #endif
                if(burnflash_ret != TRUE)
                {
                	UU_PRINTF("usbupg_item_callback : 1\n");
                    goto UPDATE_FAIL;
                }
                #ifndef _BUILD_OTA_E_
                #ifdef _OUC_LOADER_IN_FLASH_
                if(FALSE == save_upgrade_param((UINT8 *)pbuff,(UINT32)length))
                {
                    UU_PRINTF("save_upgrade_param Failed \n");
                    goto UPDATE_FAIL;
                }
                #endif
                #ifdef _CAS9_CA_ENABLE_
                if(FALSE == save_otaloader_hmac())
                {
                    UU_PRINTF("save_otaloader_hmac Failed \n");
                    goto UPDATE_FAIL;
                }      
                #endif
                #endif
                
                #ifdef _OUC_LOADER_IN_FLASH_
                if ( (0 == ver_info.m_m_ver ) ||
                    (0 == ver_info.s_m_ver ) ||
                    (ver_info.m_m_ver != ver_info.s_m_ver) ||
                    (0 == ver_info.u_m_ver) ||
                    (0 == ver_info.us_m_ver)
                    #ifdef _MOD_DYNAMIC_LOAD_
                    || (0 == ver_info.dyn_m_ver)
                    #endif
                    )
                #else
                if ( (0 == ver_info.m_m_ver ) ||
                    (0 == ver_info.s_m_ver ) ||
                    (ver_info.m_m_ver != ver_info.s_m_ver))
                #endif
                {
                     UU_PRINTF("usbupg_item_callback : 2\n");
                    goto UPDATE_FAIL;
                }
#else
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                BOOL check_ret;

                usb_upg_set_memory(pbuff,(UINT32)length);
                MEMSET(&ver_info, 0, sizeof(struct usbupg_ver_info));
                code_start_addr = (UINT32)pbuff;
                get_boot_total_area_len(&code_start_offset);
                check_ret=usbupg_check(&ver_info,(UINT8 *)pbuff, (UINT32)length);
                #ifdef _SMI_ENABLE_
                //reset the main code vendor ID
                vendor_id_select(VID_CHOOSE_VMX_VENDORID);
                #endif
                if((check_ret!=TRUE)||(ver_info.m_m_ver != ver_info.s_m_ver))
                    goto UPDATE_FAIL;
                #ifndef _BUILD_OTA_E_
                #ifdef _OUC_LOADER_IN_FLASH_
                save_upgrade_param(NULL,0x200000);
                #endif
                #endif
#else
            /* For All code e.g product_glass_s3602.abs need to backup the HDCP from flash and
                rewrite to all code image e.g. product_glass_s3602.abs                      */
                usbupg_type = usbupg_get_block_type(block_idx);
                if (USBUPG_ALLCODE == usbupg_type)
                {
                    chunk_init((unsigned long)pbuff,length);
                    if (!usbupg_allcode_pretreat(pbuff, length)){
		                UU_PRINTF("usbupg_item_callback : 3\n");			
                        goto UPDATE_FAIL;
                }
                }
                if (USBUPG_MAINCODE == usbupg_type)
                {
                    result =usbupg_maincode_pretreat(pbuff, length, popup_strid);
                    if (RET_UPDATE_FAIL == result)
                     {   
                            UU_PRINTF("usbupg_item_callback : 5\n");
                        goto UPDATE_FAIL;
                    	}
                    else if(RET_ERROR_HANDLER== result)
                    {
                    	    UU_PRINTF("usbupg_item_callback : 6\n");
                        usbupg_error_handle(popup_strid,apvscr);
                        return ret;;
                    }
                }
                if (USBUPG_SEECODE == usbupg_type)
                {
#ifdef CI_PLUS_SUPPORT
                    // seecode signature check
                    if (FALSE == usbupg_seecode_check((UINT8 *)pbuff, (UINT32)length))
                        goto UPDATE_FAIL;
#endif
                }
#endif
#endif
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg(NULL, NULL, RS_USBUPG_SURE_BURN_FLASH);
#if defined( SUPPORT_CAS7) || defined( SUPPORT_CAS9)
                #ifndef SD_UI
                win_compopup_set_pos((OSD_MAX_WIDTH - 400)>>1,150);
                #else
                win_compopup_set_pos((OSD_MAX_WIDTH - 280)>>1,100);
                #endif
#endif
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
                #ifndef SD_UI
                win_compopup_set_pos((OSD_MAX_WIDTH - 540)>>1,120);
                #else
                win_compopup_set_pos((OSD_MAX_WIDTH - 300)>>1,120);
                #endif
#endif
                choice = win_compopup_open_ext(&back_saved);
                if(WIN_POP_CHOICE_YES == choice)
                {
#if defined(_CAS9_CA_ENABLE_) || defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_) || defined(_GEN_CA_ENABLE_)
                    //flash protect off.
                    #ifdef FLASH_SOFTWARE_PROTECT 
					
		             ap_set_flash_lock_len(OTA_LOADER_END_ADDR);
                    #endif                    
                    //only burn maincode and seecode
                    win_usbupg_process_update(0);
                    burnflash_ret = win_usb_burnflash(ver_info);
                    //flash protect on.
                    #ifdef FLASH_SOFTWARE_PROTECT 
		             ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
                    #endif                    
                    if(FALSE == burnflash_ret)
                    {
                        UU_PRINTF("usbupg_item_callback : 7\n");
                        goto UPDATE_FAIL;
                    }
			        #ifdef _BUILD_LOADER_COMBO_
			        else
			        {
			            loader_check_run_parameter();
			            loader_set_run_parameter(0,1);
			            loader_check_run_parameter();

        			    win_compopup_init(WIN_POPUP_TYPE_OK);
			            win_compopup_set_msg("Burn successfully,press OK to reboot!", NULL, 0);
        			    win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        			    win_compopup_open_ext(&back_saved);
	 
			            hw_watchdog_reboot(); 
	                }
			        #endif
                    
                    #ifdef _OUC_LOADER_IN_FLASH_
                    if(TRUE == burnflash_ret)
                    {
                        UINT32 tmp = 0;
                        
                        win_usbupg_process_update(100);
                        osal_task_sleep(1000);
                        
						if(0 == tmp)
						{
							;
						}
                        dog_m3327e_attach(1);

                        /**
                        *  before enter watchdog_reboot,you MUST NOT read/write flash
                        *  as soon as possible.make sure the bus is clear.
                        */
                        osal_interrupt_disable();
                        *((volatile UINT32 *)0xb802e098)=0xc2000d03;
                        tmp = *((volatile UINT32 *)0xb802e098);
                        dog_mode_set(0, DOG_MODE_WATCHDOG, 10, NULL);
                        while(1)
                        {
                             // for cpu loop
                            ;
                        }
                        //hw_watchdog_reboot(); 
                    }
                    #endif
#else
                    win_usbupg_process_update(0);
                    win_usbupg_msg_update(RS_OTA_BURNFLASH_DO_NOT_POWER_OFF);
                    osd_update_vscr(apvscr);
                    ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_POOLING);
                    osal_task_dispatch_off();
                    burnflash_ret = usbupg_burnflash(block_idx,(UINT8 *)pbuff,(UINT32)length,\
                                                (usbupg_prgrs_disp)win_usbupg_process_update);
                    if(FALSE == burnflash_ret)
                    {
                    	 UU_PRINTF("usbupg_item_callback : 8\n");
                        goto UPDATE_FAIL;
                    }
                    osd_update_vscr(apvscr);
                    osal_task_dispatch_on();
                    ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_INTERRUPT);
#endif
                    usbupg_sub_process(burnflash_ret, usbupg_type, popup_strid, block_idx);
                }
                else
                {
                    win_usbupg_process_update(0);
                    popup_strid = 0;
                }
            }
            else if(USBUPG_FIELREAD_ERROR_OVERFLOW == fileread_ret)
            {
                popup_strid =RS_MSG_UPGRADE_DATA_LARGE;
            }
            else
            {
UPDATE_FAIL:
                UU_PRINTF("%s-%d got RS_MSG_UPGRADE_INVALID_FILE\n",__FUNCTION__,__LINE__);
                popup_strid = RS_MSG_UPGRADE_INVALID_FILE;
                ret = PROC_LOOP;
            }
            usbupg_error_handle(popup_strid,apvscr);
#endif
        }
    }
    return ret;
}

static VACTION usbupg_con_keymap(POBJECT_HEAD pobj, UINT32 vkey)
{
    VACTION act= VACT_PASS;

    switch(vkey)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
#ifdef _BUILD_UPG_LOADER_
        act = VACT_SWITCH_TO_DVB;
#else
#ifdef _BUILD_USB_LOADER_
        act = VACT_PASS;
#else
        act = VACT_CLOSE;
#endif
#endif
        break;
#ifdef _BUILD_UPG_LOADER_
    case V_KEY_SWAP:
        act = VACT_SWITCH_TO_OTAUPG;
        break;
#endif
#ifdef _BUILD_LOADER_COMBO_
    case V_KEY_SWAP:
        act = VACT_SWITCH_TO_OTAUPG;
        break;
#endif
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}
#ifdef _BC_CA_NEW_
#define OTA_FLAG_OFFSET  1024
#define OTA_PARA_CHUNKID  0x11EE0200

void update_flash_data(UINT32 offset, INT32 len, UINT8 *data)
{
    UINT8 *app_flash_buffer = NULL;
    INT32 func_ret=0;

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

BOOL loader_set_run_parameter(BOOL set,BOOL type)
{
	UINT32 chid = 0;
	UINT32 block_addr = 0;
	UINT8 *para_block = NULL;
	UINT32 para_block_len = 0;

    struct sto_device *flash_dev = NULL;
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    chid = OTA_PARA_CHUNKID;
    block_addr=sto_chunk_goto(&chid, 0xFFFF0000, 1);		
	para_block_len = sto_fetch_long((UINT32)block_addr + CHUNK_LENGTH);
	
    para_block = (UINT8 *)((0xFFFFFFF8 & (UINT32)MALLOC(para_block_len + 0xf)));
    	
    sto_get_data(flash_dev, (void *)para_block, block_addr, para_block_len);

	if (set)
	{
        *(para_block+CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET)=1;  	 //Run from loader	
	}  
	else
	{
	    *(para_block+CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET)=0;   //Run from APP 
	    *(para_block+CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET-2)=0;  //clear OSM trigger flag also
	}

	if (type)
	{
        *(para_block+CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET-1)=1;   //USB 
	}  
	else
	{
	    *(para_block+CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET-1)=0;   //OTA
	}	
	
	update_flash_data(block_addr, para_block_len, para_block);
	free(para_block);
	return SUCCESS;
}

BOOL loader_check_run_parameter()
{
	UINT32 chid = 0;
	UINT32 block_addr = 0, otaparam_addr = 0;
	INT32 func_ret=0;
	UINT8  run_flag=0;

    struct sto_device *flash_dev = NULL;
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

	// find ota parameter addr.
    chid = OTA_PARA_CHUNKID;
    block_addr=sto_chunk_goto(&chid, 0xFFFF0000, 1);

	// shift 128 bytes for skip TUNRK header, get real OTA parameter addr
	otaparam_addr = block_addr + CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET;
		
	// read data from flash
    func_ret=sto_get_data(flash_dev, &run_flag,otaparam_addr, 1);	


	otaparam_addr = block_addr + CHUNK_HEADER_SIZE+OTA_FLAG_OFFSET-1;
		
	// read data from flash
    func_ret=sto_get_data(flash_dev, &run_flag,otaparam_addr, 1);		
	return run_flag;
}
#endif

static PRESULT usbupg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
#ifdef _BUILD_USB_LOADER_
    MULTISEL *pmsel;
#else
    struct statvfs buf;
#endif
    PRESULT ret = PROC_PASS;

#if defined (_BC_CA_NEW_) && !defined (_BUILD_LOADER_COMBO_)
    UINT8   back_saved = 0;
#endif
    switch(event)
    {
        case EVN_PRE_OPEN:
#if defined (_BC_CA_NEW_) && !defined (_BUILD_LOADER_COMBO_)
    	win_compopup_init(WIN_POPUP_TYPE_OKNO);
    	win_compopup_set_msg("Press Yes to excute USB UPG !", NULL, 0);

    	if(WIN_POP_CHOICE_YES == win_compopup_open_ext(&back_saved))
    	{	
	     	loader_check_run_parameter();
	     	loader_set_run_parameter(1,1);
	     	loader_check_run_parameter();
	
    	 	hw_watchdog_reboot();
    	}
		else
		{
         	ap_send_key(V_KEY_EXIT,TRUE);	
		}  
#endif
#ifdef OTA_BUF_SHARE_FB
        vdec_stop((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV), 1, 0);
#endif
#if defined _NV_PROJECT_SUPPORT_ && !defined _BUILD_UPG_LOADER_
            /* set upg flag & save ota cfg */
            if(SUCCESS != usbupg_switch_to_upgloader())
            {
                libc_printf("switch to upgloader fail!\n");
                ret = PROC_LOOP;
                break;
            }
#else
            //epg_release();
            wincom_open_title(pobj, RS_USBUPG_UPGRADE_BY_USB, 0);
#ifdef _BUILD_UPG_LOADER_
            api_set_preview_vpo_color(TRUE);
            image_restore_vpo_rect();//TODO
            wincom_open_help(pobj, usbupg_helpinfo, HELP_CNT);
#else
	        #ifndef _BUILD_LOADER_COMBO_
            wincom_open_help(NULL, NULL, HELP_TYPE_RS232);
	        #endif
#endif
            osd_set_progress_bar_pos(&usbupg_bar_progress,(INT16)0);
            osd_set_text_field_content(&usbupg_txt_progress, STRING_NUM_PERCENT, (UINT32)0);
            ret = win_usbupg_init();
            if(PROC_LEAVE == ret)
            {
                return ret;
            }
        #if (defined(SUPPORT_BC) || defined(SUPPORT_BC_STD))
            bc_cas_run_bc_task(FALSE);
        #endif
#endif
            break;
        case EVN_POST_OPEN:
            api_set_system_state(SYS_STATE_USB_UPG);
            win_usbupg_msg_clear();
            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        case EVN_POST_CLOSE:
        #ifdef OTA_BUF_SHARE_FB
		    api_show_menu_logo(); 
        #endif
            api_set_system_state(SYS_STATE_NORMAL);
            wincom_close_title();
            wincom_close_help();
        #if (defined(SUPPORT_BC) || defined(SUPPORT_BC_STD))
            bc_cas_run_bc_task(TRUE);
        #endif
            break;
        case EVN_MSG_GOT:
            if(CTRL_MSG_SUBTYPE_CMD_EXIT == param1)
            {
                ret = PROC_LEAVE;
            }
#ifdef DISK_MANAGER_SUPPORT
            if ((CTRL_MSG_SUBTYPE_CMD_STO== param1 ) && (USB_STATUS_OVER == param2))
            {
            #ifdef _BUILD_USB_LOADER_
                usbupg_create_filelist(0,&usbupg_files_cnt);
                pmsel = &usbupg_filelist_sel;
                osd_set_multisel_sel(pmsel, 0);
                osd_set_multisel_count(pmsel, (usbupg_files_cnt));
                osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);

                POBJECT_HEAD focus_obj = osd_get_focus_object(pobj);        
                osd_track_object((POBJECT_HEAD)focus_obj,C_UPDATE_ALL);
              
            #else
                MEMSET(&buf, 0, sizeof(struct statvfs));
                // if disk used by usb upgrade detached, exit this menu
                if (fs_statvfs("/c", &buf) != 0)
                {
                    ret = PROC_LEAVE;
                }
            #endif
            }
#endif
            break;
#ifdef _BUILD_UPG_LOADER_
        case EVN_UNKNOWN_ACTION:
            ret = usbupg_unkown_act_proc((VACTION)(param1>>16));
            break;
#endif
#ifdef _BUILD_LOADER_COMBO_
        case EVN_UNKNOWN_ACTION:
            ret = usbupg_unkown_act_proc((VACTION)(param1>>16));
            break;
#endif
        default:
            break;
    }
    return ret;
}

static PRESULT win_usbupg_init(void)
{
    RET_CODE block_ret= 0;
    MULTISEL *pmsel = NULL;
    UINT16 upg_show_count=0;

#ifdef _BUILD_LOADER_COMBO_
	if(usbupg_blocks_cnt)
	{
	    return PROC_PASS;
	}	   
#endif
    usbupg_block_free();
    block_ret = usbupg_block_init(&usbupg_blocks_cnt);
    if(block_ret == RET_FAILURE)
    {
        return PROC_LEAVE;
    }

    usbupg_create_filelist(0,&usbupg_files_cnt);

    pmsel = &usbupg_sel_upgrade_type;
    osd_set_multisel_sel(pmsel, 0);
    upg_show_count=usbupg_blocks_cnt;
#ifndef _BUILD_UPG_LOADER_
#ifdef HDCP_IN_FLASH
    upg_show_count=upg_show_count-1;
#endif
    upg_show_count= upg_show_count- 2;/*except for bootloader,userdb*/
#endif
    osd_set_multisel_count(pmsel, upg_show_count);

#ifdef USBUPG_ONLY_ALLCODE
    osd_set_attr(&usbupg_item_con1, C_ATTR_INACTIVE);
    osd_change_focus((POBJECT_HEAD)&win_usbupg_con,2,C_UPDATE_ALL);
#endif

#if defined(_RETAILER_PROJECT_) && defined(_OUC_LOADER_IN_FLASH_)
    osd_set_attr(&usbupg_item_con1, C_ATTR_INACTIVE);
    osd_change_focus((POBJECT_HEAD)&win_usbupg_con,2,C_UPDATE_ALL);
#endif

    pmsel = &usbupg_filelist_sel;
    osd_set_multisel_sel(pmsel, 0);
    osd_set_multisel_count(pmsel, usbupg_files_cnt);

    usbupg_txt_start.b_x = TXTN_L_OF;

    return PROC_PASS;
}
#endif
#endif

