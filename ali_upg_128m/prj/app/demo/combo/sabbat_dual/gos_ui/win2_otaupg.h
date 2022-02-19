/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg.h
*
*    Description:   The realize of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN2_CA_OTAUPG_H_
#define _WIN2_CA_OTAUPG_H_
#include <api/libota/lib_ota.h>

#define C_SECTOR_SIZE       0x10000
#define OTA_PRINTF  libc_printf
//#define OTA_PRINTF(...)
#define OTA_UPDATE_PROGRESS   100

#define OTA_FLAG_PTN    (PROC_SUCCESS | PROC_FAILURE | PROC_STOPPED)
#define OTA_FLAG_TIMEOUT    10

#define WIN_SH_IDX	  	WSTL_WINSEARCH_03_HD//WSTL_WIN_BODYRIGHT_01_HD


#define MTXT_MSG_SH_IDX		WSTL_TEXT_08_HD
#define MTXT_SWINFO_SH_IDX	WSTL_TEXT_09_HD

#define MTXT_SH_IDX	WSTL_TEXT_08_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_HD
#define TXT_HL_IDX   WSTL_BUTTON_05_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_PERCENT_SH_IDX	WSTL_TEXT_09_HD

#ifndef SD_UI
#define W_L         	GET_MID_L(W_W)
#define W_T         	98
#define W_W         	692
#define W_H         	488

#define MTXT_MSG_L    (W_L + 30)
#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 60)
#define MTXT_MSG_H     140

#define BTN_L        MTXT_MSG_L//(W_L + 100)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W       MTXT_MSG_W// (W_W-200)
#define BTN_H        40

#define MTXT_SWINFO_L    MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     160

#define BAR_L         MTXT_MSG_L//(W_L + 100)
#define BAR_T         (W_T + W_H - 80)
#define BAR_W        (MTXT_MSG_W-90)//480
#define BAR_H         24

#define TXTP_L        (BAR_L + BAR_W)
#define TXTP_T         (BAR_T-8)
#define TXTP_W         90
#define TXTP_H         40
#else
#define    W_W     	482//493
#define    W_H     	360//320
#define    W_L     		GET_MID_L(W_W)//60
#define    W_T     		60//GET_MID_T(W_H)//60

#define MTXT_MSG_L    (W_L + 30)
#define MTXT_MSG_T     (W_T + 10)
#define MTXT_MSG_W     (W_W - 40)
#define MTXT_MSG_H     90

#define BTN_L        (W_L + 120)
#define BTN_T        (MTXT_MSG_T + MTXT_MSG_H + 20)
#define BTN_W        250
#define BTN_H        30

#define MTXT_SWINFO_L    	MTXT_MSG_L
#define MTXT_SWINFO_T     (BTN_T + 60)
#define MTXT_SWINFO_W     MTXT_MSG_W
#define MTXT_SWINFO_H     160

#define BAR_L         (W_L + 20)
#define BAR_T         (W_T + W_H - 60)
#define BAR_W         (MTXT_MSG_W-90)//300
#define BAR_H         16

#define TXTP_L      (BAR_L + BAR_W)
#define TXTP_T      (BAR_T - 8)
#define TXTP_W      90//sharon 90
#define TXTP_H     	30
#endif

#define B_BG_SH_IDX		WSTL_BARBG_01_HD
#define B_MID_SH_IDX		WSTL_NOSHOW_IDX
#define B_SH_IDX			WSTL_BAR_01_HD

#define LDEF_MTXT(root,varMtxt,nextObj,l,t,w,h,sh,cnt,content)	\
	DEF_MULTITEXT(varMtxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    	0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    NULL,NULL,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,NULL,content)


#define LDEF_TXT_BTN(root,varTxt,nextObj,ID,IDu,IDd,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,IDu,IDd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    win2_otaupg_btn_keymap,win2_otaupg_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,resID,NULL)


#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
	DEF_PROGRESSBAR(varBar, &root, nxtObj, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX,\
		NULL, NULL, style, 0, 0, B_MID_SH_IDX, B_SH_IDX, \
		rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_TXT_PROGRESS(root,varTxt,nextObj,l,t,w,h,str)		\
    DEF_TEXTFIELD(varTxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_PERCENT_SH_IDX,TXT_PERCENT_SH_IDX,TXT_PERCENT_SH_IDX,TXT_PERCENT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,0,str)


#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    win2_otaupg_keymap,win2_otaupg_callback,  \
    nxtObj, focusID,0)


#define CODE_LENGTH_OFFSET                  0x200
// negative offset from the end of the data
#define CODE_VERSION_OFFSET                 0x10
// negative offset from the end of the code
#define CODE_VERSION_LEN                    8
#define CODE_VERSION_PADDED_LEN_MIN            0x0
#define CODE_VERSION_PADDED_LEN_MAX            (0x100-1)
#define CODE_APPENDED_LEN                    0x300

  #define NUM_ZERO          (0)

typedef struct
{
    UINT32 compressed_addr;
    UINT32 compressed_len;
    UINT32 uncompressed_addr;
    UINT32 uncompressed_len;
    UINT32 swap_addr;
    UINT32 swap_len;

    UINT32 ota_fw_addr;        // received firmware address.
    UINT32 ota_fw_size;     // received firmware size.

    UINT32 backup_db_addr;    // backup current database address
    UINT32 backup_db_size;    // backup current database size;
    UINT32 update_total_sectors;
    UINT32 update_secotrs_index;

    UINT32 cipher_buf_addr;
    UINT32 cipher_buf_size;
    UINT32 decrypted_data_addr;
    UINT32 decrypted_data_size;
}OTA_INFO, *POTA_INFO;

typedef enum
{
	OTA_STEP_GET_INFO = 0,
	OTA_STEP_DOWNLOADING,
	OTA_STEP_UN7ZIP_CODE,
	OTA_STEP_BURNFLASH,
}OTA_STEP;

extern OSAL_ID  g_ota_flg_id;
extern CONTAINER g_win2_otaupg;
extern TEXT_CONTENT ca_otaupg_mtxt_content_msg[];      /* */
extern TEXT_CONTENT ca_otaupg_mtxt_content_sw[];      /* */
extern MULTI_TEXT   ca_otaupg_mtxt_msg;      /* */
extern MULTI_TEXT   ca_otaupg_mtxt_sw;      /* */
extern TEXT_FIELD   ca_otaupg_txt_btn;      /* */
extern PROGRESS_BAR ca_otaupg_progress_bar;       /* */
extern TEXT_FIELD   ca_otaupg_progress_txt;//
#endif

