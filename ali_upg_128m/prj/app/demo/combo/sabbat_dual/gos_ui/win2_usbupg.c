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
#include <sys_config.h>
#ifdef USB_MP_SUPPORT
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

#ifdef DISK_MANAGER_SUPPORT
#include <api/libfs2/statvfs.h>
#endif
#include <api/libfs2/unistd.h>

#include "../osdobjs_def.h"
#include "../win_com_popup.h"
#include "../string.id"
#include "../images.id"
#include "../osd_config.h"
#include "../osd_rsc.h"
#include "../control.h"
#include "../win_com.h"
//#include "win_usbupg.h"
#include "../menus_root.h" //by_blushark
//#include "../win_com_menu_define.h"
#include <api/librsa/rsa_verify.h>
//#include "power.h"
//#include "key.h"
#include <api/libfs2/stdio.h>
#include "win2_usbupg.h"
#include "../gos_ui/gacas_loader_db.h"
#include "../gos_ui/gacas_upg.h"
#include "../gos_ui/gacas_aes.h"
#include "../power.h"

//#define USBUPG_ONLY_ALLCODE
#define BACKUP_USB_LOADER
#define READ_USBUPG_BUFFER_SIZE 0x800000	/* 10M */
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
#define USBUPG_MSG_MAX_LINE 6
#define USBUPG_CHUNKID_PARA 0x11EE0200

static void win2_usbupg_process_update(UINT32 process);
void win2_usbupg_prompt_show(char *str,UINT16 str_id);
static void win2_usbupg_msg_clear(void);
static void win2_usbupg_msg_show(UINT8 index,char *str,UINT16 str_id);

static VACTION win2_usbupg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win2_usbupg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win2_usbupg_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win2_usbupg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win2_usbupg_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win2_usbupg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
extern int heap_printf_free_size(UINT8 b_printf_now);

static UINT32 ca_usbupg_msg_line= 0 ;

#if defined(_BUILD_USB_LOADER_)
#define WIN_SH_IDX      WSTL_WINSEARCH_03_HD
#else
#define WIN_SH_IDX   	WSTL_WIN_BODYRIGHT_01_HD
#endif
#define CON_SH_IDX	WSTL_BUTTON_01_HD
#define CON_HL_IDX	WSTL_BUTTON_05_HD
#define CON_HL1_IDX	WSTL_BUTTON_05_HD//WSTL_BUTTON_02
#define CON_SL_IDX	WSTL_BUTTON_01_HD
#define CON_GRY_IDX	WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_HD //WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#define USBUPG_INFO_SH_IDX	WSTL_TEXT_09_HD


#define PROGRESSBAR_SH_IDX			WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX		WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX	WSTL_BAR_04_HD

#define B_MID_SH_IDX                WSTL_NOSHOW_IDX

#define PROGRESS_TXT_SH_IDX	WSTL_BUTTON_01_HD

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

#define W_L         	GET_MID_L(W_W)
#define W_T         	98
#define W_W         	692
#define W_H         	488

#define CON_L       	(W_L+20)
#define CON_T       	(W_T + 4)
#define CON_W       	(W_W - 40)
#define CON_H       	40//62
#define CON_GAP    	4

#define TXTN_L_OF   	4
#define TXTN_W      	200//200
#define TXTN_H      	40
#define TXTN_T_OF   	((CON_H - TXTN_H)/2)

#define TXTS_L_OF   	(TXTN_L_OF + TXTN_W+30)
#define TXTS_W      	(CON_W - TXTN_W - 64)
#define TXTS_H      		40//28
#define TXTS_T_OF   	((CON_H - TXTS_H)/2)

#define MSG_SH_IDX 	WSTL_TEXT_09_HD//WSTL_TEXT_09_HD//sharon WSTL_TEXT_04_HD

#define BAR_L   	(CON_L+5)
#define BAR_T   	(CON_T + (CON_H + CON_GAP)*3+10)
#define BAR_W   	(CON_W-BAR_TXT_W-16)
#define BAR_H   	24//24

#define BAR_TXT_L 	(BAR_L + BAR_W + 6)
#define BAR_TXT_W   	120//sharon 60
#define BAR_TXT_T 	( BAR_T - 6)
#define BAR_TXT_H	(BAR_H+12)//24 

#define MSG_L   	(CON_L+5)
#define MSG_T   	(BAR_T + BAR_H+10)
#define MSG_W   	(CON_W-10)
#define MSG_H   	CON_H
#define MSG_GAP 	4

#define UPG_MODE_ID 	2
#define UPG_FILE_ID 	1
#define START_ID    	3

#define LINE_L_OF   	0
#define LINE_T_OF   	(CON_H+4)
#define LINE_W      		CON_W
#define LINE_H      		4

#ifdef USBUPG_ONLY_ALLCODE
#define WINUSB_DFT_FOCUS_ID         START_ID
#define WINUSB_FILE_ID_UP           	START_ID
#define WINUSB_START_ID_DOWN      UPG_FILE_ID
#else
#define WINUSB_DFT_FOCUS_ID         UPG_MODE_ID
#define WINUSB_FILE_ID_UP           	UPG_MODE_ID
#define WINUSB_START_ID_DOWN   	UPG_MODE_ID
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    win2_usbupg_item_keymap,win2_usbupg_item_callback,  \
    conobj, ID,1)
    
#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)       \
    DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_ACTIVE,0, 0,0,0,0,0, l,t,w,h, \
        0,0,0,\
        0,NULL,NULL,C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)

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
    win2_usbupg_item_sel_keymap,win2_usbupg_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
	DEF_PROGRESSBAR(varBar, &root, nxtObj, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, \
		NULL, NULL, style, 0, 0, WSTL_NOSHOW_IDX, WSTL_BAR_01_HD, \
		rl,rt , rw, rh, 1, 100, 100, 0)
   
#define LDEF_PROGRESS_TXT(root,varTxt,nxtObj,l,t,w,h)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, USBUPG_INFO_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 5,0,0,display_strs[1])
    
#define LDEF_TXT_MSG(root,var_txt)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, \
    MSG_SH_IDX,MSG_SH_IDX,MSG_SH_IDX,MSG_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 4,0,0,display_strs[0])

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    win2_usbupg_con_keymap,win2_usbupg_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_INFOCON(var_con,nxt_obj,l,t,w,h)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,USBUPG_INFO_SH_IDX,   \
    NULL,NULL,  \
    &nxt_obj, 0,0)


#define LDEF_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,l_arrow,r_arrow,ID,idu,idd,l,t,w,h,\
                        hl,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W-20,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL  ,ID, l + TXTS_L_OF-50 ,t + TXTS_T_OF,TXTS_W+80,TXTS_H,style,cur,cnt,ptbl) 

#define LDEF_ITEM_SEL_S(root,var_con,nxt_obj,var_txt,var_num,l_arrow,r_arrow,ID,idu,idd,l,t,w,h,\
                        hl,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,&l_arrow  ,ID, l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,TXTS_H,style,cur,cnt,ptbl) \
    LDEF_BMP(&var_con,l_arrow,&r_arrow,l + TXTN_L_OF+TXTN_W, t ,30,h,0/*IM_ARROW_LEFT*/) \
    LDEF_BMP(&var_con,r_arrow,NULL,l + w-30, t ,30,h,0/*IM_ARROW_RIGHT*/)

#define LDEF_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,\
                        hl,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT1(&var_con,var_txt,NULL/*&varLine*/,l,t + TXTN_T_OF,w,TXTN_H,res_id)    \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

LDEF_ITEM_SEL(g_win2_usbupg,ca_usbupg_item_con1,&ca_usbupg_item_con2,ca_usbupg_txt_file_select,ca_usbupg_filelist_sel,\
    ca_usbupg_item_l_arrow0,ca_usbupg_item_r_arrow0,1,3,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,CON_HL_IDX,\
    RS_USBUPG_UPGRADE_FILE,STRING_PROC,0,0,NULL)

LDEF_ITEM_SEL(g_win2_usbupg,ca_usbupg_item_con2,&ca_usbupg_item_con3,ca_usbupg_txt_upgrade_type,ca_usbupg_sel_upgrade_type,\
    ca_usbupg_item_l_arrow1,ca_usbupg_item_r_arrow1,2,3,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,CON_HL_IDX,RS_TOOLS_UPGRADE_MODE,\
    STRING_PROC,0,0,NULL)


#if 0//def _C0200A_CA_ENABLE_
LDEF_ITEM_TXT(g_win2_usbupg,ca_usbupg_item_con3,&ca_usbupg_info_con,ca_usbupg_txt_start,ca_usbupg_item_line2,\
    3,2,WINUSB_START_ID_DOWN,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL1_IDX,RS_COMMON_OK)
#else
LDEF_ITEM_TXT(g_win2_usbupg,ca_usbupg_item_con3,&ca_usbupg_info_con,ca_usbupg_txt_start,ca_usbupg_item_line2,\
    3,2,2,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL1_IDX,RS_COMMON_START)
#endif

LDEF_INFOCON(ca_usbupg_info_con, ca_usbupg_bar_progress,CON_L,CON_T + (CON_H + CON_GAP)*3, CON_W, BAR_H+20+((MSG_H+MSG_GAP)*5))

LDEF_PROGRESS_BAR(ca_usbupg_info_con,ca_usbupg_bar_progress,&ca_usbupg_txt_progress, \
    BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W-4,BAR_H-10)

LDEF_PROGRESS_TXT(ca_usbupg_info_con, ca_usbupg_txt_progress, NULL, \
        BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT_MSG(ca_usbupg_info_con,ca_usbupg_txt_msg)

LDEF_WIN(g_win2_usbupg,&ca_usbupg_item_con1,W_L,W_T,W_W,W_H,WINUSB_DFT_FOCUS_ID)

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
void gausb_null()
{
}

#define GAUSB_FLASH_SECTOR_SIZE		0x10000
//#define CHUNK_ID_MAIN_CODE        	0x01FE0101
//#define CHUNK_ID_MAIN_CODE_SEE	0x06F90101
//#define CHUNK_ID_LOGO_BOOT        	0x02FD0100
//#define CHUNK_ID_LOGO_MAIN        	0x02FD0200
#define VACT_STB_FLASH_OUT            (0xF0)
#define PERCENT_MAX 				100
#if (GACAS_FLASH_TOTAL_SIZE == 0x400000)
#define MAIN_CODE_FLASH_ADDR	0xA0000
#else
#define MAIN_CODE_FLASH_ADDR	0x200000
#endif

typedef enum
{
	GAUSB_UPG_NO_ERROR = 0,
	GAUSB_UPG_ERROR_MEM_INIT,
	GAUSB_UPG_ERROR_FILE_NOT_EXIST,
	GAUSB_UPG_ERROR_FILE_READ_NO_FILE,
	GAUSB_UPG_ERROR_FILE_READ_OVER_FLOW,
	GAUSB_UPG_ERROR_FILE_READ_LEN,
	GAUSB_UPG_ERROR_FILE_READ_UNKOWN,
	GAUSB_UPG_ERROR_FILE_DECRYPT,
	GAUSB_UPG_ERROR_FILE_CRC32,
	GAUSB_UPG_ERROR_FILE_OPERATOR,
	GAUSB_UPG_ERROR_FILE_VER_TIME,
	GAUSB_UPG_ERROR_FILE_CHECK,
	GAUSB_UPG_ERROR_FILE_BURN,
}GAUSB_UPG_ERROR_CODE;
typedef struct
{
	UINT8 	upg_type;
	UINT32 	read_addr;
	UINT32 	read_size;

	UINT32 	update_total_sectors;
	UINT32 	update_secotrs_index;

	UINT32 decrypt_addr;
	UINT32 decrypt_size;
}GAUSB_INFO_T;
UINT8 b_gausb_upg_file_is_read = 0;
GAUSB_INFO_T g_gausb_info;

UINT8 b_allow_upgrade_all = 0;
UINT8 b_need_upgrade_loader = 0;
extern UINT32 g_gacas_tmp_buf_addr;
extern void win2_otaupg_show_swinfo_display(unsigned char index,char *p_str,unsigned char b_update );

int gausb_get_file_len(char* file_name)
{
	int flen = 0;
	FILE * file = NULL;
	file = fopen(file_name,"r");
	if(NULL == file)
	{
		UU_PRINTF("gausb_get_file_len---(%s) =1= NULL\n",file_name);
		return 0;
	}
	fseek(file,0,SEEK_END);
	flen = ftell(file);
	UU_PRINTF("gausb_get_file_len-- flen = %8x \n",flen);    
	fclose(file);
	return flen;
}

int gausb_read_file(char* file_dir,char *buf,UINT32 buf_len,UINT32*p_read_len,usbupg_prgrs_disp progress_disp)
{
	int rlen = 0;
	char* rbuf = buf;
	int flen = 0;
	int offset = 0;
	int file_ret = USBUPG_FILEREAD_NO_ERROR;
	FILE * file = NULL;
	file = fopen(file_dir,"r");
	if(NULL == file)
	{
		UU_PRINTF("gausb_read_file----ERROR--NULL\n");
            return USBUPG_FILEREAD_ERROR_NOFILE;
	}
	fseek(file,0,SEEK_END);
	flen = ftell(file);
	if(flen > (int)buf_len)
	{
		UU_PRINTF("gausb_read_file----ERROR--flen[%8x]> buf_len[%8x]\n",flen,buf_len);
		flen = buf_len;
		//file_ret = USBUPG_FIELREAD_ERROR_OVERFLOW;
		//fclose(file);
		//return file_ret;
	}
	while(offset<flen)
	{
		if(offset > (int)buf_len)
		{
			UU_PRINTF("gausb_read_file----ERROR--offset[%8x]> buf_len[%8x]\n",offset,buf_len);
			file_ret = USBUPG_FIELREAD_ERROR_OVERFLOW;
			break;
		}
		fseek(file,offset,SEEK_SET);
		if((offset+GAUSB_FLASH_SECTOR_SIZE)<= flen)
		{
			rlen = GAUSB_FLASH_SECTOR_SIZE;
		}
		else if(offset<flen)
		{
			rlen = flen - offset;
		}
		else
		{
			UU_PRINTF("gausb_read_file----ERROR--offset[%8x]>= flen[%8x]\n",offset,flen);
			file_ret = USBUPG_FIELREAD_ERROR_OVERFLOW;
			break;
		}
		if(fread(rbuf,rlen,1,file) == 0)
		{
			UU_PRINTF("gausb_read_file----ERROR--[%8x][%8x][%8x]\n",rbuf,rlen,offset);
			file_ret = USBUPG_FIELREAD_ERROR_READERROR;
			break;
		}
		else
		{
			rbuf += rlen;
			offset += rlen;
			//UU_PRINTF("gausb_read_file----OK--[%8x][%8x][%8x]\n",rbuf,rlen,offset);
			progress_disp(100*offset/flen);
		}
	}
	*p_read_len = offset;
	UU_PRINTF("gausb_read_file-- rlen = %8x \n",offset);    
	fclose(file);
	return file_ret;
}

int gausb_check_upg_file_all(void)
{
	FILE * file = NULL;
	int flen = 0;
	UINT8 rbuf[16];
	UINT8 dbuf[16];
	
	b_allow_upgrade_all = 0;
	b_need_upgrade_loader = 0;		
	file = fopen(GAUSB_FILE_DIR,"r");
	if(NULL == file)
	{
		return -1;
	}
	fseek(file,0,SEEK_END);
	flen = ftell(file);
	UU_PRINTF("gausb_check_upg_file_all-- flen = %8x \n",flen);    
	if(flen >= 0x400000)
	{
		memset(rbuf,0,sizeof(rbuf));
		memset(dbuf,0,sizeof(dbuf));
		fseek(file,flen-0x40,SEEK_SET);
		if(fread(rbuf,16,1,file) > 0)
		{
			gacas_aes_printf_bin_16(rbuf);
			if(0 == gacas_decrypt_upg_file((UINT8*)rbuf,(UINT8*)dbuf,16))
			{
				UINT32 check_ver_len = 0;
				check_ver_len= (dbuf[3]<<24)|(dbuf[2]<<16)|(dbuf[1]<<8)|dbuf[0];
				gacas_aes_printf_bin_16(dbuf);
				UU_PRINTF("gausb_check_upg_file_all-- check_ver_len = %d\n",check_ver_len);    
				if(check_ver_len > 0)
				{
					b_allow_upgrade_all  = 1;
				}
			}
		}
		memset(rbuf,0,sizeof(rbuf));
		memset(dbuf,0,sizeof(dbuf));
		fseek(file,MAIN_CODE_FLASH_ADDR+0x10,SEEK_SET);
		if(fread(rbuf,16,1,file) > 0)
		{
			gacas_aes_printf_bin_16(rbuf);
			if(0 == gacas_decrypt_upg_file((UINT8*)rbuf,(UINT8*)dbuf,16))
			{
				gacas_aes_printf_bin_16(dbuf);
				CHUNK_HEADER chunk_header_flash;
		
				sto_get_chunk_header(CHUNK_ID_MAIN_CODE, &chunk_header_flash);
				UU_PRINTF("gausb_check_upg_file_all-- name-[%s]-flash--\n",chunk_header_flash.name);    
				UU_PRINTF("gausb_check_upg_file_all-- name-[%s]-upg--\n",dbuf);    
				if(memcmp(dbuf,chunk_header_flash.name,16) != 0 && memcmp(dbuf,GACAS_UPG_OPERATOR_NAME,GACAS_UPG_OPERATOR_NAME_LEN) != 0)
				{
 #ifndef GACAS_UPG_WITH_NO_LOADER
					b_need_upgrade_loader = 1;		
 #endif
					libc_printf("gausb_check_upg_file_all--need_upgrade_loader###\n");
				}
			}
		}
	}
	fclose(file);
	return 0;
}
int gausb_check_upg_file(void)
{
	FILE * file = NULL;
	
	file = fopen(GAUSB_FILE_DIR,"r");
	if(NULL == file)
	{
		return -1;
	}
	fclose(file);
	return 0;
}

int gausb_mem_init(void)
{
	heap_printf_free_size(1);
	libc_printf("PVR_VOB_BUFFER--[%X][%X]\n",__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN);
	libc_printf("BUFFER_MAX_LEN--[%x]\n",GACAS_UPG_BUFFER_MAX_LEN);
	g_gausb_info.read_size 		= GACAS_UPG_BUFFER_MAX_LEN;
	g_gausb_info.decrypt_size 	= GACAS_UPG_BUFFER_MAX_LEN;
#if 1
	g_gausb_info.decrypt_addr 	= (__MM_PVR_VOB_BUFFER_ADDR | 0xa0000000);
	g_gausb_info.read_addr 		= g_gausb_info.decrypt_addr+0x40;
#else
	g_gausb_info.decrypt_addr 	= (__MM_PVR_VOB_BUFFER_ADDR | 0xa0000000);
	if(g_gacas_tmp_buf_addr == 0)
		g_gacas_tmp_buf_addr = (UINT32)MALLOC(GACAS_UPG_BUFFER_MAX_LEN + 0xf);
	g_gacas_tmp_buf_addr = (0xFFFFFFF8 & g_gacas_tmp_buf_addr);
	if(g_gausb_info.read_addr == 0)
		g_gausb_info.read_addr 	= g_gacas_tmp_buf_addr;
	if(g_gausb_info.read_addr == 0)
	{
		g_gausb_info.read_size = 0;
		return GAUSB_UPG_ERROR_MEM_INIT;
	}
#endif
	libc_printf("mem_init--[%8x][%8x][%8x][%8x][%8x]\n",g_gacas_tmp_buf_addr,g_gausb_info.read_addr,g_gausb_info.read_size,g_gausb_info.decrypt_addr,g_gausb_info.decrypt_size);
	if(g_gausb_info.read_addr != 0)
		MEMSET((void*)g_gausb_info.read_addr,0,g_gausb_info.read_size);
	if(g_gausb_info.decrypt_addr != 0)
		MEMSET((void*)g_gausb_info.decrypt_addr,0,g_gausb_info.decrypt_size);
	heap_printf_free_size(1);
	return GAUSB_UPG_NO_ERROR;
}

int gausb_upg_burn_flash(void)
{
	int ret;
	char msg_info[100];
	UINT32 usb_file_addr;
	UINT32 usb_file_size;
	UINT32 usb_flash_addr;
	UINT32 usb_flash_size;
	UINT32 chunk_id[4];
	memset(msg_info,0,sizeof(msg_info));
	sprintf(msg_info,"Start burn flash,please do not shut down!");
	win2_usbupg_msg_show(ca_usbupg_msg_line,msg_info,RS_OTA_BURNFLASH_DO_NOT_POWER_OFF);
	ca_usbupg_msg_line++;
	win2_usbupg_process_update(0);

	usb_file_addr 		= g_gausb_info.decrypt_addr;
	usb_file_size 		= g_gausb_info.decrypt_size;
	if(ca_usbupg_sel_upgrade_type.n_sel == 2)
	{
#if 1
		usb_flash_addr 	= 0;
		usb_flash_size		= g_gausb_info.decrypt_size;
#else
		usb_flash_addr 		= 0x80000;//0;
		usb_flash_size		= 0x780000;//g_gausb_info.decrypt_size;
		usb_file_addr 		= g_gausb_info.decrypt_addr;
		usb_file_size 		= g_gausb_info.decrypt_size;
		usb_file_addr		+= 0x80000;
		usb_file_size		= usb_file_size;
#endif
		ret = gacas_upg_burn_flash_by_addr(usb_flash_addr,usb_flash_size,usb_file_addr,usb_file_size,0);
	}
	else if(ca_usbupg_sel_upgrade_type.n_sel == 1)
	{
		chunk_id[0] = CHUNK_ID_MAIN_CODE;
		chunk_id[1] = CHUNK_ID_MAIN_CODE_SEE;
		chunk_id[2] = CHUNK_ID_LOGO_BOOT;
		chunk_id[3] = CHUNK_ID_LOGO_MAIN;
		gacas_upg_set_updata_chunk_id(chunk_id,4);
		if(b_need_upgrade_loader == 1)
		{
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER);
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER_SEE);
		}		
		ret = gacas_upg_burn_flash_by_chunk(0);
	}
	else
	{
		chunk_id[0] = CHUNK_ID_MAIN_CODE;
		chunk_id[1] = CHUNK_ID_MAIN_CODE_SEE;
		gacas_upg_set_updata_chunk_id(chunk_id,2);
		if(b_need_upgrade_loader == 1)
		{
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER);
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER_SEE);
		}
		ret = gacas_upg_burn_flash_by_chunk(0);		
	}
	if(ret != 0)
	{
		ret = GAUSB_UPG_ERROR_FILE_BURN;
		win2_usbupg_prompt_show("Upgraded USB file burn to flash error!",0);
		win2_usbupg_msg_show(ca_usbupg_msg_line,"Upgraded USB file burn to flash error!",0);
		ca_usbupg_msg_line++;
	}
	return ret;
}
UINT32 gausb_time_ver_change(UINT8*p_time)
{
	UINT32 year = 0 ;
	UINT32 month = 0;
	UINT32 day = 0;
	UINT32 time = 0;
	UINT8 *p_t_time = p_time;
	while(*p_t_time >= '0' && *p_t_time <= '9')
	{
		year *= 10;
		year += (*p_t_time - '0');
		p_t_time++;
	}
	while(*p_t_time == 0x2d)
	{
		p_t_time++;
	}
	while(*p_t_time >= '0' && *p_t_time <= '9')
	{
		month *= 10;
		month += (*p_t_time - '0');
		p_t_time++;
	}
	while(*p_t_time == 0x2d)
	{
		p_t_time++;
	}
	while(*p_t_time >= '0' && *p_t_time <= '9')
	{
		day *= 10;
		day += (*p_t_time - '0');
		p_t_time++;
	}
	if(day > 99)
		day = 99;
	if(month > 99)
		month = 99;
	if(year > 9999)
		year = 9999;
	time = day + (month*100)+(year*10000);
	return time;
}

INT32 gausb_stb_flash_data_out(void)
{
	UINT32 block_addr = 0;
	UINT32 block_len = 0;
	UINT32 data_len = 0;
	//RET_CODE ret = RET_FAILURE;
	struct sto_device *sto_dev = NULL;
	UINT8 *p_data_temp = NULL;
	char msg_info[100];

	memset(msg_info,0,sizeof(msg_info));
	sprintf(msg_info,"STB Flash output to USB start......");
	win2_usbupg_msg_show(1,msg_info,0);
	gausb_mem_init();
	block_len = 0;
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == sto_dev)
	{
		sprintf(msg_info,"STB Flash output to USB start......Fail!");
		win2_usbupg_msg_show(1,msg_info,0);
		libc_printf("Error-- can't get sto device.\n");
		return RET_FAILURE;
	}	
	block_addr 	= sto_chunk_get_start();
	block_len 		= sto_chunk_get_maxlen();
//	ret = sto_get_chunk_len(chid, &block_addr, &block_len);
	memset(msg_info,0,sizeof(msg_info));
	sprintf(msg_info,"STB Flash size(0x%08lx), reading......",block_len);
	win2_usbupg_msg_show(2,msg_info,0);
	data_len = block_len;
	if(data_len > GACAS_UPG_BUFFER_MAX_LEN)
	{
		libc_printf("Error-- data_len == [%8x].\n",data_len);
		sprintf(msg_info,"STB Flash size(0x%08lx), reading......Fail..len!",block_len);
		win2_usbupg_msg_show(2,msg_info,0);
		return RET_FAILURE;
	}	
	libc_printf("stb_flash_data----addr[%8x][%8x]len\n",block_addr,block_len);
	p_data_temp = (UINT8*)g_gausb_info.decrypt_addr;//g_gacas_tmp_buf_addr;
	if (NULL == p_data_temp)
	{
		libc_printf("Error-- MALLOC--p_data_temp.\n");
		sprintf(msg_info,"STB Flash size(0x%08lx), reading......Fail..no mem!",block_len);
		win2_usbupg_msg_show(2,msg_info,0);
		return RET_FAILURE;
	}		
	if ((INT32)data_len != sto_get_data(sto_dev, p_data_temp, block_addr,data_len))
	{
		libc_printf("Error---sto_get_data---fail!\n");
		sprintf(msg_info,"STB Flash size(0x%08lx), reading......Fail..get data!",block_len);
		win2_usbupg_msg_show(2,msg_info,0);
		return RET_FAILURE;
	}	
	sprintf(msg_info,"STB Flash size(0x%08lx), reading......OK!",block_len);
	win2_usbupg_msg_show(2,msg_info,0);	
	memset(msg_info,0,sizeof(msg_info));
	sprintf(msg_info,"STB Flash size(0x%08lx), writing......",block_len);
	win2_usbupg_msg_show(3,msg_info,0);

	FILE *fp = fopen("/mnt/uda1/Ali3711c_DDR64M_all.bin","wb");
	if(!fp)
	{
		libc_printf("open userdb.bin error\n");
		sprintf(msg_info,"STB Flash size(0x%08lx), writing......Fail!",block_len);
		win2_usbupg_msg_show(3,msg_info,0);
		return RET_FAILURE;
	}
	fseek(fp, 0,SEEK_SET);
	fwrite(p_data_temp,1,data_len,fp);
	fflush(fp);
	fclose(fp);
	fsync("/mnt/uda1");	
	libc_printf("gausb_stb_flash_data_out----Ali3711c_DDR64M_all.bin[%8x][%8x]-SUCCESS\n",data_len,data_len-0x10);
	sprintf(msg_info,"STB Flash size(0x%08lx), writing......OK!",block_len);
	win2_usbupg_msg_show(3,msg_info,0);
	return RET_SUCCESS;	
}

INT32 gacas_ota_time_check(UINT32 ota_file_addr, UINT32 ota_file_size, UINT8 upg_type, INT32 *p_errCode)
{
	UINT32 id;
	CHUNK_HEADER chunk_header_flash;
	CHUNK_HEADER chunk_header_upg;
	UINT32 time_ver_flash = 0;
	UINT32 time_ver_upg = 0;
	UINT32 crc32_1;
	UINT32 crc32_2;
	UINT32 check_ver_len = 0;
	UINT8   check_ver_data[56];
	UINT32	read_len = ota_file_size;
	UINT8 *p_decrypt_buff = NULL;
	char msg_info[100];
	UINT32 u32_bootOffset = 0;

	memset(msg_info, 0, sizeof(msg_info));
	//b_need_upgrade_loader = 0;
	p_decrypt_buff 	= (UINT8 *)ota_file_addr;
	crc32_1 = gacas_aes_crc32(p_decrypt_buff,read_len-4);
	crc32_2 = (p_decrypt_buff[read_len-1]<<24)|(p_decrypt_buff[read_len-2]<<16)|(p_decrypt_buff[read_len-3]<<8)|p_decrypt_buff[read_len-4];
	libc_printf("[%8x]--CRC32:[%8x]-[%8x]\n",read_len-4,crc32_1,crc32_2);
	if(crc32_1 != crc32_2)
	{
		if (upg_type == 1)//OTA
		{
			win2_otaupg_show_swinfo_display(0,"OTA file error, CRC32!",1);
			win2_usbupg_prompt_show("OTA file error, CRC32!",0);
		}
		*p_errCode = GAUSB_UPG_ERROR_FILE_CRC32;
		return RET_FAILURE;
	}
	id = CHUNK_ID_MAIN_CODE;
	check_ver_len= (p_decrypt_buff[read_len-61]<<24)|(p_decrypt_buff[read_len-62]<<16)|(p_decrypt_buff[read_len-63]<<8)|p_decrypt_buff[read_len-64];
	if(check_ver_len > 56)
		check_ver_len = 56;
	memset(check_ver_data,0,sizeof(check_ver_data));
	memcpy(check_ver_data,&p_decrypt_buff[read_len-60],check_ver_len);
	libc_printf("check_ver--:[%d][%s]\n",check_ver_len,check_ver_data);

	memset(&chunk_header_flash,0,sizeof(CHUNK_HEADER));
	memset(&chunk_header_upg,0,sizeof(CHUNK_HEADER));
    
	gacas_loader_get_boot_len(&u32_bootOffset);
	chunk_init((unsigned long)(p_decrypt_buff+u32_bootOffset), (unsigned int)(read_len-u32_bootOffset));
	if (sto_get_chunk_header(id, &chunk_header_flash) == 0)
	{
		libc_printf("ERROR--:sto_get_chunk_header-[%x]\n",id);
		b_need_upgrade_loader = 0;	
		return RET_SUCCESS;
	}
	get_chunk_header(id, &chunk_header_upg);
	if(chunk_header_upg.id == id && chunk_header_flash.id == id)
	{
		libc_printf("name--:[%s]upg-[%s]stb\n",chunk_header_upg.name,chunk_header_flash.name);
		libc_printf("time--:[%s]upg-[%s]stb\n",chunk_header_upg.time,chunk_header_flash.time);
		libc_printf("ver--:[%s]upg-[%s]stb\n",chunk_header_upg.version,chunk_header_flash.version);
		libc_printf("len--:[%8x]upg-[%8x]stb\n",chunk_header_upg.len,chunk_header_flash.len);
		libc_printf("offset--:[%8x]upg-[%8x]stb\n",chunk_header_upg.offset,chunk_header_flash.offset);
		if((check_ver_len > 0 && check_ver_len <= 16) 
		&& memcmp(chunk_header_flash.time,check_ver_data,check_ver_len) == 0)
		{
			time_ver_flash = gausb_time_ver_change(chunk_header_flash.time);
			time_ver_upg 	= gausb_time_ver_change(chunk_header_upg.time);
			libc_printf("upg--by--time_ver:[%d]upg-[%d]stb\n",time_ver_upg,time_ver_flash);
			if(memcmp(chunk_header_upg.name,chunk_header_flash.name,16) != 0
			&& memcmp(chunk_header_upg.name,GACAS_UPG_OPERATOR_NAME,GACAS_UPG_OPERATOR_NAME_LEN) != 0)
			{
#ifndef GACAS_UPG_WITH_NO_LOADER
				b_need_upgrade_loader = 1;					
#endif
				libc_printf("ota--need_upgrade_loader###\n");
			}
		}
		else if(memcmp(chunk_header_upg.name,chunk_header_flash.name,16) == 0
			||memcmp(chunk_header_upg.name,GACAS_UPG_OPERATOR_NAME,GACAS_UPG_OPERATOR_NAME_LEN) == 0)
		{
			time_ver_flash = gausb_time_ver_change(chunk_header_flash.time);
			time_ver_upg 	= gausb_time_ver_change(chunk_header_upg.time);
			if(time_ver_flash > time_ver_upg)
			{
				libc_printf("error--time_ver:[%d]upg-[%d]stb\n",time_ver_upg,time_ver_flash);
				sprintf(msg_info, "Error,time ver:%s --> %s",chunk_header_flash.time,chunk_header_upg.time);
				if (upg_type == 1)//OTA
				{
					win2_otaupg_show_swinfo_display(0,msg_info,1);
					win2_usbupg_prompt_show(msg_info,0);
				}
				else //USB UPGRADE
				{
					win2_usbupg_msg_show(ca_usbupg_msg_line,msg_info,0);
					ca_usbupg_msg_line++;
				}
				*p_errCode = GAUSB_UPG_ERROR_FILE_VER_TIME;
				return RET_FAILURE;
			}
		}
		else
		{
			libc_printf("error--operator:[%s]upg-[%s]stb\n",chunk_header_upg.name,chunk_header_flash.name);
			sprintf(msg_info, "Error,operator:%s --> %s",chunk_header_flash.name,chunk_header_upg.name);
			if (upg_type == 1)
			{
				win2_otaupg_show_swinfo_display(0,msg_info,1);
				win2_usbupg_prompt_show(msg_info,0);
			}
			else //USB UPGRADE
			{
				win2_usbupg_msg_show(ca_usbupg_msg_line,msg_info,0);
				ca_usbupg_msg_line++;
			}
			*p_errCode = GAUSB_UPG_ERROR_FILE_OPERATOR;
			return RET_FAILURE;
		}
		sprintf(msg_info, "Upgrade,operator:%s --> %s",chunk_header_flash.name,chunk_header_upg.name);
		if (upg_type == 1) //OTA
		{
			win2_otaupg_show_swinfo_display(0,msg_info,0);
		}
		else //USB UPGRADE
		{
			win2_usbupg_msg_show(ca_usbupg_msg_line,msg_info,0);
			ca_usbupg_msg_line++;
		}
		sprintf(msg_info, "Upgrade,time ver:%s --> %s",chunk_header_flash.time,chunk_header_upg.time);
		if (upg_type == 1) //OTA
		{
			win2_otaupg_show_swinfo_display(1,msg_info,1);
		}
		else //USB UPGRADE
		{
			win2_usbupg_msg_show(ca_usbupg_msg_line,msg_info,0);
			ca_usbupg_msg_line++;
		}
	}
	else
	{
		libc_printf("error--no--app\n");
		if (upg_type == 1)//OTA
		{
			win2_otaupg_show_swinfo_display(0,"OTA file error, no app!",1);
			win2_usbupg_prompt_show("OTA file error, no app!",0);
		}
		*p_errCode = GAUSB_UPG_ERROR_FILE_CHECK;
		return RET_FAILURE;
	}
	return RET_SUCCESS;
}	


int gausb_upg_read_file(void)
{
	int ret;
	static UINT32	read_len;
	char msg_info[100];
	//UINT8*p_read_buff;
	//UINT8*p_decrypt_buff;
	//UINT32 i;
	
	libc_printf("%s()----%s\n", __FUNCTION__,GAUSB_FILE_NAME);
	if(0 != gausb_check_upg_file())//no file selected to upgrade
	{
		ret =  GAUSB_UPG_ERROR_FILE_NOT_EXIST;		
		goto deal_end;
	}
	if(0 != gausb_mem_init())
	{
		ret =  GAUSB_UPG_ERROR_MEM_INIT;		
		goto deal_end;
	}
	ca_usbupg_msg_line = 0;
	memset(msg_info, 0, sizeof(msg_info));
	sprintf(msg_info, "Reading upgrade file......");
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);
	ret = gausb_read_file(GAUSB_FILE_DIR, (char*)g_gausb_info.read_addr, g_gausb_info.read_size, &read_len, win2_usbupg_process_update);
	if(ret != 0)
	{
		goto deal_end;
	}
	sprintf(msg_info,"Reading upgrade file......OK!");
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);
	ca_usbupg_msg_line++;
	memset(msg_info, 0, sizeof(msg_info));
	sprintf(msg_info, "Decrypting upgrade file,len = 0x%lx......", read_len);
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);	
    
	//p_read_buff 	= (UINT8*)g_gausb_info.read_addr;
	//p_decrypt_buff 	= (UINT8*)g_gausb_info.decrypt_addr;
	ret = gacas_decrypt_upg_file((UINT8*)g_gausb_info.read_addr, (UINT8*)g_gausb_info.decrypt_addr, read_len);
    LDR_PRINT("af gacas_decrypt_upg_file, ret=0x%x (0x%x, 0x%x, 0x%x)\n", ret, g_gausb_info.read_addr, g_gausb_info.decrypt_addr, read_len);
	//libc_printf("read_buff:[%8x]-[%8x]\n",read_len-0x40,read_len-0x10);
	//gacas_aes_printf_bin_16(&p_read_buff[read_len-0x40]);
	//gacas_aes_printf_bin_16(&p_read_buff[read_len-0x10]);
	//libc_printf("decrypt_buff:[%8x]-[%8x]\n",read_len-0x40,read_len-0x10);
	//gacas_aes_printf_bin_16(&p_decrypt_buff[read_len-0x40]);
	//gacas_aes_printf_bin_16(&p_decrypt_buff[read_len-0x10]);
	//FREE(g_gacas_tmp_buf_addr);
	//g_gausb_info.read_addr 	= 0;
	//g_gausb_info.read_size  	= 0;
	//g_gacas_tmp_buf_addr			= 0;
	heap_printf_free_size(1);
	if(0 != ret)
	{
		ret = GAUSB_UPG_ERROR_FILE_DECRYPT;
		goto deal_end;
	}
	sprintf(msg_info,"Decrypting upgrade file,len = 0x%lx......OK!", read_len);
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);		
	ca_usbupg_msg_line++;
	if (gacas_ota_time_check(g_gausb_info.decrypt_addr, read_len, 2, (INT32*)&ret) != RET_SUCCESS)
	{
		LDR_PRINT("time check failed, error code=%d\n", ret);
		goto deal_end;
	}
	
	g_gausb_info.decrypt_size = read_len;
	memset(msg_info, 0, sizeof(msg_info));
	sprintf(msg_info, "Checking decrypt file,len = 0x%lx......", g_gausb_info.decrypt_size);
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);		
	gacas_upg_init(win2_usbupg_process_update);
	if(0 != gacas_upg_check_file(g_gausb_info.decrypt_addr, g_gausb_info.decrypt_size))
	{
        LDR_PRINT("gacas_upg_check_file fail \n");
		ret = GAUSB_UPG_ERROR_FILE_CHECK;
		goto deal_end;
	}
    
	gacas_upg_encrypt_chunk(CHUNK_ID_MAIN_CODE, 1);
	memset(msg_info, 0, sizeof(msg_info));
	sprintf(msg_info, "Checking decrypt file,len = 0x%lx......OK!", g_gausb_info.decrypt_size);
	win2_usbupg_msg_show(ca_usbupg_msg_line, msg_info, 0);
	ca_usbupg_msg_line++;
deal_end:
	switch(ret)
	{
		case GAUSB_UPG_ERROR_MEM_INIT:
			win2_usbupg_prompt_show("Malloc 8M ByteS mem is fail!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_NOT_EXIST:
			win2_usbupg_prompt_show("Upgraded USB file is not exist!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_READ_NO_FILE:
			win2_usbupg_prompt_show("Upgraded USB file read fail,no file!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_READ_OVER_FLOW:
			win2_usbupg_prompt_show("Upgraded USB file read fail,data over flow!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_READ_LEN:
			win2_usbupg_prompt_show("Upgraded USB file read fail,len is error!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_READ_UNKOWN:
			win2_usbupg_prompt_show("Upgraded USB file read fail,unkown error!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_DECRYPT:
			win2_usbupg_prompt_show("Upgraded USB file decrypt fail!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_CRC32:
			win2_usbupg_prompt_show("USB file error, CRC32!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_OPERATOR:
			win2_usbupg_prompt_show("USB file error, operator!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_VER_TIME:
			win2_usbupg_prompt_show("USB file error, time version!",0);
			break;
		case GAUSB_UPG_ERROR_FILE_CHECK:
			win2_usbupg_prompt_show("Upgraded USB file is not support!",0);
			break;
		default:
			break;
	}
	return ret;
}

void win2_update_vscr(void)
{
	LPVSCR apVscr = NULL;
	apVscr = osd_get_task_vscr(osal_task_get_current_id());
	osd_update_vscr(apVscr);	
}

static void win2_usbupg_process_update(UINT32 process)
{
	if(process > PERCENT_MAX)
	{
		process = 0;
	}
	//libc_printf("process_update--[%d]\n",process);
	osd_set_progress_bar_pos(&ca_usbupg_bar_progress,(INT16)process);
	osd_set_text_field_content(&ca_usbupg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);

	osd_draw_object( (POBJECT_HEAD)&ca_usbupg_bar_progress, C_UPDATE_ALL);
	osd_draw_object( (POBJECT_HEAD)&ca_usbupg_txt_progress, C_UPDATE_ALL);
	win2_update_vscr();
#ifdef WATCH_DOG_SUPPORT
	dog_set_time(0, 0);
#endif
}

void win2_usbupg_prompt_show(char *str,UINT16 str_id)
{
	UINT8 back_saved;
	LDR_PRINT("prompt_show--[%s]\n", str);
	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(str, NULL, str_id);
	win_compopup_open_ext(&back_saved);
	osal_task_sleep(3000);
	win_compopup_smsg_restoreback();
}

static void win2_usbupg_msg_clear(void)
{
	UINT8 i= 0;
	//OSD_RECT rect;

	for(i=0;i<USBUPG_MSG_MAX_LINE;i++)
	{
		ca_usbupg_txt_msg.head.frame.u_left  	= MSG_L;
		ca_usbupg_txt_msg.head.frame.u_top   	= MSG_T+(MSG_H+MSG_GAP)*i;
		ca_usbupg_txt_msg.head.frame.u_width 	= MSG_W;
		ca_usbupg_txt_msg.head.frame.u_height	= MSG_H;
		osd_set_text_field_content(&ca_usbupg_txt_msg, STRING_ANSI, (UINT32)"  ");
		osd_draw_object( (POBJECT_HEAD)&ca_usbupg_txt_msg, C_UPDATE_ALL);
	}
	win2_update_vscr();
}
static void win2_usbupg_msg_show(UINT8 index,char *str,UINT16 str_id)
{
	//OSD_RECT rect;

	if(index > 0 && index% USBUPG_MSG_MAX_LINE == 0)
	{
		win2_usbupg_msg_clear();
	}
	index = index%USBUPG_MSG_MAX_LINE;
	ca_usbupg_txt_msg.head.frame.u_left  	= MSG_L;
	ca_usbupg_txt_msg.head.frame.u_top   = MSG_T+(MSG_H+MSG_GAP)*index;
	ca_usbupg_txt_msg.head.frame.u_width = MSG_W;
	ca_usbupg_txt_msg.head.frame.u_height= MSG_H;
	libc_printf("msg_show--[%d][%s]\n",index,str);
	if(str == NULL)
	{
		osd_set_text_field_content(&ca_usbupg_txt_msg, STRING_ID, (UINT32)str_id);
	}
	else
	{
		osd_set_text_field_content(&ca_usbupg_txt_msg, STRING_ANSI, (UINT32)str);
	}
	osd_draw_object( (POBJECT_HEAD)&ca_usbupg_txt_msg, C_UPDATE_ALL);
	win2_update_vscr();
}

static PRESULT win2_usbupg_init(void)
{
	//RET_CODE block_ret= 0;
	MULTISEL *pmsel = NULL;
	//UINT16 upg_show_count=0;

	ca_usbupg_txt_msg.p_string = display_strs[10];
	memset(&g_gausb_info,0,sizeof(g_gausb_info));
	//gacas_loader_db_deal_one_time(GACAS_LOADER_TYPE_USB_BOOT);
	//gacas_loader_db_deal_one_time(GACAS_LOADER_TYPE_USB_MANNAL);
	//gacas_kdf_test();
	pmsel = &ca_usbupg_sel_upgrade_type;
	osd_set_multisel_sel(pmsel, 0);
	//MainCode
	//Logo Main	
	//AllCode
	osd_set_multisel_count(pmsel, 2);

	//No File
	//product_sabbat_ali3281_ca.abs.ecf
	pmsel = &ca_usbupg_filelist_sel;
	osd_set_multisel_sel(pmsel, 0);
	osd_set_multisel_count(pmsel, 0);

	//only all_code upg
//	osd_set_attr(&ca_usbupg_item_con1, C_ATTR_INACTIVE);
	osd_change_focus((POBJECT_HEAD)&g_win2_usbupg,2,C_UPDATE_ALL);

	ca_usbupg_txt_start.b_x = TXTN_L_OF;
	heap_printf_free_size(1);
	return PROC_PASS;
}

static VACTION win2_usbupg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
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

static PRESULT win2_usbupg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT8 bid= 0;
	UINT16 sel= 0;
	//UINT16 block_idx= 0;
	UINT16 *uni_str= 0;
	//UINT32 block_len= 0;
	char str_buf[50] = {0};
	//char str_extend[4] = {0};
	//char str_filter[4] = {0};
	PRESULT ret = PROC_PASS;
	//USBUPG_FILENODE filenode;
	//MULTISEL *pmsel = NULL;

	//MEMSET(&filenode, 0x0, sizeof(USBUPG_FILENODE));
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
						case 2:
							sprintf(str_buf,"%s","AllCode");
							break;
						case 1:
							if(b_need_upgrade_loader == 0)
								sprintf(str_buf,"%s","MainCode&Logo");
							else
								sprintf(str_buf,"%s","MainCode&Logo&Loader");								
							break;
						case 0:
							if(b_need_upgrade_loader == 0)
								sprintf(str_buf,"%s","MainCode");
							else
								sprintf(str_buf,"%s","MainCode&Loader");																
							break;
						default:
							sprintf(str_buf,"%s","AllCode");
							break;
					}
					com_asc_str2uni((UINT8*)str_buf, uni_str);
					break;
				case UPG_FILE_ID:
					if(0 == gausb_check_upg_file())
					{
						b_gausb_upg_file_is_read = 1;
						snprintf(str_buf, 50, "%s", GAUSB_FILE_NAME);
						com_asc_str2uni((UINT8*)str_buf, uni_str);
					}
					else
					{
						b_gausb_upg_file_is_read = 0;
						snprintf(str_buf, 50, "No File");
						com_asc_str2uni((UINT8*)str_buf, uni_str);
					}
					break;
				default:
					uni_str[0] = 0;//com_asc_str2uni("", uni_str);
					break;
			}
			break;
		case EVN_POST_CHANGE:
			break;
		default:
			break;
	}
	return ret;
}

static VACTION win2_usbupg_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
	VACTION act=VACT_PASS;

	switch(key)
	{
		case V_KEY_ENTER:
		case V_KEY_RIGHT:
			act = VACT_ENTER;
			break;
		case V_KEY_RED:
			act = VACT_STB_FLASH_OUT;
			break;			
		default:
			act = VACT_PASS;
			break;
	}
	return act;
}
static PRESULT win2_usbupg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT8 bid= 0;
	//UINT8 back_saved= 0;
	//UINT16 block_idx= 0;
	//UINT16 file_idx= 0;
	//UINT16 popup_strid= 0;
	//INT32 length= 0;
	//INT32 vscr_idx;
	//char str_buff[20];
	//void *pbuff = NULL;
	//BOOL burnflash_ret= FALSE;
	VACTION unact =VACT_PASS;
	PRESULT ret =PROC_PASS;
	//LPVSCR apVscr = NULL;
	//win_popup_choice_t choice =WIN_POP_CHOICE_NULL;
	//struct usbupg_ver_info ver_info;
	//int result=0;
	//USBUPG_BLOCK_TYPE usbupg_type=USBUPG_ALLCODE;

	ret = PROC_PASS;
	bid = osd_get_obj_id(pobj);
	if( EVN_UNKNOWN_ACTION == event)
	{
		unact = (VACTION)(param1>>16);
		if((VACT_STB_FLASH_OUT == unact) && (START_ID==bid))
		{
			gausb_stb_flash_data_out();
		}
		else if((VACT_ENTER==unact) && (START_ID==bid))
		{
			win2_usbupg_msg_clear();
			win2_usbupg_process_update(0);
			if(gausb_upg_read_file() == 0)
			{
                	//	win_compopup_init(WIN_POPUP_TYPE_OKNO);
                	//	win_compopup_set_msg(NULL, NULL, RS_USBUPG_SURE_BURN_FLASH);
			//	choice = win_compopup_open_ext(&back_saved);
			//	if(WIN_POP_CHOICE_YES == choice)
                		{	
					osal_task_dispatch_off();
					if(0 != gausb_upg_burn_flash())
					{
						osal_task_dispatch_on();
						UU_PRINTF("gausb_upg_burn_flash--- -FAIL\n");
						return ret;
					}
					osal_task_dispatch_on();
					win2_usbupg_process_update(100);
					win2_usbupg_prompt_show("USB Upgrade is success, the STB will auto shut off!",0);
					power_off_process(0);
					power_on_process();		
				}
			//	else
			//	{
			//		win2_usbupg_process_update(0);
			//		popup_strid = 0;
			//	}				
			}
			osd_set_attr(&ca_usbupg_item_con1, C_ATTR_ACTIVE);
			osd_set_attr(&ca_usbupg_item_con2, C_ATTR_ACTIVE);
			osd_set_attr(&ca_usbupg_item_con3, C_ATTR_ACTIVE);
			osd_draw_object( (POBJECT_HEAD)&ca_usbupg_item_con1, C_UPDATE_ALL);
			osd_draw_object( (POBJECT_HEAD)&ca_usbupg_item_con2, C_UPDATE_ALL);
			osd_track_object( (POBJECT_HEAD)&ca_usbupg_item_con3, C_UPDATE_ALL);
			//OSD_update_vscr(apvscr);
		}
		else if((VACT_ENTER==unact) && (UPG_MODE_ID==bid))
		{
			win2_usbupg_process_update(ca_usbupg_msg_line*20);
			win2_usbupg_msg_show(ca_usbupg_msg_line,"usbupg_msg_show-----",0);
			ca_usbupg_msg_line++;
			if(ca_usbupg_msg_line >=USBUPG_MSG_MAX_LINE)
				ca_usbupg_msg_line = 0;		
		}
	}
	return ret;
}

static VACTION win2_usbupg_con_keymap(POBJECT_HEAD pobj, UINT32 vkey)
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
			//act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
			break;
	}
	return act;
}

static PRESULT win2_usbupg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch(event)
	{
		case EVN_PRE_OPEN:
			//wincom_open_title(pobj, RS_USBUPG_UPGRADE_BY_USB, 0);
			wincom_open_title_ext(pobj,GACAS_UPG_TITLE_USB,0);
			wincom_open_help(NULL, NULL, HELP_TYPE_RS232);
			osd_set_progress_bar_pos(&ca_usbupg_bar_progress,(INT16)0);
			osd_set_text_field_content(&ca_usbupg_txt_progress, STRING_NUM_PERCENT, (UINT32)0);
			win2_usbupg_init();
			{
				MULTISEL *pmsel;
				pmsel = &ca_usbupg_filelist_sel;
				osd_set_multisel_sel(pmsel, 0);
				if(gausb_check_upg_file_all() == 0)
				{
					osd_set_multisel_count(pmsel, 1);
				}
				else
				{
					osd_set_multisel_count(pmsel, 0);	
				}
				osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
				if(b_allow_upgrade_all == 1)
				{
					pmsel = &ca_usbupg_sel_upgrade_type;
					osd_set_multisel_sel(pmsel, 2);
					osd_set_multisel_count(pmsel, 3);					
				}
				else
				{
					pmsel = &ca_usbupg_sel_upgrade_type;
					osd_set_multisel_sel(pmsel, 0);
					osd_set_multisel_count(pmsel, 2);					
				}
			}
            		break;
		case EVN_POST_OPEN:
			api_set_system_state(SYS_STATE_USB_UPG);
			win2_usbupg_msg_clear();
			break;
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			api_set_system_state(SYS_STATE_NORMAL);
			wincom_close_title();
			wincom_close_help();
			break;
		case EVN_MSG_GOT:
			if(CTRL_MSG_SUBTYPE_CMD_EXIT == param1)
			{
				ret = PROC_LEAVE;
			}
#ifdef DISK_MANAGER_SUPPORT
			if ((CTRL_MSG_SUBTYPE_CMD_STO== param1 ) && (USB_STATUS_OVER == param2))
			{
				MULTISEL *pmsel;
				pmsel = &ca_usbupg_filelist_sel;
				osd_set_multisel_sel(pmsel, 0);
				if(gausb_check_upg_file_all() == 0)
				{
					osd_set_multisel_count(pmsel, 1);
				}
				else
				{
					osd_set_multisel_count(pmsel, 0);	
				}
				osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
				if(b_allow_upgrade_all == 1)
				{
					pmsel = &ca_usbupg_sel_upgrade_type;
					osd_set_multisel_sel(pmsel, 2);
					osd_set_multisel_count(pmsel, 3);					
				}
				else
				{
					pmsel = &ca_usbupg_sel_upgrade_type;
					osd_set_multisel_sel(pmsel, 0);
					osd_set_multisel_count(pmsel, 2);					
				}
			        if(2 == osd_get_focus_id(pobj))
					osd_track_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
				else
					osd_draw_object((POBJECT_HEAD)pmsel,C_UPDATE_ALL);
			}
#endif
            		break;
		default:
			break;
	}
	return ret;
}
#endif

