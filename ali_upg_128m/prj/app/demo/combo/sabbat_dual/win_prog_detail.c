/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_detail.c
*
*    Description:   The realize of program detail info
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <version/ver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
//#include <api/libtsi/si_epg.h>

#include <api/libosd/osd_lib.h>
#include <api/libclosecaption/lib_closecaption.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_signal.h"
#include "win_prog_detail.h"
#include "control.h"
#include "ap_ctrl_display.h"
#include "key.h"
#include "win_chunk_info.h"
#ifdef SFU_TEST_SUPPORT
#include "sfu_test.h"
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER    g_win_progdetail;

#define FFT_MODULATION_2         2
#define FFT_MODULATION_4         4
#define FFT_MODULATION_8         8
#define FFT_MODULATION_16       16
#define FFT_MODULATION_32       32

//reference the light channel list
#define FFT_MODULATION_DQPSK        2
#define FFT_MODULATION_QPSK         4
#define FFT_MODULATION_16QAM        16
#define FFT_MODULATION_64QAM        64
#define FFT_MODULATION_256QAM       65

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
//extern TEXT_FIELD txt_satname;
#endif


#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
//extern TEXT_FIELD txt_string2;
#endif

#ifdef SFU_TEST_SUPPORT
#define SFU_ENTER_NUMBER  2008
#endif
//volatile BOOL g_b_is_open_sfu = FALSE;
#ifndef SD_UI
#define W_L 145//240
#define W_T 100//100
#define W_W 718//490
#define W_H 330//230

#define TXT_T (W_T + 20 + 4)
#define TXT_W 415//460
#define TXT_H  44
#define TXT_STR_W  260
#define TXT_STR_L (W_L + 30)
#define TXT_L (TXT_STR_L + TXT_STR_W)

#define SIGNAL_OFFSET 20
#define SIGNAL_L (W_L + SIGNAL_OFFSET)
#define SIGNAL_T (TXT_T + TXT_H*4 + SIGNAL_OFFSET - 8)
#define SIGNAL_W (W_W - SIGNAL_OFFSET *2)
#define SIGNAL_H (W_T + W_H - SIGNAL_T)

#else
#define W_L 58//240
#define W_T 60//100
#define W_W 490//490
#define W_H 240//230

#define TXT_T (W_T + 10)
#define TXT_W 300//460
#define TXT_H  30
#define TXT_STR_W  130
#define TXT_STR_L (W_L + 16)
#define TXT_L (TXT_STR_L + TXT_STR_W)

#define SIGNAL_OFFSET 8
#define SIGNAL_L (W_L + SIGNAL_OFFSET)
#define SIGNAL_T (TXT_T + TXT_H*4 + SIGNAL_OFFSET - 8)
#define SIGNAL_W (W_W - SIGNAL_OFFSET *2)
#define SIGNAL_H (W_T + W_H - SIGNAL_T)
#endif

#define PROG_WIN_SH_IDX         WSTL_POP_WIN_01_8BIT
#define TXT_INFO_IDX            WSTL_TEXT_30_8BIT
 //WSTL_TEXT_28_8BIT//WSTL_TEXT_04

static VACTION win_progdetail_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win_progdetail_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);
#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)
#define LDEF_STRTXT(root,var_txt,nxt_obj,l,t,w,h,str_id)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,str_id,NULL)

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,sh,next_obj,focus_id)      \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        win_progdetail_keymap,win_progdetail_callback,  \
        next_obj, focus_id,1)

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS \
    || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
LDEF_TXT(&g_win_progdetail,txt_progname,\
        &txt_satname,TXT_L,(TXT_T + TXT_H * 0),\
    TXT_W,TXT_H,display_strs[0])
LDEF_TXT(&g_win_progdetail,txt_satname,\
        &txt_tpinfo,TXT_L,(TXT_T + TXT_H * 1),\
    TXT_W,TXT_H,display_strs[1])
LDEF_TXT(&g_win_progdetail,txt_tpinfo,\
        &txt_pidinfo,TXT_L,(TXT_T + TXT_H * 2),\
    TXT_W,TXT_H,display_strs[2])
LDEF_TXT(&g_win_progdetail,txt_pidinfo,\
        &txt_string1,TXT_L,(TXT_T + TXT_H * 3),\
    TXT_W,TXT_H,display_strs[3])
LDEF_STRTXT(&g_win_progdetail,txt_string1,\
        &txt_string2,TXT_STR_L,(TXT_T + TXT_H * 0),\
    TXT_STR_W,TXT_H,RS_SYSTEM_TIMERSET_CHANNEL)
LDEF_STRTXT(&g_win_progdetail,txt_string2,\
        &txt_string3,TXT_STR_L,(TXT_T + TXT_H * 1),\
    TXT_STR_W,TXT_H,RS_INFO_SATELLITE)
LDEF_STRTXT(&g_win_progdetail,txt_string3,\
        &txt_string4,TXT_STR_L,(TXT_T + TXT_H * 2),\
    TXT_STR_W,TXT_H,RS_INFO_TRANSPONDER)
LDEF_TXT(&g_win_progdetail,txt_string4,\
        NULL,TXT_STR_L,(TXT_T + TXT_H * 3),\
        TXT_STR_W,TXT_H,display_strs[4])
#else
LDEF_TXT(&g_win_progdetail,txt_progname,\
          &txt_tpinfo,TXT_L,(TXT_T + TXT_H * 0),\
      TXT_W,TXT_H,display_strs[0])
LDEF_TXT(&g_win_progdetail,txt_tpinfo,\
          &txt_pidinfo,TXT_L,(TXT_T + TXT_H * 1),\
      TXT_W,TXT_H,display_strs[1])
LDEF_TXT(&g_win_progdetail,txt_pidinfo,\
          &txt_string1,TXT_L,(TXT_T + TXT_H * 2),\
      TXT_W,TXT_H,display_strs[2])
LDEF_STRTXT(&g_win_progdetail,txt_string1,\
          &txt_string3,TXT_STR_L,(TXT_T + TXT_H * 0),\
      TXT_STR_W,TXT_H,RS_SYSTEM_TIMERSET_CHANNEL)
LDEF_STRTXT(&g_win_progdetail,txt_string3,\
           &txt_string4,TXT_STR_L,(TXT_T + TXT_H * 1),\
       TXT_STR_W,TXT_H,RS_INFO_TRANSPONDER)
LDEF_TXT(&g_win_progdetail,txt_string4,\
         NULL,TXT_STR_L,(TXT_T + TXT_H * 2),\
     TXT_STR_W,TXT_H,display_strs[3])
#endif

LDEF_WIN(g_win_progdetail,NULL,W_L,W_T,W_W,W_H,PROG_WIN_SH_IDX,&txt_progname,1)

static void info_load(void);
#ifdef BC_TEST
static BOOL vmx_test = FALSE;
#endif

/*******************************************************************************
*   function definition
*******************************************************************************/
static VACTION win_progdetail_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_INFOR:
		#ifdef BC_TEST
			vmx_test = TRUE;
		#endif
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

BOOL check_magic_key_firmware_info(UINT32 code)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    return FALSE;
#endif

#ifndef SUPPORT_CAS9
    static UINT8 magic_key[] = { V_KEY_5, V_KEY_6,V_KEY_5,V_KEY_6,V_KEY_5 };
#endif
    int key_cnt = 0;

	if(0 == key_cnt)
	{
		;
	}
#ifndef SUPPORT_CAS9
    if (magic_key[key_cnt++] == code)
    {
        if (key_cnt == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt = 0;
    return FALSE;
}
#ifndef _CAS9_CA_ENABLE_
static char info[500];
#endif
void show_firmware_info(void)
{
#ifndef _CAS9_CA_ENABLE_
    char *p = info;
    UINT8 back_save, i;
    UINT32 reg_temp,reg_addr;
    UINT16 str_len;
    UINT8 temp=0;
    int p_size = 500;
    INT32 ret = 0;

    char *p_sdk_ver = NULL;
    p_sdk_ver = get_sdk_ver();
    MEMSET(p, 0x0, p_size);
    strncpy(p, (const char *)p_sdk_ver, (p_size-1));
    str_len = STRLEN(info);
    p = info + str_len;
    p_size -= str_len;

    char *p_app_ver = NULL;
    p_app_ver = get_app_ver();
    strncpy(p, (const char *)p_app_ver, (p_size-1));
    str_len = STRLEN(info);
    p = info + str_len;
    p_size -= str_len;

    char *p_core_ver = NULL;
    p_core_ver = get_core_ver();
    strncpy(p,(const char *)p_core_ver, (p_size-1));
    str_len = STRLEN(info);
    p = info + str_len;
    p_size -= str_len;

    ret = snprintf(p, p_size, "0xb8000074/7c: 0x%08lx 0x%08lx\n",
    *((volatile UINT32 *)0xb8000074), *((volatile UINT32 *)0xb800007c));
    if(0 == ret)
    {
        ali_trace(&ret);
    }
    str_len = STRLEN(info);
    p = info + str_len;
    p_size -= str_len;

    reg_temp = *((volatile UINT32 *)0xb8018504);
    ret = snprintf(p, p_size, "0xb8018504: 0x%08lx\n", reg_temp);
    if(0 == ret)
    {
        ali_trace(&ret);
    }
    str_len = STRLEN(info);
    p = info + str_len;
    p_size -= str_len;

    reg_addr = 0xb8001270;
    i=0;
    while(reg_addr <= 0xb800129c)
    {
        temp=i%4;
        if(temp== NUM_ZERO)
        {
            ret = snprintf(p, p_size, "0x%lx: ", reg_addr);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
            str_len = STRLEN(info);
            p = info + str_len;
            p_size -= str_len;
        }

        reg_temp = *((volatile UINT32 *)(reg_addr));
        ret = snprintf(p, p_size, "0x%08lx ",reg_temp);
        if(0 == ret)
        {
            ali_trace(&ret);
        }
        str_len = STRLEN(info);
        p = info + str_len;
        p_size -= str_len;
        reg_addr += 4;
        i++;
        temp=i%4;
        if(temp == NUM_ZERO)
        {
            ret = snprintf(p, p_size, "\n");
            if(0 == ret)
            {
                ali_trace(&ret);
            }
            str_len = STRLEN(info);
            p = info + str_len;
            p_size -= str_len;
        }
    }
    win_compopup_init(WIN_POPUP_TYPE_OK);
    win_compopup_set_msg_ext(info, NULL, 0);
    win_compopup_set_frame(GET_MID_L(900), GET_MID_T(500), 900, 500);
    win_compopup_open_ext(&back_save);
#endif
}


static PRESULT win_progdetail_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
#ifdef SFU_TEST_SUPPORT
    static int input_num = 0;
#endif
    PRESULT ret = PROC_PASS;
        UINT32  vkey=0;
        //BOOL send_msg_flag=FALSE;
        UINT32 key_transform=0;
        UINT32 wnd_count=0;
        //PRESULT obj_ret = PROC_PASS;

	if(0 == wnd_count)
	{
		;
	}
	if(0 == key_transform)
	{
		;
	}
    switch(event)
    {
    case EVN_PRE_OPEN:
        #ifdef SUPPORT_CAS7
        restore_ca_msg_when_exit_win();
        #endif
        wnd_count=api_inc_wnd_count();
        info_load();
		#ifdef BC_TEST
		vmx_test = FALSE;
		#endif
#if (CC_ON==1)
        cc_vbi_show_on(FALSE);
#endif
        break;
    case EVN_POST_OPEN:
        //win_minisignal_open(pobj);
        win_minisignal_open_ex(pobj,SIGNAL_L,SIGNAL_T);
        win_signal_refresh();
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_STATUS_SIGNAL == param1)
        {
            win_signal_refresh();
        }
        else if(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE == param1)
        {
            info_load();
            osd_draw_object((POBJECT_HEAD)&txt_pidinfo,C_UPDATE_ALL);
        }
        else if(CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE == param1)
        {
            info_load();
            osd_draw_object((POBJECT_HEAD)&txt_pidinfo,C_UPDATE_ALL);
        }

        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        win_minisignal_restore();
        win_signal_close();
            wnd_count=api_dec_wnd_count();
#ifdef ENABLE_ATSC_CC_BY_VBI
            cc_vbi_show_on(TRUE);
#endif
        #ifdef SUPPORT_CAS9
            restore_ca_msg_when_exit_win();
        #endif
        #ifdef SUPPORT_CAS7
        win_ca_mmi_check();
        #endif

		#ifdef BC_TEST
		if(vmx_test)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_vmx_test, TRUE);
		}
		#endif
        break;

    case EVN_UNKNOWNKEY_GOT:

            key_transform=ap_hk_to_vk(0,param1,&vkey);
        if (factorytest_check_magic_key_diag(vkey))
        {
            ap_clear_all_menus();
            //send_msg_flag=
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&win_diag, TRUE);
        }

         if(check_magic_key_firmware_info(vkey))
         {
            if (sys_ic_get_chip_id() >= ALI_S3602F)
            {
                show_firmware_info();
            }
         }

#ifdef SFU_TEST_SUPPORT
        //libc_printf("%s %d\n",__FUNCTION__,vkey);
        if((vkey <= V_KEY_9) /*&& (vkey >= V_KEY_0)*/)
        {
            input_num *= 10;
            input_num += vkey;

            if(SFU_ENTER_NUMBER == input_num)
            {
                 //#define IOBASEADDRRESS  0xB8000000
                 //#if (SYS_CHIP_MODULE == ALI_S3601)
                 //#define EXT_INT_STAT    (IOBASEADDRRESS + 0x30)
                 //#define EXT_INT_MASK    (IOBASEADDRRESS + 0x34)
                 //#elif (SYS_CHIP_MODULE == ALI_S3602)
                 //#define EXT_INT_STAT    (IOBASEADDRRESS + 0x30)
                 //#define EXT_INT_MASK    (IOBASEADDRRESS + 0x38)
                 //#else
                 //#define EXT_INT_STAT    (IOBASEADDRRESS + 0x38)
                 //#define EXT_INT_MASK    (IOBASEADDRRESS + 0x3C)
                 //#endif
                 //disable pan and ir interrupt
                 //*(volatile UINT32*)EXT_INT_MASK =
         //(*(volatile UINT32*)EXT_INT_MASK) & (~0x10080000);
                 //libc_printf("Enter SFU auto test\n");

//                 extern struct pan_device*   g_pan_dev;
//                 pan_close(g_pan_dev);
//Don't need to close panel for diable IR now,
//because it will case IR no respondence after implement 'stop' command.
//                 g_b_is_open_sfu = TRUE;

                 input_num = 0;
                 sfutest_init();
                 ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_progdetail,FALSE);
            }
        }
        else
        {
            input_num = 0;
        }
#endif
        break;

    default:
        break;
    }

    return ret;
}

static void get_text_field_content(char *str, UINT32 str_buf_size, T_NODE *t_node)
{
#ifdef DVBC_SUPPORT
        void *freq_str = NULL;
#endif
        char *fft = NULL;
    char *modulation = NULL;
    int sprintf_ret = 0;
    char *str_cat_ret = NULL;
    INT32 t_len = 0;

	if(NULL == str_cat_ret)
	{
		;
	}
    if(FRONTEND_TYPE_S == t_node->ft_type)
    {
        sprintf_ret=snprintf(str, str_buf_size,"%lu/%s/%lu", t_node->frq, \
                    (0 == t_node->pol) ? "H" : "V",t_node->sym);
    #ifdef MULTISTREAM_SUPPORT
        if (1 == t_node->t2_profile)
        {
            sprintf_ret = snprintf(str + STRLEN(str), str_buf_size - STRLEN(str), "/%c%2d", 'M', t_node->plp_id);
        }
        else
        {
            sprintf_ret = snprintf(str + STRLEN(str), str_buf_size - STRLEN(str), "/%c", 'S');
        }
    #endif
        if(0 == sprintf_ret)
        {
            ali_trace(&sprintf_ret);
        }
    }
#ifdef DVBC_SUPPORT
    else if(FRONTEND_TYPE_C == t_node->ft_type)
    {
        freq_str = (void *)(display_strs[MAX_DISP_STR_ITEMS-2]);
        get_freq_str(t_node->frq, freq_str, 2*(MAX_DISP_STR_LEN),FALSE);
        sprintf_ret=snprintf(str, str_buf_size,"%s/%lu/%s",(char *)freq_str, \
                    t_node->sym, qam_table[t_node->fec_inner - 4]);
        if(0 == sprintf_ret)
        {
            ali_trace(&sprintf_ret);
        }
    }
#endif
    else if(FRONTEND_TYPE_T == t_node->ft_type)
    {
        fft = "";
        if (FFT_MODULATION_2==t_node->FFT )
        {
            fft = "2K";
        }
        else if (FFT_MODULATION_8==t_node->FFT )
        {
            fft = "8K";
        }
        else if (NUM_ONE==t_node->FFT )
        {
            fft = "1K";
        }
        else if (FFT_MODULATION_4==t_node->FFT )
        {
            fft = "4K";
        }
        else if (FFT_MODULATION_16==t_node->FFT )
        {
            fft = "16K";
        }
        else if (FFT_MODULATION_32==t_node->FFT )
        {
            fft = "32K";
        }

        modulation = "";
        if (FFT_MODULATION_DQPSK==t_node->modulation )
        {
            modulation = "DQPSK";
        }
        else if (FFT_MODULATION_QPSK==t_node->modulation )
        {
            modulation = "QPSK";
        }
        else if (FFT_MODULATION_16QAM==t_node->modulation )
        {
            modulation = "16QAM";
        }
        else if (FFT_MODULATION_64QAM==t_node->modulation )
        {
            modulation = "64QAM";
        }
        else if (FFT_MODULATION_256QAM==t_node->modulation )
        {
            modulation = "256QAM";
        }

        //modify for program infor Transponder more "//"
        //sprintf(str,"%dKHz/%s/%s", t_node->frq, FFT, modulation);
        sprintf_ret=snprintf(str, str_buf_size,"%luKHZ",t_node->frq);
        if(0 == sprintf_ret)
        {
            ali_trace(&sprintf_ret);
        }
        if((char *)""!=fft)
        {
            t_len = str_buf_size - strlen(str);
            if (t_len > 1)
            {
                str_cat_ret=strncat(str,"/", t_len-1);
            }
            t_len = str_buf_size - strlen(str);
            if (t_len > (INT32)strlen(fft))
            {
                str_cat_ret=strncat(str,fft, t_len-1);
            }
        }
        if((char *)""!=modulation)
        {
            t_len = str_buf_size - strlen(str);
            if (t_len > 1)
            {
                str_cat_ret=strncat(str,"/", t_len - 1);
            }
            t_len = str_buf_size - strlen(str);
            if (t_len > (INT32)strlen(modulation))
            {
                str_cat_ret=strncat(str,modulation, t_len - 1);
            }
        }
    }
}

static void info_load(void)
{
    UINT8 str_audio_type[32]={0};
    char str[50]={0};
    UINT16 audiopid=0;
    UINT16 ttxpid=0;
    UINT16 subtpid=0;
    UINT32 cur_audio=0;
    TEXT_FIELD *txt=NULL;
    P_NODE p_node;
    T_NODE t_node;
    S_NODE s_node;
    UINT8 nim_fft=0;
    UINT8 nim_modulation=0;
    struct nim_device *priv_nim_dev = NULL;
    struct ts_route_info ts_route;
    int sprintf_ret=0;
    //RET_CODE ts_route_ret=RET_SUCCESS;
    BOOL func_flag=FALSE;
    INT32 tp_ret=0;
    INT32 sat_ret=0;
    UINT32 __MAYBE_UNUSED__ height_one=0;
    UINT32 __MAYBE_UNUSED__ height_two=0;
    UINT32 __MAYBE_UNUSED__ height_three=0;

    height_one=TXT_T + TXT_H;
    height_two=TXT_T + TXT_H*2;
    height_three=TXT_T + TXT_H*3;

	if(0 == sat_ret)
	{
		;
	}
	if(0 == tp_ret)
	{
		;
	}
	if(FALSE == func_flag)
	{
		;
	}
	if(0 == subtpid)
	{
		;
	}
	if(0 == ttxpid)
	{
		;
	}
    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&ts_route, 0x0, sizeof (ts_route));

    func_flag=sys_data_get_curprog_info(&p_node);
    tp_ret=get_tp_by_id(p_node.tp_id,&t_node);
    sat_ret=get_sat_by_id(t_node.sat_id, &s_node);

    if ((FRONTEND_TYPE_T == t_node.ft_type)||(FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        if(RET_FAILURE == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, \
                               NULL, &ts_route))
        {
            PRINTF("ts_route_get_by_type() failed!\n");
        }
        if(NUM_ZERO == ts_route.nim_id)
        {
            priv_nim_dev = g_nim_dev;
        }
        else if(NUM_ONE == ts_route.nim_id)
        {
            priv_nim_dev = g_nim_dev2;
        }
        if(priv_nim_dev != NULL)
        {
            if(priv_nim_dev->get_fftmode)
            {
                priv_nim_dev->get_fftmode(priv_nim_dev,&nim_fft);
                t_node.FFT = nim_fft;
            }
            if(priv_nim_dev->get_modulation)
            {
                priv_nim_dev->get_modulation(priv_nim_dev,&nim_modulation);
                t_node.modulation = nim_modulation;
            }
        }
    }
    cur_audio = (p_node.cur_audio >= p_node.audio_count) ? 0 : p_node.cur_audio;
    audiopid = p_node.audio_pid[cur_audio];
    ttxpid = p_node.teletext_pid;
    subtpid = p_node.subtitle_pid;
    if(NUM_ZERO == p_node.audio_count)
    {
        sprintf_ret=snprintf(str,50,"V %d  No Audio  P %d",\
                    p_node.video_pid,p_node.pcr_pid);
        if(0 == sprintf_ret)
        {
            ali_trace(&sprintf_ret);
        }
    }
    else
    {
        key_check_ddplus_stream_changed(&audiopid);

        if((audiopid  & 0xE000) == AC3_DES_EXIST)
        {
            strncpy((char *)str_audio_type, "Dolby D", (32-1));
        }
        else if((audiopid & 0xE000) == EAC3_DES_EXIST)
        {
            strncpy((char *)str_audio_type, "Dolby D+", (32-1));
        }
        else if((audiopid & 0xE000) == AAC_DES_EXIST)
        {
            strncpy((char *)str_audio_type, "AAC", (32-1));
        }
        else if((audiopid & 0xE000) == ADTS_AAC_DES_EXIST)
        {
            strncpy((char *)str_audio_type, "AAC", (32-1));
        }
        else
        {
            MEMSET(str_audio_type, 0, sizeof(str_audio_type));
        }
        if(NUM_ZERO == str_audio_type[0])
        {
            sprintf_ret=snprintf(str,50,"V %d/ A %d/ P %d",p_node.video_pid, \
                        audiopid & 0x1FFF,p_node.pcr_pid);
            if(0 == sprintf_ret)
            {
                ali_trace(&sprintf_ret);
            }
        }
        else
        {
            sprintf_ret=snprintf(str,50,"V %d/ A %d[%s]/ P %d",p_node.video_pid, \
                        audiopid & 0x1FFF,str_audio_type, p_node.pcr_pid);
            if(0 == sprintf_ret)
            {
                ali_trace(&sprintf_ret);
            }
        }
    }
    txt = &txt_pidinfo;
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)

    txt = &txt_satname;
#ifndef DB_USE_UNICODE_STRING
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)s_node.sat_name);
#else
    osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)s_node.sat_name);
#endif

//modify for program infor 2011 11 05
    if(t_node.ft_type != FRONTEND_TYPE_S)
    {//when the satellite is not DVBS, not show satellite

        remove_menu_item_conwithout_focus(&g_win_progdetail, \
                          (OBJECT_HEAD *)& txt_satname, 0);
        remove_menu_item_conwithout_focus(&g_win_progdetail, \
                          (OBJECT_HEAD *)& txt_string2,  TXT_H );
        //delete the item from the container
        if(height_two==txt_tpinfo.head.frame.u_top)
            osd_move_object((POBJECT_HEAD)&txt_tpinfo, 0, (-TXT_H), TRUE);
        if(height_three==txt_pidinfo.head.frame.u_top)
            osd_move_object((POBJECT_HEAD)&txt_pidinfo, 0, (-TXT_H), TRUE);
        //adjust the postion
    }
    if(t_node.ft_type == FRONTEND_TYPE_S)
    {
        BOOL  no_remove_satname=TRUE, no_remove_string2=TRUE;
        OBJECT_HEAD* obj=osd_get_container_next_obj(&g_win_progdetail);
        while(obj!=NULL)
        {//linkage has txt_satname
            if((OBJECT_HEAD*)&txt_satname==obj)
                no_remove_satname=FALSE;
            obj=osd_get_objp_next(obj);
        }
        if(no_remove_satname)
        {//txt_satname has been removed, then add it to the container
            txt_satname.head.p_next=\
                osd_get_objp_next(((OBJECT_HEAD *)& txt_progname));
            txt_progname.head.p_next=(OBJECT_HEAD *)& txt_satname;
        }
        obj=osd_get_container_next_obj(&g_win_progdetail);
        while(obj!=NULL)
        {//linkage has txt_string2
            if((OBJECT_HEAD*)&txt_string2==obj)
                no_remove_string2=FALSE;
            obj=osd_get_objp_next(obj);
        }
        if(no_remove_string2)
        {
            //txt_string2 has been removed, then add it to the container
            txt_string2.head.p_next=\
                osd_get_objp_next(((OBJECT_HEAD *)& txt_string1));
            txt_string1.head.p_next=(OBJECT_HEAD *)& txt_string2;
        }
        //move down the fllowing textfield
        if(height_one==txt_string3.head.frame.u_top)
        {
            osd_move_object((POBJECT_HEAD)&txt_string3, 0, (TXT_H), TRUE);
        }
        if(height_two==txt_string4.head.frame.u_top)
            osd_move_object((POBJECT_HEAD)&txt_string4, 0, (TXT_H), TRUE);
        if(height_one==txt_tpinfo.head.frame.u_top)
        {
            osd_move_object((POBJECT_HEAD)&txt_tpinfo, 0, (TXT_H), TRUE);
        }
        if(height_two==txt_pidinfo.head.frame.u_top)
            osd_move_object((POBJECT_HEAD)&txt_pidinfo, 0, (TXT_H), TRUE);
    }
//modify end

#endif

    get_text_field_content(str, 50, &t_node);

    txt = &txt_tpinfo;
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

    txt = &txt_progname;
#ifndef DB_USE_UNICODE_STRING
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)p_node.service_name);
#else
    osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)p_node.service_name);
#endif
    txt = &txt_string4;
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"PID");
}

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
//add a function to handle pin checking
//window open after prog detail window opened
void progdetail_win_close()
{
    osd_clear_object((POBJECT_HEAD)&g_win_progdetail, C_UPDATE_ALL);
    menu_stack_pop();
}
#endif
