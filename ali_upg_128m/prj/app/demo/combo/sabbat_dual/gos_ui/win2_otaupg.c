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
#if 1//defined(GOS_WIN2_UI)
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
#include <sys_parameters.h>

#include "../control.h"
#include "../osdobjs_def.h"
#include "../string.id"
#include "../images.id"
#include "../osd_config.h"
#include "../menus_root.h"
#include "../win_com.h"
#include "../win_signalstatus.h"
#include <api/librsa/flash_cipher.h>

#include <api/libota/lib_ota.h>
#include "../win_ota_set.h"
#include "win2_otaupg.h"
#include "../gos_ui/gacas_loader_db.h"
#include "../gos_ui/gacas_upg.h"
#include "../gos_ui/gacas_aes.h"
#include "../key.h"
#include "../power.h"

#define DECRPT_KEY_ID           0x22DD0100
#define DECRPT_KEY_MASK         0xFFFF0000
#define HEX_ZERO   0x30
#define HEX_NIME   0x39
#define HEX_A_LOWERCASE  97
#define HEX_F_LOWERCASE   102
#define HEX_A_UPPERCASE  65
#define HEX_F_UPPERCASE   70

UINT32 		gos_ota_proc_ret=0;  /* PROC_SUCCESS / PROC_FAILURE / PROC_STOPPED */
UINT8  		gos_ota_user_stop=0;
OTA_STEP 	gos_ota_proc_step=0;
//static UINT8 *p_otaosd_buff  = NULL;
//add by colin: used for judging wether the third block id
//(ciplus chunk id) of the flash and the memory is identical
struct dl_info 	gos_ota_dl_info;
OTA_INFO 	gos_ota_info;
/*******************************************************************************
* Objects declaration
*******************************************************************************/

static VACTION win2_otaupg_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win2_otaupg_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION win2_otaupg_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win2_otaupg_callback(POBJECT_HEAD pobj,VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT  win2_otaupg_message_proc(UINT32 msg_type, UINT32 msg_code);
static INT32 win2_otaupg_set_memory(void);
static INT32 win2_otaupg_get_info(void);
static INT32 win2_otaupg_download(void);
static INT32 win2_otaupg_un7zip_code(void);
static void win2_otaupg_draw_process(UINT32 process);
static void win2_otaupg_init(void);
INT32  win2_otaupg_get_btn_state(void);
UINT16 win2_otaupg_get_btn_strid(INT32 btn_state);
INT32 win2_otaupg_reboot(void);
void win2_otaupg_set_btn_msg_display(BOOL update);
void win2_otaupg_set_swinfo_display(BOOL update);
void win2_otaupg_update_process_old(UINT32 process);
void win2_otaupg_update_process_ex(UINT32 process);
void win2_otaupg_show_msg(UINT8 *ascii_str);
void win2_otaupg_show_swinfo_display(unsigned char index,char *p_str,unsigned char b_update );
extern int un7zip(unsigned char *in, unsigned char *out, unsigned char *buf);
extern void win_otaset_get_ota_info(UINT16 *p_pid,UINT32 *p_freq,UINT32 *p_symb,UINT32 *p_mod);
extern void win2_usbupg_prompt_show(char *str,UINT16 str_id);
extern INT32 gacas_ota_time_check(UINT32 ota_file_addr, UINT32 ota_file_size, UINT8 upg_type, INT32 *p_errCode);


LDEF_MTXT(g_win2_otaupg,ca_otaupg_mtxt_msg,&ca_otaupg_txt_btn,\
    MTXT_MSG_L, MTXT_MSG_T, MTXT_MSG_W, \
    MTXT_MSG_H,MTXT_MSG_SH_IDX,1,ca_otaupg_mtxt_content_msg)

LDEF_TXT_BTN(g_win2_otaupg,ca_otaupg_txt_btn,\
    &ca_otaupg_mtxt_sw,1,1,1,\
    BTN_L, BTN_T, BTN_W, BTN_H,\
    RS_DISEQC12_MOVEMENT_STOP)

LDEF_MTXT(g_win2_otaupg,ca_otaupg_mtxt_sw,\
    &ca_otaupg_progress_bar,\
    MTXT_SWINFO_L, MTXT_SWINFO_T, \
    MTXT_SWINFO_W, MTXT_SWINFO_H,MTXT_SWINFO_SH_IDX,\
    3,ca_otaupg_mtxt_content_sw)

LDEF_PROGRESS_BAR(g_win2_otaupg,ca_otaupg_progress_bar,\
        &ca_otaupg_progress_txt,    \
        BAR_L, BAR_T, BAR_W, BAR_H, \
    PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,\
    2,5,BAR_W-4,BAR_H-10)//    4,5,BAR_W-8,14)
    
LDEF_TXT_PROGRESS(g_win2_otaupg,ca_otaupg_progress_txt, NULL, \
        TXTP_L, TXTP_T, TXTP_W, TXTP_H,display_strs[0])

LDEF_WIN(g_win2_otaupg,&ca_otaupg_mtxt_msg,W_L,W_T,W_W,W_H,1)

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/

TEXT_CONTENT ca_otaupg_mtxt_content_msg[] =
{
    {STRING_ID,{0}},
};

TEXT_CONTENT ca_otaupg_mtxt_content_sw[] =
{
    {STRING_UNICODE,{0}}, //display_strs[10]
    {STRING_UNICODE,{0}}, //isplay_strs[11]
    {STRING_UNICODE,{0}}//display_strs[12]
};
static UINT32 old_process = 0;
extern UINT16 ota_pid;
extern UINT32 g_gacas_tmp_buf_addr;
extern UINT32       g_ota_size;
extern UINT8 b_need_upgrade_loader;
//extern void ap_send_epg_PF_update();
//extern void ap_send_epg_sch_update();
//extern void ap_send_epg_detail_update();
extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);
extern int heap_printf_free_size(UINT8 b_printf_now);
#define   OTAUPG_TIMER_NAME   "gos_ota"
#define 	UN7ZIP_CODE_TIME_COUNTER	30//70
UINT32    OTAUPG_TIMER_TIME;
static ID	otaupg_timer = OSAL_INVALID_ID;
static UINT32 ota_time_counter = 0;

unsigned int gacas_get_unzip_size(unsigned char *in)
{
    unsigned int size=0;

    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

INT32 gacas_ota_burn_flash(void)
{
	UINT32 ota_file_addr;
	UINT32 ota_file_size;
	UINT32 ota_flash_addr;
	UINT32 ota_flash_size;
	//UINT8*p_read_buff;
	//UINT8*p_decrypt_buff;

	INT32 ret = SUCCESS ;
	//char msg_info[100];
    	UINT32 errCode = 0;

	win2_otaupg_show_swinfo_display(0," ",0);
	win2_otaupg_show_swinfo_display(1," ",0);
	win2_otaupg_show_swinfo_display(2," ",1);	
	gos_ota_info.update_total_sectors 	= 0;
	gos_ota_info.update_secotrs_index 	= 0;
	gos_ota_info.uncompressed_len 		= (UINT32)gacas_get_unzip_size((UINT8 *)gos_ota_info.compressed_addr);
	//p_read_buff 	= (UINT8*)gos_ota_info.uncompressed_addr;
	//p_decrypt_buff 	= (UINT8*)gos_ota_info.compressed_addr;
	ret = gacas_decrypt_upg_file((UINT8*)gos_ota_info.uncompressed_addr,(UINT8*)gos_ota_info.compressed_addr,gos_ota_info.uncompressed_len);
//	libc_printf("read_buff:[%8x]-[%8x]\n",gos_ota_info.uncompressed_len-0x40,gos_ota_info.uncompressed_len-0x10);
//	gacas_aes_printf_bin_16(&p_read_buff[gos_ota_info.uncompressed_len-0x40]);
//	gacas_aes_printf_bin_16(&p_read_buff[gos_ota_info.uncompressed_len-0x10]);
//	libc_printf("decrypt_buff:[%8x]-[%8x]\n",gos_ota_info.uncompressed_len-0x40,gos_ota_info.uncompressed_len-0x10);
//	gacas_aes_printf_bin_16(&p_decrypt_buff[gos_ota_info.uncompressed_len-0x40]);
//	gacas_aes_printf_bin_16(&p_decrypt_buff[gos_ota_info.uncompressed_len-0x10]);
	if(ret != 0)
	{
		win2_otaupg_show_swinfo_display(0,"OTA file error, decrypt fail!",1);
		win2_usbupg_prompt_show("OTA file error, decrypt fail!",0);
		return PROC_FAILURE;
	}
	MEMCPY((UINT8*)gos_ota_info.uncompressed_addr,(UINT8*)gos_ota_info.compressed_addr,gos_ota_info.uncompressed_len);
	ota_file_addr 	= gos_ota_info.uncompressed_addr;
	ota_file_size 	= gos_ota_info.uncompressed_len;	
	gacas_upg_init(win2_otaupg_update_process_old);
	if (gacas_ota_time_check(ota_file_addr,ota_file_size,1,(INT32*)&errCode) != RET_SUCCESS)
	{
		libc_printf("time check failed, error code=%d\n", errCode);
        	return PROC_FAILURE;
	}
    
	ret = gacas_upg_check_file(ota_file_addr,ota_file_size);
	if(ret != 0)
	{
		win2_otaupg_show_swinfo_display(0,"OTA file error, check file!",1);
		win2_usbupg_prompt_show("OTA file error, check file!",0);		
		return PROC_FAILURE;
	}
	gacas_upg_encrypt_chunk(CHUNK_ID_MAIN_CODE,1);
	if(gacas_loader_db_is_ota_by_addr(&ota_flash_addr,&ota_flash_size) == TRUE)
	{
#ifndef GACAS_UPG_WITH_NO_LOADER
		if(b_need_upgrade_loader == 1)
		{
			if(ota_flash_addr > 0x80000)
			{
				ota_flash_size 	= ota_flash_size +(ota_flash_addr-0x80000);
				ota_flash_addr	= 0x80000;
			}
		}
#endif		
		ota_file_addr += ota_flash_addr;
		ota_file_size  = ota_flash_size;
		//应用需要备份loader，OTA时不需要备份
#ifdef GOS_CONFIG_APP
		ret = gacas_upg_burn_flash_by_addr(ota_flash_addr,ota_flash_size,ota_file_addr,ota_file_size,1);
#else
		ret = gacas_upg_burn_flash_by_addr(ota_flash_addr,ota_flash_size,ota_file_addr,ota_file_size,0);
#endif
	}
	else
	{
		if(gacas_upg_get_updata_chunk_num() == 0)
		{
			UINT32 chunk_id[6];
			chunk_id[0] = CHUNK_ID_MAIN_CODE;
			chunk_id[1] = CHUNK_ID_MAIN_CODE_SEE;
			chunk_id[2] = CHUNK_ID_LOGO_BOOT;
			chunk_id[3] = CHUNK_ID_LOGO_MAIN;
			gacas_upg_set_updata_chunk_id(chunk_id,4);
		}
#ifndef GACAS_UPG_WITH_NO_LOADER
		if(b_need_upgrade_loader == 1)
		{
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER);
			gacas_upg_add_updata_chunk_id(CHUNK_ID_LOADER_SEE);
		}
#endif				
		//应用需要备份loader，OTA时不需要备份
#ifdef GOS_CONFIG_APP
		ret = gacas_upg_burn_flash_by_chunk(1);
#else
		ret = gacas_upg_burn_flash_by_chunk(0);
#endif
	}
	win2_otaupg_update_process_old(OTA_UPDATE_PROGRESS); 
	osal_flag_set(g_ota_flg_id,(SUCCESS == ret) ? PROC_SUCCESS : PROC_FAILURE);	
    	return ret;
}

void win2_otaupg_show_swinfo_display(unsigned char index,char *p_str,unsigned char b_update )
{
	MULTI_TEXT *mtxt_swinfo=NULL;
	TEXT_CONTENT *tcont=NULL;

	mtxt_swinfo = &ca_otaupg_mtxt_sw;

	if(index > 2)
		return;
	tcont = &ca_otaupg_mtxt_content_sw[index];
	tcont->text.p_string = display_strs[10+index];
	com_asc_str2uni((UINT8 *)p_str,tcont->text.p_string);
	osd_draw_object((POBJECT_HEAD)mtxt_swinfo, C_UPDATE_ALL);
	
	if(b_update)
	{
		osd_draw_object((POBJECT_HEAD)mtxt_swinfo, C_UPDATE_ALL);
	}
}

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION win2_otaupg_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
	VACTION act=0;

	switch(key)
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT win2_otaupg_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact=0;
	INT32 btn_state=0;
	LPVSCR apvscr=0;
	//INT32 func_ret=0;
	//BOOL func_flag=FALSE;


	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(VACT_ENTER == unact)
			{
				/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
				btn_state 		= win2_otaupg_get_btn_state();
				apvscr 		= osd_get_task_vscr(osal_task_get_current_id());
				switch(btn_state)
				{
					case 0:
						if(gos_ota_proc_step < OTA_STEP_UN7ZIP_CODE)
						{
							if(!gos_ota_user_stop)
							{
								//func_flag=ota_stop_service();
								//gos_ota_user_stop = 1;
							}
						}
						break;
					case 1:
						ret = PROC_LEAVE;
						break;
					case 2:
						win2_otaupg_download();
						win2_otaupg_set_btn_msg_display(TRUE);
						osd_update_vscr(apvscr);
						break;
					case 3:
						win2_otaupg_un7zip_code();
						win2_otaupg_set_btn_msg_display(TRUE);
						osd_update_vscr(apvscr);
						break;
					case 4:
						gos_ota_proc_ret = 0;
						gos_ota_user_stop = 0;
						gos_ota_proc_step = OTA_STEP_BURNFLASH;
						osal_flag_clear(g_ota_flg_id, OTA_FLAG_PTN);	
						win2_otaupg_set_btn_msg_display(TRUE);
						osd_update_vscr(apvscr);
						//if (NUM_ZERO != win2_otaupg_burnflash())
						if (SUCCESS != gacas_ota_burn_flash())
						{
							gos_ota_proc_ret = PROC_FAILURE;
							ret = PROC_LEAVE;
							break;
						}
						win2_otaupg_set_btn_msg_display(TRUE);
						osd_update_vscr(apvscr);
						break;
					case 5:
						win2_otaupg_reboot();
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


static VACTION win2_otaupg_keymap(POBJECT_HEAD pobj, UINT32 key)
{
	VACTION act=VACT_PASS;

	switch(key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			if(OTA_STEP_BURNFLASH == gos_ota_proc_step  ||gacas_loader_db_is_ota_force() == TRUE)
			{
				act = VACT_PASS;
			}
			else
			{
				act = VACT_CLOSE;
			}
			break;
		case V_KEY_POWER:
			libc_printf("V_KEY_POWER-----\n");
			act = VACT_PASS+1;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

void win2_otaupg_timer_func(UINT unused)
{
	api_stop_timer(&otaupg_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
	libc_printf("ap_send_msg\n");
}

static PRESULT win2_otaupg_callback(POBJECT_HEAD pobj,VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//INT32 func_ret=0;
	//BOOL func_flag=FALSE;

	switch(event)
	{
		case EVN_PRE_OPEN:
#ifndef _BUILD_OTA_E_
			epg_release();
#endif
			OTAUPG_TIMER_TIME = 1000*15; 
			//gacas_loader_db_deal_one_time(GACAS_LOADER_TYPE_OTA_FORCE);
			api_set_system_state(SYS_STATE_OTA_UPG);
			//p_otaosd_buff = MALLOC(OSD_VSRC_MEM_MAX_SIZE);
			win2_otaupg_set_memory();

			//wincom_open_title((POBJECT_HEAD)&g_win2_otaupg,RS_TOOLS_SW_UPGRADE_BY_OTA, 0);
			wincom_open_title_ext((POBJECT_HEAD)&g_win2_otaupg,(UINT8*)GACAS_UPG_TITLE_OTA,0);
			win2_otaupg_get_info();
			win2_otaupg_set_btn_msg_display(FALSE);
			win2_otaupg_set_swinfo_display(FALSE);
			win2_otaupg_init();
			break;

		case EVN_POST_OPEN:
			otaupg_timer = api_start_timer(OTAUPG_TIMER_NAME,OTAUPG_TIMER_TIME,win2_otaupg_timer_func);  
			break;

		case EVN_PRE_CLOSE:
			if((OTA_STEP_BURNFLASH == gos_ota_proc_step) && (gos_ota_proc_ret != PROC_FAILURE))
			{
				ret = PROC_LOOP;
				break;
			}
			else
			{
				if(NUM_ZERO == gos_ota_proc_ret)
				{
					ota_stop_service();
				}
			}
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

		break;

		case EVN_POST_CLOSE:
			//if(p_otaosd_buff != NULL)
			//{
			//osd_task_buffer_free(OSAL_INVALID_ID,p_otaosd_buff);
			//FREE(p_otaosd_buff);
			//p_otaosd_buff = NULL;
			//}
#ifndef _BUILD_OTA_E_
    			epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START/*buffer*/, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
#endif
			api_set_system_state(SYS_STATE_NORMAL);
			break;
		case EVN_UNKNOWN_ACTION:
			ret = PROC_LOOP;
			break;
		case EVN_MSG_GOT:
			ret = win2_otaupg_message_proc(param1,param2);
			if(param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT) 
			{ 
				ret = PROC_LEAVE;
			}
			break;
		default:
			break;
	}
	return ret;
}

static PRESULT  win2_otaupg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_PASS;
	UINT flgptn = 0;
	UINT32 hkey;


	if(CTRL_MSG_SUBTYPE_STATUS_OTA_PROCESS_MSG == msg_type)
	{
		win2_otaupg_draw_process(msg_code);
	}

	if(NUM_ZERO==gos_ota_proc_ret)
	{
		//osal_flag_wait(&flgptn,g_ota_flg_id,OTA_FLAG_PTN,OSAL_TWF_ORW,OTA_FLAG_TIMEOUT);
		if(E_FAILURE == osal_flag_wait(&flgptn,g_ota_flg_id, OTA_FLAG_PTN, OSAL_TWF_ORW,OTA_FLAG_TIMEOUT))
		{
			libc_printf("osal_flag_wait() failed!\n");
		}
		//libc_printf("gos_ota_proc_step==[%d][%d]\n",gos_ota_proc_step,ota_time_counter);
		ota_time_counter++;
		if(gos_ota_proc_step == OTA_STEP_UN7ZIP_CODE)
		{
			if(CTRL_MSG_SUBTYPE_STATUS_OTA_PROCESS_MSG == msg_type && msg_code == 100)
			{
				ota_time_counter = UN7ZIP_CODE_TIME_COUNTER;
			}
			if(ota_time_counter < UN7ZIP_CODE_TIME_COUNTER)
			{
				win2_otaupg_draw_process(ota_time_counter*100/UN7ZIP_CODE_TIME_COUNTER);
			}
		}
		//libc_printf("gos_ota_user_stop==[%d]\n",gos_ota_user_stop);
		switch(gos_ota_proc_step)
		{
			case OTA_STEP_GET_INFO:
			case OTA_STEP_DOWNLOADING:
			case OTA_STEP_UN7ZIP_CODE:
				if(gos_ota_user_stop &&  (flgptn & PROC_STOPPED ) )
				{
					gos_ota_proc_ret = PROC_STOPPED;
				}

				if(!gos_ota_user_stop && (flgptn & PROC_FAILURE) )
				{
					gos_ota_proc_ret = PROC_FAILURE;
				}

				if(flgptn & PROC_SUCCESS)
				{
					if(gos_ota_user_stop)
					{
						gos_ota_proc_ret = PROC_STOPPED;
					}
					else
					{
						gos_ota_proc_ret = PROC_SUCCESS;				
						ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
						ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
					}
				}
				break;
			case OTA_STEP_BURNFLASH:
				if(flgptn & PROC_FAILURE)
				{
					gos_ota_proc_ret = PROC_FAILURE;
				}
				if(flgptn & PROC_SUCCESS)
				{
					gos_ota_proc_ret = PROC_SUCCESS;
					ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
					ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
				}
				break;
			default:
				break;
		}

		if(gos_ota_proc_ret!= NUM_ZERO)
		{
			win2_otaupg_set_btn_msg_display(TRUE);
			if(OTA_STEP_GET_INFO == gos_ota_proc_step)
			{
				win2_otaupg_set_swinfo_display(TRUE);
			}
		}
	}
	return ret;
}

static INT32 win2_otaupg_set_memory(void)
{
	heap_printf_free_size(1);

	if(g_gacas_tmp_buf_addr == 0)
		g_gacas_tmp_buf_addr 			= (UINT32)MALLOC(GACAS_UPG_BUFFER_MAX_LEN + 0xf);
	gos_ota_info.uncompressed_addr 	= __MM_PVR_VOB_BUFFER_ADDR;
	gos_ota_info.uncompressed_len 	= GACAS_UPG_BUFFER_MAX_LEN; //0x500000
	gos_ota_info.swap_addr 		= gos_ota_info.uncompressed_addr+ gos_ota_info.uncompressed_len;
	gos_ota_info.swap_len 			= 256*1024;
	gos_ota_info.backup_db_addr 	= gos_ota_info.swap_addr+gos_ota_info.swap_len;
	gos_ota_info.backup_db_size 		= 0x200000;
	gos_ota_info.compressed_addr 	= (0xFFFFFFF8 & g_gacas_tmp_buf_addr);
	gos_ota_info.compressed_len 	= GACAS_UPG_BUFFER_MAX_LEN;//0x200000;

	ota_mem_config(
	gos_ota_info.compressed_addr,
	gos_ota_info.compressed_len,
	gos_ota_info.uncompressed_addr,
	gos_ota_info.uncompressed_len,
	gos_ota_info.swap_addr,
	gos_ota_info.swap_len);

//	gos_ota_info.cipher_buf_addr 		= gos_ota_info.backup_db_addr;
//	gos_ota_info.cipher_buf_size 		= 0x200000;
//	gos_ota_info.decrypted_data_addr 	= gos_ota_info.cipher_buf_addr + gos_ota_info.cipher_buf_size;
//	gos_ota_info.decrypted_data_size 	= 0x200000;
//	flash_cipher_buf_init((UINT8 *)gos_ota_info.cipher_buf_addr, gos_ota_info.cipher_buf_size);
	heap_printf_free_size(1);
//	p_otaosd_buff = (UINT8 *)((gos_ota_info.decrypted_data_addr+gos_ota_info.decrypted_data_size)|0xa0000000);
	return 0;
}


static INT32 win2_otaupg_get_info(void)
{

	//BOOL ota_get_info_ret=FALSE;

	gos_ota_proc_ret= 0;
	gos_ota_user_stop= 0;
	gos_ota_proc_step= OTA_STEP_GET_INFO;
	ota_time_counter = 0;
	MEMSET(&gos_ota_dl_info,0,sizeof(struct dl_info));
	osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);
	ota_get_download_info(ota_pid,&gos_ota_dl_info);
	return SUCCESS;
}

static INT32 win2_otaupg_download(void)
{

	//BOOL ota_download_ret=FALSE;

	gos_ota_proc_ret = 0;
	gos_ota_user_stop = 0;
	gos_ota_proc_step = OTA_STEP_DOWNLOADING;
	ota_time_counter = 0;

	osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);
	ota_start_download(ota_pid,win2_otaupg_update_process_ex);
    	return SUCCESS;
}

static INT32 win2_otaupg_un7zip_code(void)
{

	//BOOL ota_download_ret=FALSE;

	gos_ota_proc_ret = 0;
	gos_ota_user_stop = 0;
	gos_ota_proc_step = OTA_STEP_UN7ZIP_CODE;
	ota_time_counter = 0;
	osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);
	ota_start_unzip_code(ota_pid,win2_otaupg_update_process_ex);
	return SUCCESS;
}


static void win2_otaupg_draw_process(UINT32 process)
{
	PROGRESS_BAR *bar = &ca_otaupg_progress_bar;
	TEXT_FIELD  *txt = &ca_otaupg_progress_txt;
	TEXT_CONTENT tcont;
	MULTI_TEXT *mtxt_msg = &ca_otaupg_mtxt_msg;
	UINT16 str_buf[50]={0};
	char    progress_strs[30]={0};
	LPVSCR apvscr = NULL;

	//PRESULT obj_ret=PROC_PASS;
	//int sprintf_ret=0;
	//UINT32 str_ret=0;

	MEMSET(&tcont,0,sizeof(TEXT_CONTENT));
	MEMSET(&apvscr,0,sizeof(LPVSCR));
	tcont.text.p_string = str_buf;
	tcont.b_text_type = STRING_UNICODE;
	osd_task_buffer_init(osal_task_get_current_id(),NULL);
	if(OTA_START_BACKUP_FLAG == process)
	{
		com_asc_str2uni((UINT8*)"Start backup OTA data ...", str_buf);
		osd_set_multi_text_content(mtxt_msg, &tcont);
		osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
		osd_set_multi_text_content(mtxt_msg, &ca_otaupg_mtxt_content_msg[0]);
	}
	else if(OTA_START_END_FLAG == process)
	{
		osd_set_multi_text_content(mtxt_msg, &ca_otaupg_mtxt_content_msg[0]);
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
		snprintf(progress_strs,30, "%lu%%",process);
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


static void win2_otaupg_init(void)
{
	PTEXT_FIELD ptxt = NULL;
	MEMSET(&ptxt,0,sizeof(PTEXT_FIELD));
	ptxt = &ca_otaupg_progress_txt;
	osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"0%");
	PROGRESS_BAR *bar = &ca_otaupg_progress_bar;

	osd_set_progress_bar_pos(bar,0);
}

/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
INT32  win2_otaupg_get_btn_state(void)
{
	INT32 ret = 0;

	if(PROC_SUCCESS == gos_ota_proc_ret)
	{
		ret = 0;
	}
	else if(PROC_STOPPED == gos_ota_proc_ret)
	{
		ret = 1;
	}
	else if(PROC_FAILURE == gos_ota_proc_ret)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}

	switch(gos_ota_proc_step)
	{
		case OTA_STEP_GET_INFO:
			if(PROC_SUCCESS == gos_ota_proc_ret)
			{
				ret = 2;
			}
			break;
		case OTA_STEP_DOWNLOADING:
			if(PROC_SUCCESS == gos_ota_proc_ret)
			{
				ret = 3;
			}
			break;
		case OTA_STEP_UN7ZIP_CODE:
			if(PROC_SUCCESS == gos_ota_proc_ret)
			{
				ret = 4;
			}
			break;
		case OTA_STEP_BURNFLASH:
			if(PROC_SUCCESS == gos_ota_proc_ret)
			{
				ret = 5;
			}
			else if(0 == gos_ota_proc_ret)
			{
				ret = 4;//5;
			}
			break;
		default:
			break;
	}
	return ret;
}

UINT16 win2_otaupg_get_btn_strid(INT32 btn_state)
{
	UINT16 btn_strid = 0;
	if(gos_ota_proc_step == OTA_STEP_UN7ZIP_CODE)
	{
		btn_strid = RS_DISPLAY_PLEASE_WAIT;
		return btn_strid;
	}
	switch(btn_state)
	{
		case 0:
			btn_strid = RS_DISPLAY_DOWNLOADING;//RS_DISEQC12_MOVEMENT_STOP;
			break;
		case 1:
			btn_strid = RS_DISPLAY_DOWNLOADING;//RS_HELP_EXIT;
			break;
		case 2:
			btn_strid = RS_DISPLAY_DOWNLOADING;
			break;
		case 3:
			btn_strid = RS_DISPLAY_PLEASE_WAIT;
			break;
		case 4:
			btn_strid = RS_OTA_BURNFLASH;
			break;
		case 5:
			btn_strid = RS_MSG_UPGRADE_REBOOT;
			break;
		default:
			btn_strid = 0;
			break;
	}
	return btn_strid;
}

INT32 win2_otaupg_reboot(void)
{
	gos_ota_proc_ret = 0;
	gos_ota_user_stop = 0;
	//gos_ota_proc_step = OTA_STEP_BURNFLASH;
#ifdef AUTO_OTA
	set_ota_upgrade_found(FALSE);
#endif
	osal_flag_clear(g_ota_flg_id,OTA_FLAG_PTN);

	gacas_loader_db_ota_success();
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
void win2_otaupg_show_msg(UINT8 *ascii_str)
{
	TEXT_CONTENT *tcont=NULL;
	MULTI_TEXT *mtxt_msg=NULL;

	mtxt_msg = &ca_otaupg_mtxt_msg;
	tcont = ca_otaupg_mtxt_content_msg;
	com_asc_str2uni(ascii_str,len_display_str);
	tcont->b_text_type = STRING_UNICODE;
	tcont->text.p_string = len_display_str;
	osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
}

void win2_otaupg_set_btn_msg_display(BOOL update)
{
    MULTI_TEXT *mtxt_msg=NULL;
    TEXT_FIELD *txt_btn=NULL;
    UINT16 btn_strid=0;
    TEXT_CONTENT *tcont=NULL;
    //UINT16 cur_sw_ver = (UINT16)(SYS_SW_VERSION&0xffff);
    UINT8 *src=NULL;
    UINT8 str_len=0;
    INT32 btn_state=0;
    LPVSCR apvscr = NULL;
    //PRESULT obj_ret=PROC_PASS;
    //UINT32 trans_ret=0;
    //int cpy_ret=0;

    MEMSET(&apvscr,0,sizeof(LPVSCR));
    tcont = ca_otaupg_mtxt_content_msg;
    tcont->b_text_type = STRING_ID;

    mtxt_msg = &ca_otaupg_mtxt_msg;
    txt_btn = &ca_otaupg_txt_btn;

/* 0 - stop , 1 - exit , 2 - download 3 - burnflash 4 - reboot*/
    btn_state 	= win2_otaupg_get_btn_state();
    btn_strid 	= win2_otaupg_get_btn_strid(btn_state);

	switch(gos_ota_proc_step)
	{
    		case OTA_STEP_GET_INFO:
			switch(gos_ota_proc_ret)
			{
				case 0:
					tcont->text.w_string_id = RS_OTA_COLLECT_INFORMATION;
					break;
				case PROC_SUCCESS:
					com_asc_str2uni((UINT8*)"Collect information...SUCCESS!",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_STOPPED:
					com_asc_str2uni((UINT8*)"User canceled",len_display_str);

					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_FAILURE:
					tcont->text.w_string_id = RS_OTA_COLLECT_INFORMATION_FAIL;
					break;
				default:
					break;
			}
			break;
			
		case OTA_STEP_DOWNLOADING:
			api_stop_timer(&otaupg_timer);
			switch(gos_ota_proc_ret)
			{
				case 0:
					com_asc_str2uni((UINT8*)"Downing OTA File...",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_SUCCESS:
					com_asc_str2uni((UINT8*)"Downing OTA File...OK!",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_STOPPED:
					com_asc_str2uni((UINT8*)"User Canceled",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_FAILURE:
					com_asc_str2uni((UINT8*)"Downing OTA File...FAIL!",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				default:
					break;
			}
			break;	
		case OTA_STEP_UN7ZIP_CODE:
			switch(gos_ota_proc_ret)
			{
				case 0:
					com_asc_str2uni((UINT8*)"Unzip Code...",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_SUCCESS:
					com_asc_str2uni((UINT8*)"Unzip Code...OK!",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_STOPPED:
					com_asc_str2uni((UINT8*)"User Canceled",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				case PROC_FAILURE:
					com_asc_str2uni((UINT8*)"Unzip Code...FAIL!",len_display_str);
					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
				default:
					break;
			}
			break;		
		case OTA_STEP_BURNFLASH:
			switch(gos_ota_proc_ret)
			{
				case 0:
					tcont->text.w_string_id = RS_OTA_BURNFLASH_DO_NOT_POWER_OFF;
					break;
				case PROC_SUCCESS:
					src = osd_get_unicode_string(RS_DISPLAY_NEXT);
					com_uni_str_copy_char((UINT8*)len_display_str, src);
					str_len = com_uni_str_len(len_display_str);
					com_asc_str2uni((UINT8*)" : ",&len_display_str[str_len]);
					str_len = com_uni_str_len(len_display_str);
					src = osd_get_unicode_string(RS_MSG_UPGRADE_REBOOT);
					com_uni_str_copy_char((UINT8*)&len_display_str[str_len], src);

					tcont->b_text_type = STRING_UNICODE;
					tcont->text.p_string = len_display_str;
					break;
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

    osd_set_text_field_content(txt_btn, STRING_ID, (UINT32)btn_strid);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
        apvscr = osd_get_task_vscr(osal_task_get_current_id());
        if((apvscr != NULL) && (apvscr->lpb_scr != NULL))
        {
            apvscr->update_pending = 1;
            osd_update_vscr(apvscr);
        }
        if((OTA_STEP_BURNFLASH==gos_ota_proc_step  ) &&( NUM_ZERO==gos_ota_proc_ret))
        {
            //osd_draw_object( (POBJECT_HEAD)txt_btn, C_UPDATE_ALL);
            osd_track_object( (POBJECT_HEAD)txt_btn, C_UPDATE_ALL);
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

void win2_otaupg_set_swinfo_display(BOOL update)
{
	MULTI_TEXT *mtxt_swinfo=NULL;
	TEXT_CONTENT *tcont=NULL;
	struct dl_info *info=NULL;
	char str[50]={0};
	//PRESULT obj_ret=PROC_PASS;
	//UINT32 str_ret=0;
	//int sprintf_ret __MAYBE_UNUSED__=0;

	mtxt_swinfo = &ca_otaupg_mtxt_sw;
	info = &gos_ota_dl_info;

	tcont = &ca_otaupg_mtxt_content_sw[0];
	tcont->text.p_string = display_strs[10];

	tcont = &ca_otaupg_mtxt_content_sw[1];
	tcont->text.p_string = display_strs[11];

	tcont = &ca_otaupg_mtxt_content_sw[2];
	tcont->text.p_string = display_strs[12];

	UINT16 pid;
	UINT32 freq;
	UINT32 symb;
	UINT32 mode = 0;

	win_otaset_get_ota_info(&pid,&freq,&symb,&mode);
	tcont = &ca_otaupg_mtxt_content_sw[0];
	snprintf(str, 50, "OTA PID: %d",pid);
	com_asc_str2uni((UINT8*)str,tcont->text.p_string);
	tcont = &ca_otaupg_mtxt_content_sw[1];
	if(mode == QAM16)
	mode = 16;
	else if(mode == QAM32)
	mode = 32;
	else if(mode == QAM64)
	mode = 64;
	else if(mode == QAM128)
	mode = 128;
	else if(mode == QAM256)
	mode = 256;
	snprintf((char*)str, 50, "OTA TP: %ldKHz/%ldKBps/QAM%ld",freq*10,symb,mode);
	com_asc_str2uni((UINT8*)str,tcont->text.p_string);


	if( ((OTA_STEP_GET_INFO == gos_ota_proc_step) && (PROC_SUCCESS == gos_ota_proc_ret))|| (gos_ota_proc_step>OTA_STEP_GET_INFO))
	{
	tcont = &ca_otaupg_mtxt_content_sw[2];
	snprintf(str, 50, "OTA SIZE : %lu bytes",info->sw_size);
	com_asc_str2uni((UINT8*)str,tcont->text.p_string);
	}
	else
	{
	tcont = &ca_otaupg_mtxt_content_sw[2];
	tcont->text.p_string[0] = 0;//str_ret=com_asc_str2uni("",tcont->text.p_string);
	}

	if(update)
	{
		osd_draw_object((POBJECT_HEAD)mtxt_swinfo, C_UPDATE_ALL);
	}
}


void win2_otaupg_update_process_old(UINT32 process)
{
	PROGRESS_BAR *bar = &ca_otaupg_progress_bar;
	TEXT_FIELD  *txt = &ca_otaupg_progress_txt;
	TEXT_CONTENT tcont;
	MULTI_TEXT *mtxt_msg = &ca_otaupg_mtxt_msg;
	UINT16 str_buf[50]={0};
	char    progress_strs[30]={0};
	LPVSCR apvscr = NULL;

	//UINT32 str_ret=0;
	//int sprintf_ret=0;
	//PRESULT obj_ret=PROC_PASS;

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
		com_asc_str2uni((UINT8*)"Start backup OTA data ...", str_buf);
		osd_set_multi_text_content(mtxt_msg, &tcont);
		osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
		osd_set_multi_text_content(mtxt_msg, &ca_otaupg_mtxt_content_msg[0]);
	}
	else if(OTA_START_END_FLAG == process)
	{
		osd_set_multi_text_content(mtxt_msg, &ca_otaupg_mtxt_content_msg[0]);
		osd_draw_object((POBJECT_HEAD)mtxt_msg, C_UPDATE_ALL);
	}
	else
	{
		if(OTA_UPDATE_PROGRESS<process )
		{
			process = 0;
		}

		osd_set_progress_bar_pos(bar,process);
		snprintf(progress_strs,30, "%lu%%",process);
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



void win2_otaupg_update_process_ex(UINT32 process)
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

