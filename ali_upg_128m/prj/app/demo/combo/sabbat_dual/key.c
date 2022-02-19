/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: key.c
*
*    Description: This file contains application's key function - key_task().
                  It process input key modules for the application and others misc.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <bus/dog/dog.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>
#include <hld/decv/vdec_driver.h>
#include <hld/deca/deca.h>
#include <api/libsi/si_tdt.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <api/libci/ci_plus.h>
#include "ap_ctrl_ci.h"
#ifdef SUPPORT_BC
#include <bus/otp/otp.h>
#endif
#ifdef SUPPORT_CAS9
#include <hld/dmx/dmx.h>
#include <api/libcas/mcas.h>
#endif

#include <api/libtsg/lib_tsg.h>

#include "./platform/board.h"
#include "sys_config.h"
#include "copper_common/system_data.h"
#include "control.h"
#include "key.h"
#include "vkey.h"
#include "win_com.h"
#include "win_signalstatus.h"
#include "scpi.h"
#include "ctrl_key_proc.h"
#include "win_automatic_standby.h"
#include "win_signal.h"
#ifdef CAS9_VSC_RAP_ENABLE
#include "vsc_rap/vsc_rap.h"
#endif
#ifdef NEW_SELF_TEST_UART_ENABLE
#if defined (SUPPORT_BC_STD)
#include "bc_ap_std/self_test.h"
#else if defined (SUPPORT_BC)
#include "bc_ap/bc_self_test.h"
#endif
#endif
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_ca_common.h"
#endif
#ifdef SFU_TEST_SUPPORT
#include "sfu_test.h"
#endif
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif


/**
  **  Variates define **
  */
#define SIGNAL_PRINTF       PRINTF
#define CA_PROG_ABOR_RESET_COUNT    8

//#define DUMP_KEY_CODE
#define qam_patch_printf(...)    do{}while(0)
//#define QAM_patch_PRINTF        libc_printf

#ifdef _NV_PROJECT_SUPPORT_
#define     INPUT_TASK_PRIORITY     OSAL_PRI_HIGH
#else
#define     INPUT_TASK_PRIORITY     OSAL_PRI_NORMAL
#endif
#define     INPUT_TASK_STACKSIZE        0x1000
#define     INPUT_TASK_QUANTUM      10

#define IR_KEY_MASK 0xFFFFFFFF
#define IR_NEC_KEY_MASK 0xFFFF0000

#define KEY_DETECT_INTERVAL     100
#define SIGNAL_DETECT_DELAY_TIMES (300/KEY_DETECT_INTERVAL)///ms
#define DELAY_SIGNAL_DETECT_DELAY_TIMES (SIGNAL_DETECT_DELAY_TIMES*4)

#define DELAY_MS(ms)      osal_task_sleep(ms)

/**
  *  struct define
  */

/**
  * 0 - Normal, write both to LED and buf
  * 1 - write to LED only
   *2 - write to buf only
  */
enum _WRITE_LED_BUF
{
    WRITE_LED_BUF = 0,
    WRITE_LED,
    WRITE_BUF,
    WRITE_NONE
};


/**
  *  Global  variates
  */
/***************************************************/
/*        ALL kinds of RCU code defintion             */
/***************************************************/
enum signal_flag signal_detect_flag =SIGNAL_CHECK_NORMAL;
UINT8  upgrade_detect_flag = 0;

#ifdef GPIO_RGB_YUV_SWITCH
UINT8 g_rgb_yuv_switch =1;
UINT8 g_rgb_yuv_changed =0;
#endif

UINT8    led_display_flag = 0;
UINT8    led_display_data[10] = {0};
UINT8    led_data_len= 0;

UINT32 app_uart_id = SCI_FOR_RS232;

/**
  *  Local  variates
  */
static OSAL_ID INPUT_TASK_ID = OSAL_INVALID_ID;

static pfn_key_notify_callback m_pfn_key_notify= 0;
static UINT8 set_signal_check_flag = 0;

static UINT16 m_ddplus_dynamic_pid= 0;
static UINT16 m_dd_ddplus_changed= 0;

#ifdef GPIO_RGB_YUV_SWITCH
  static UINT8 m_rgb_yuv_changing= 0;
#endif
static UINT8 m_ci_reset_count= 0;
static UINT32  key_address_mask= 0;
static UINT32  key_address= 0;

static UINT8   lock_status= 0;
static UINT8   lock_status2= 0;
static UINT8   standby_status= 0;

/** for function key_check_signal_status(void) */
#ifdef DISPLAY_SIGNAL_QUALITY_EXT
    static UINT8 quality_pre = 0;
    static UINT16 level_pre = 0;
#else
    static UINT8 quality_pre = 0;
    static UINT16 level_pre = 0;
#endif

static UINT32 delay_time = 0;
static UINT32 wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;

#ifndef _BUILD_OTA_E_
  static UINT32 unlock_times = TIME_COUNT_0;
  static UINT32 unlock_start_tick = 0;
  #ifdef _S3281_
    static UINT32 first_reset = 0;
  #endif
#endif

#ifndef _BUILD_OTA_E_
#ifdef CI_SLOT_DYNAMIC_DETECT
static UINT32 m_my_tick;
#endif
#endif

/** for function key_check_picture_status(void) */
static enum VDEC_PIC_STATUS m_vdec_status = VDEC_PIC_NO_SHOW;
/**Add for avoid EXIT message send out */
static UINT32 m_tick_of_last_nonnormal_status= 0;
static UINT32 m_vdec_pic_no_show_time= 0;
static UINT32 m_vdec_pic_no_show_flag= 0;
static UINT32 m_vdec_display_index= 0;
static UINT32 m_vdec_picture_freeze_flag= 0;
static UINT32 m_vdec_picture_freeze_time= 0;
static UINT8  m_vdec_status_checking= 0;
static UINT8  m_dmx0_scramble= 0;
static UINT32 m_dmx0_scramble_start_time= 0;

#ifdef SUPPORT_CAS9
static UINT32 mmi_tick= 0;
static UINT8 g_hint_sc_error=0; //vicky140318#1 add for #19660
static UINT32 g_av_ng_tick=0; //vicky140318#1 add #19660
#endif

#if(defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
static BOOL c1700a_key_exist = FALSE;
static BOOL c1700a_upg_send = FALSE;
#endif

#ifdef _INVW_JUICE
static int kcnt = 0;
#endif

#ifdef WATCH_DOG_SUPPORT
static OSAL_ID DOG_TASK_ID=OSAL_INVALID_ID;
#endif

/**
  *  Function  statement
  */
void pvr_monitor_task(void);
extern int get_mainmenu_open_stat(void);


/**
  *  Function  entity
  */

void key_set_upgrade_check_flag(UINT8 upgrade_flag)
{
    upgrade_detect_flag = upgrade_flag;
}

void key_set_signal_check_flag(enum signal_flag  flag)
{
    signal_detect_flag = flag;
    set_signal_check_flag = 1;
    set_signal_scramble_fake(0);
    g_signal_scram_flag = 0;
    g_signal_unlock_flag = 0;
#ifndef _BUILD_OTA_E_
    ap_ci_reset_ca_pmt_cnt();
#endif
    m_ci_reset_count = 0;
    g_ca_prog_reset_flag = 0;
}

pfn_key_notify_callback ap_set_key_notify_proc(pfn_key_notify_callback pfn_notify)
{
    pfn_key_notify_callback old_proc = m_pfn_key_notify;

    m_pfn_key_notify = pfn_notify;

    return old_proc;
}

UINT32 ap_key_msg_code_to_vkey(UINT32 msg_code)
{
    UINT32 vkey= 0;

    ap_hk_to_vk(0, msg_code, &vkey);

    return vkey;
}

BOOL ap_key_map_use_hkey(UINT32 msg_code, PFN_KEY_MAP key_map)
{
    VACTION action = VACT_PASS;
    UINT32  vkey = V_KEY_NULL;
    UINT32  i = 0;

    /*If no keymap function,it means the object doesn't process key message */
    if(NULL == key_map)
    {
        return FALSE;
    }

    do
    {
        i = ap_hk_to_vk(i, msg_code, &vkey);
        if(V_KEY_NULL == vkey)
        {
            break;
        }
        action = key_map(NULL, vkey);
    } while (VACT_PASS == action);

    return (action != VACT_PASS);
}


static UINT8 key_device_init(void)
{

#if 0
    g_pan_dev = (struct pan_device *) dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
    if(NULL == g_pan_dev)
    {
        PRINTF("dev_get_by_name failed\n");
        return FALSE;
    }

    retval = pan_open(g_pan_dev);
    if(retval!=SUCCESS)
    {
        PRINTF("pan_open failed retval=%d\n",retval);
        return FALSE;
    }
    pan_display(g_pan_dev," on ", 4);
#endif
    if ( NULL == g_pan_dev )
    {
        return FALSE;
    }

    led_data_len = g_pan_dev->led_num;

    PRINTF("---------UIKeyInit() ,led display ----\n");
    return TRUE;
}


BOOL key_get_key(struct pan_key *key_struct, UINT32 timeout)
{
    struct pan_key *pan_key = NULL;

    pan_key = pan_get_key(g_pan_dev,timeout);
#ifdef SFU_TEST_SUPPORT
    if (sfutesting_is_turn_on())
    {
        pan_key = NULL;
    }
#endif
    if ( NULL == pan_key )
    {
        return FALSE;
    }

    MEMCPY(key_struct, pan_key, sizeof(struct pan_key));
    if (PAN_KEY_INVALID == key_struct->code)
    {
        return FALSE;
    }

#ifdef AUTOMATIC_STANDBY
    ap_set_access_active(TRUE);
    if(get_automatic_windows_state())
    {
        return FALSE;
    }
#endif

#ifdef FSC_SUPPORT
    extern screen_back_state_t screen_back_state;
    extern UINT32 g_fsc_change_time;
    if(((key_struct->code == 0x60df22dd) || (key_struct->code == 0x60dfb847))
        && (key_struct->state == PAN_KEY_PRESSED)
        && (screen_back_state == SCREEN_BACK_VIDEO))
    {        
        g_fsc_change_time = osal_get_tick();  
#ifdef FSC_TICK_PRINTF
        UINT32 cur_tick = osal_get_tick();
        libc_printf("FSC Tick->%s %d \n",__FUNCTION__,((cur_tick >= g_fsc_change_time)?(cur_tick - g_fsc_change_time):
            (0xFFFFFFFF - g_fsc_change_time) - cur_tick));
#endif
    }
#endif    
    return TRUE;
}

void key_check_signal_status(void)
{
#ifndef _BUILD_OTA_E_
    sys_state_t system_state = SYS_STATE_INITIALIZING;
#endif
    UINT8 quality= 0;
    UINT8 lock= 0;
    UINT8 lock_ano= 0;
    UINT8 quality_cur= 0;
    UINT8 level_cur = 0;
    UINT16 level = 0;
    UINT32 msgcode= 0;
    UINT32 cur_ber= 0;
    struct nim_device *nim_dev = NULL;
    struct nim_device *nim_another = NULL;
	
    /* Signal detect */
    if (SINGLE_TUNER == g_tuner_num)
    {
        cur_tuner_idx = 0;
    }

    if(0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
        nim_another = g_nim_dev2;
    }
    else
    {
        nim_dev = g_nim_dev2;
        nim_another = g_nim_dev;
    }

#ifdef FSC_SUPPORT
    fcc_dump_tuner_info();
#endif

    //nim_get_lock(nim_dev, &lock);
    	//Send a msg for scrolling textfield.
	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING, 0, FALSE);

    delay_time++;

    if( SIGNAL_CHECK_PAUSE == signal_detect_flag)
    {
        osal_task_sleep(10);
        return;
    }
    else if(SIGNAL_CHECK_RESET == signal_detect_flag)
    {
        //signal_detect_flag = SIGNAL_CHECK_NORMAL;
        //wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;
        //delay_time = 0;
        if(set_signal_check_flag)
        {
            wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;
            delay_time = 0;
        }
    }

    if(set_signal_check_flag)
    {
        set_signal_check_flag = 0;
    }

    if(delay_time == wait_time)
    {
        if( DELAY_SIGNAL_DETECT_DELAY_TIMES == wait_time )
        {
            signal_detect_flag = SIGNAL_CHECK_NORMAL;
        }

        nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
        nim_get_agc(nim_dev, &level_cur);
#endif
        nim_get_snr(nim_dev, &quality_cur);
        //nim_get_BER(nim_dev, &cur_ber);
        if(NULL != nim_dev->do_ioctl)
        {
          nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level_cur);
          nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_BER, (UINT32)&cur_ber);
        }
        //if((lock) && (psys_data->ber_printf))
       //     libc_printf("Current BER is %d 10e-8\n",cur_ber);
       //NIM_PRINTF( "current BER is  %d /1000000\n", ber_value);

        level = (level_cur + level_pre)/2;
        quality = (quality_cur + quality_pre)/2;
        level_pre = level_cur;
        quality_pre = quality_cur;

        dem_signal_to_display(lock,(UINT8 *)(&level),&quality);
        msgcode =(cur_tuner_idx<<24) + (level<<16) + (quality<<8) + lock;
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SIGNAL, msgcode, FALSE);
#ifndef _BUILD_OTA_E_
        if(play_chan_nim_busy)
        {
        	level = 0;
        	quality = 0;
            lock = 0;
        }
#endif
        win_signal_set_level_quality(level,quality,lock);

#ifdef SHOW_TWO_TUNER_LOCK_STATUS
        if(DUAL_TUNER == g_tuner_num )
        {
            nim_get_lock(nim_another, &lock_ano);
            if(nim_dev == g_nim_dev)
            {
                key_pan_display_lock(lock);         // first tuner lock status
                key_pan_display_lock2(lock_ano);    //second tuner lock status
            }
            else
            {
                key_pan_display_lock(lock_ano);     // first tuner lock status
                key_pan_display_lock2(lock);        //second tuner lock status
            }
        }
        else
        {
            key_pan_display_lock(lock);
        }
#else
        key_pan_display_lock(lock);
#endif
        wait_time = SIGNAL_DETECT_DELAY_TIMES;
        delay_time = 0;

#ifndef _BUILD_OTA_E_
        // when the signal is missed, try to reset the diseqc and qpsk.
        #ifdef _S3281_
        if(!lock &&( unlock_times >  TIME_COUNT_2))
        {
            if( 0 == first_reset )
            {
                first_reset++;
                wincom_reset_nim();
            }
        }
        #endif

        if( TIME_COUNT_0 == unlock_times )
            {
            unlock_start_tick = osal_get_tick();
            }

        if(lock)
        {
            unlock_times = TIME_COUNT_0;
        }
        else
        {
            unlock_times++;
        }
        if(unlock_times > TIME_COUNT_14)
        {
            unlock_times = TIME_COUNT_0;
            //libc_printf("reset tick = %d(ms)\n", osal_get_tick() - unlock_start_tick);
#ifndef NEW_DEMO_FRAME
            system_state = api_get_system_state();
            if((SYS_STATE_NORMAL == system_state ) && ( 0 != get_cc_crnt_state()  ))
            {
                //libc_printf("reset diseqc and qpsk\n");
                cc_reset_diseqc(FALSE);
                cc_reset_qpsk(FALSE);
            }
#else
            system_state = api_get_system_state();
            if( SYS_STATE_NORMAL == system_state )
            {
                //libc_printf("reset diseqc and qpsk\n");
                wincom_reset_nim();
            }
#endif
        }
#endif

    }
}

#ifndef CAS9_VSC_RAP_ENABLE
static UINT32 key_monitor_com(void)
{
    UINT8 i=0;
    UINT8 j=0;
    UINT8 ch = 0xff;
    UINT32 sci_timeout_0 = 1000;
    UINT32 sci_timeout_1 = 10;
	if(0 == j)
	{
		;
	}
#if((defined SCPI_SUPPORT))
    UINT32 uifindstrcmd=0;
    ER ret_val=OSAL_E_FAIL;
#endif
#ifdef SUPPORT_BC
   UINT32 uart_fuse = 0;
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    UINT8 *comtest_command = (UINT8 *)"vmxtest";
#else
    UINT8 *comtest_command = (UINT8 *)"comtest";
#endif
    UINT8 *apptest_command = (UINT8 *)"APP  init ok";
    INT32 comtest_command_len = STRLEN((const char *)comtest_command) - 1;
    INT32 apptest_command_len = STRLEN((const char *)apptest_command) - 1;

#if defined (SUPPORT_BC) || defined (SUPPORT_BC_STD)
    #ifdef NEW_SELF_TEST_UART_ENABLE
    if( TRUE == is_self_test_mode_activate )
    {
        self_test_uart_main();
    }
    #endif
#endif

    if(SYS_STATE_UPGRAGE_SLAVE == api_get_system_state())
    {
        return UPGRADE_NONE_MODE;
    }

    if(UPGRADE_CHECK_HOST & upgrade_detect_flag)
    {
        while( SUCCESS == sci_read_tm(app_uart_id, &ch, sci_timeout_1) )
        {
            if(apptest_command[i]==ch)
            {
                i++;
                if(i>apptest_command_len)
                {
                    return UPGRADE_HOST_MODE;
                }
            }
        }
    }

#ifndef _CAS9_CA_ENABLE_
    if(UPGRADE_CHECK_SLAVE & upgrade_detect_flag)
#endif
    {
        i = 0;
        j = 0;
        while(SUCCESS==sci_read_tm(app_uart_id,&ch,sci_timeout_0))
        {
            #if((defined SCPI_SUPPORT))
            uifindstrcmd=1;
            //osal_task_dispatch_off();
            if((NULL!=g_pby_snd_string_buf) && (j < UART_CMD_STR_LEN_MAX))
            {
                g_pby_snd_string_buf[j++]=ch;
            }
            #endif

            if ((i<=comtest_command_len) && (comtest_command[i] == ch))
            {
                sci_timeout_0 = 1000*100;
                sci_write(app_uart_id,ch);
                i++ ;
          /** Is a comtest command string, wait for upgrade */
                if (i > comtest_command_len)
                {
                    #ifdef SUPPORT_BC
                    if( SUCCESS == sci_read_tm(app_uart_id,&ch,sci_timeout_0) )
                    {
                        switch(ch)
                        {
                            case '1':
                                if( 0 != BC_CheckNSc())
                                {
                                    sci_write(app_uart_id,ch);
                                    return UPGRADE_SLAVE_MODE;
                                }
                                break;

                            case '2':
                                {
                                    UINT32 ejtag_disable = 0;

                                    otp_init(NULL);
                                    otp_read(0x3*4, &ejtag_disable, 4);

                                    if( 0 == (ejtag_disable & (1<<0)) )
                                    {
                                        sci_write(app_uart_id,ch);
                                        return UPGRADE_SLAVE_MODE;
                                        //goto end;
                                    }
                                }
                                break;

                            case '3':
                                {
                                    sci_write(app_uart_id,ch);
                                    return UPGRADE_SLAVE_MODE;
                                }
                                break;

                            case '4':
                                {
                                    otp_init(NULL);
                                    otp_read(0x3*4, &uart_fuse, 4);
                                    if( 0 == (uart_fuse&(1<<12)) )
                                    {
                                        sci_write(app_uart_id,ch);
                                        return UPGRADE_SLAVE_MODE;
                                    }
                                }
                                break;

                            default:
                                break;
                        }
                    }
                    #else
                        return UPGRADE_SLAVE_MODE;
                    #endif
                }
            }
    #ifndef SCPI_SUPPORT
            else
            {
                    break;
            }
     #endif
        }
    }

#if((defined SCPI_SUPPORT))
    if( (NULL!= g_pby_snd_string_buf) && (1== uifindstrcmd) )
    {
        uifindstrcmd=0;
        //osal_task_dispatch_on();
        ret_val = osal_msgqueue_send(g_msg_qid_scpi,g_pby_snd_string_buf,strlen(g_pby_snd_string_buf)+1, 0);
        if(OSAL_E_OK != ret_val)
        {
            MEMSET(g_pby_snd_string_buf,0,UART_CMD_STR_LEN_MAX+1);
        }
    }
#endif

    return 0;
}
#endif

#ifdef CI_SUPPORT

/* *Add this code-segment for two reason:
 *    1. Send Eixt message because, CAM pop up menu: no access, but video comeout
 *         just a few moment, however, the menu didn't close.
 *    2. We close menu for reason1, but it close CAM software update menu when video
 *        freeze slightly. So add one condition to avoid Exit message send out in this
 *        situation.
 */
static void ci_info_ui_exit_patch(void)
{
    UINT32 c_tick = 0;
    c_tick = osal_get_tick();

    if (VDEC_PIC_NO_SHOW == m_vdec_status)
    {
        m_tick_of_last_nonnormal_status = 0;
    }

    /* Don't send Exit msg if video just freeze slightly*/
    if (win_ci_info_on_screen() \
        && (c_tick-m_tick_of_last_nonnormal_status)>1000
        )
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_EXIT, FALSE);
    }
}
#endif
static void ddplus_dynamic_pid_patch(void)
{
    UINT32 audio_str_type = 0;
    RET_CODE ret_code = RET_FAILURE;

     if (AUDIO_TYPE_TEST(m_ddplus_dynamic_pid, AC3_DES_EXIST) \
                   || AUDIO_TYPE_TEST(m_ddplus_dynamic_pid, EAC3_DES_EXIST))
        {
                ret_code = deca_io_control(g_deca_dev, DECA_GET_STR_TYPE, (UINT32)&audio_str_type);

                if ( RET_SUCCESS == ret_code )
                {
                    if (( AUDIO_EC3 == audio_str_type )  \
                && AUDIO_TYPE_TEST(m_ddplus_dynamic_pid, AC3_DES_EXIST))
                    {
                        m_ddplus_dynamic_pid = (m_ddplus_dynamic_pid & 0x1fff) | EAC3_DES_EXIST;
                        m_dd_ddplus_changed = 1;
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE,
                        m_ddplus_dynamic_pid, 0);
                    }
                    else if (( AUDIO_AC3 == audio_str_type ) \
                && AUDIO_TYPE_TEST(m_ddplus_dynamic_pid, EAC3_DES_EXIST))
                    {
                        m_ddplus_dynamic_pid = (m_ddplus_dynamic_pid & 0x1fff) | AC3_DES_EXIST;
                        m_dd_ddplus_changed = 1;
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUDIO_TYPE_UPDATE,
                        m_ddplus_dynamic_pid, 0);
                    }
                }
        }
        else
        {
                m_dd_ddplus_changed = 0;
         }
    return ;
}

static UINT32 key_check_picture_status(void)
{
    UINT8 scramble= 0;
    enum VDEC_PIC_STATUS vdec_status = VDEC_PIC_NO_SHOW;
    struct vdec_status_info curstatus ;

    MEMSET(&curstatus, 0, sizeof(struct vdec_status_info));
    m_vdec_status_checking = 1;
#ifdef CI_SUPPORT
    if (win_ci_enquiry_dlg_show())
    {
      scramble = 0;
    }
    else
#endif
    {
        scramble = api_is_stream_scrambled();
    }

    if (scramble != m_dmx0_scramble )
    {
        m_dmx0_scramble = scramble;
        m_dmx0_scramble_start_time = osal_get_tick();
    }
    else
    {
        if ( 0 == m_dmx0_scramble_start_time )
            {
            m_dmx0_scramble_start_time = osal_get_tick();
            }
    }

    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&curstatus);

    if (!curstatus.u_first_pic_showed)
    {
        vdec_status = VDEC_PIC_NO_SHOW;
        m_vdec_display_index = 0;
        if (0 == m_vdec_pic_no_show_flag)
        {
            m_vdec_pic_no_show_time = osal_get_tick();
            m_vdec_pic_no_show_flag = 1;
        }
        else
        {
            if ( (VDEC_DECODING != curstatus.u_cur_status )|| (FALSE == curstatus.display_frm)
#ifdef CI_SUPPORT
                || win_ci_enquiry_dlg_show()
#endif
                )
    {
                m_vdec_pic_no_show_time = osal_get_tick();
    }
       }
    }
    else
    {
        vdec_status = VDEC_PIC_NORMAL;
        m_vdec_pic_no_show_flag = 0;
        if ( 0 == m_vdec_display_index )
        {
            m_vdec_display_index = curstatus.display_idx;
            m_vdec_picture_freeze_flag = 0;
        }
    else if (m_vdec_display_index == curstatus.display_idx)
        {
                vdec_status = VDEC_PIC_FREEZE;
                if ( 0 == m_vdec_picture_freeze_flag  )
                {
                    m_vdec_picture_freeze_time = osal_get_tick();
                    m_vdec_picture_freeze_flag = 1;
                }
                else if (( VDEC_DECODING == curstatus.u_cur_status)  || ( FALSE == curstatus.display_frm)
#ifdef CI_SUPPORT
                        || win_ci_enquiry_dlg_show()
#endif
                  )
        {
                        m_vdec_picture_freeze_time = osal_get_tick();
        }
         }
         else
         {
                m_vdec_picture_freeze_flag = 0;
                m_vdec_picture_freeze_time = 0;
                m_vdec_display_index = curstatus.display_idx;
         }
        if ((( VDEC_PIC_NO_SHOW == m_vdec_status)  || (VDEC_PIC_FREEZE == m_vdec_status ))  \
        && (VDEC_PIC_NORMAL == vdec_status ))
        {
        #ifdef CI_SUPPORT
              ci_info_ui_exit_patch();
        #endif
        }
        else
            {
        ddplus_dynamic_pid_patch();
            }
    }
    m_vdec_status = vdec_status;
    m_tick_of_last_nonnormal_status = osal_get_tick();
    m_vdec_status_checking = 0;

    return 0;
}

enum VDEC_PIC_STATUS key_get_video_status(UINT32 *start_time)
{
    enum VDEC_PIC_STATUS vdec_status = m_vdec_status;

    while (m_vdec_status_checking)
    {
        osal_task_sleep(10);
    }

    if (start_time)
    {
        if ( VDEC_PIC_FREEZE == vdec_status )
            {
            *start_time = m_vdec_picture_freeze_time;
            }
        else if ( VDEC_PIC_NO_SHOW == vdec_status )
            {
            *start_time = m_vdec_pic_no_show_time;
            }
    }

    return vdec_status;
}

void key_update_video_status_start_time(void)
{

    while (m_vdec_status_checking)
    {
        osal_task_sleep(10);
    }

    if ( VDEC_PIC_FREEZE == m_vdec_status )
    {
        m_vdec_picture_freeze_time = osal_get_tick();
    }
    else if ( VDEC_PIC_NO_SHOW == m_vdec_status )
    {
        m_vdec_pic_no_show_time = osal_get_tick();
    }
#ifndef _BUILD_OTA_E_
    ap_ci_reset_ca_pmt_status();
    ap_ci_reset_ca_pmt_cnt();
#endif
}

BOOL key_get_dmx0_scramble(UINT32 *start_time)
{

    while (m_vdec_status_checking)
    {
        osal_task_sleep(10);
    }

    if (start_time)
    {
        *start_time = m_dmx0_scramble_start_time;
    }

    return m_dmx0_scramble;
}

void key_check_ddplus_prog(void)
{
    UINT16 chan_idx= 0;
    UINT16 audio_id = 0;
    UINT32 cur_audio = 0;
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
    get_prog_at(chan_idx, &p_node);
    cur_audio = (p_node.cur_audio >= p_node.audio_count) ? 0 : p_node.cur_audio;
    audio_id = p_node.audio_pid[cur_audio];

    if ((AUDIO_TYPE_TEST(audio_id, AC3_DES_EXIST) \
        || AUDIO_TYPE_TEST(audio_id, EAC3_DES_EXIST))
        )
    {
        m_ddplus_dynamic_pid = audio_id;
    }
    else
    {
        m_ddplus_dynamic_pid = 0;
    }
}

void set_ddplus_check_pid(UINT16 pid)
{
    if ((AUDIO_TYPE_TEST(pid, AC3_DES_EXIST) \
        || AUDIO_TYPE_TEST(pid, EAC3_DES_EXIST))
        )
    {
        m_ddplus_dynamic_pid = pid;
    }
    else
    {
        m_ddplus_dynamic_pid = 0;
    }
    //libc_printf("set_ddplus_check_pid 0x%x\n", pid);
}
#ifndef _BUILD_OTA_E_
#ifdef CI_SLOT_DYNAMIC_DETECT
void key_check_ca_prog(UINT8 cam_pmt_ok)
{
    /*fake-scrable*/
    signal_lock_status lock_flag = 0;
    signal_scramble_status scramble_flag = 0;
    signal_lnbshort_status lnbshort_flag = 0;
    signal_parentlock_status parrentlock_flag = 0;
    UINT8 scramble = 0;
    UINT16 chan_idx = 0;
    P_NODE p_node;
    UINT8 ca_pmt_ok_cnt = 0;
    UINT32 pic_time = 0;
    UINT32 scramble_time = 0;
    UINT32 cur_audio = 0;
    UINT32 cur_time = osal_get_tick();
    enum VDEC_PIC_STATUS pic_status = VDEC_PIC_NO_SHOW;
    struct vdec_status_info curstatus ;

    MEMSET(&p_node,0,sizeof(P_NODE));
    pic_status = key_get_video_status(&pic_time);
    scramble = key_get_dmx0_scramble(&scramble_time);

#ifdef DVR_PVR_SUPPORT2
    if (cam_pmt_ok && !api_pvr_is_live_playing())
    {
        if (scramble && cur_time >= (scramble_time + 1000))
        {
            //libc_printf("reset pvr player because scrambled\n");
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN, MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
            ap_ci_reset_ca_pmt_cnt();
            ap_ci_reset_ca_pmt_status();
        }
        return;
    }
#endif

    sys_data_get_cur_group_channel(&chan_idx, sys_data_get_cur_chan_mode());
    get_prog_at(chan_idx, &p_node);
    prog_end_time_tick = osal_get_tick();

    cur_audio = (p_node.cur_audio >= p_node.audio_count) ? 0 : p_node.cur_audio;
    m_ddplus_dynamic_pid = p_node.audio_pid[cur_audio];
    if (!AUDIO_TYPE_TEST(m_ddplus_dynamic_pid, AC3_DES_EXIST|EAC3_DES_EXIST))
    {
        m_ddplus_dynamic_pid = 0;
    }

    if ( 0 == m_my_tick )
    {
        m_my_tick = prog_end_time_tick;
    }
#if(!defined CC_USE_TSG_PLAYER && defined CI_SLOT_DYNAMIC_DETECT)
    cc_tsg_ci_slot_task_proc();
#endif

    /* for fake-scramble prog */
    if  ((p_node.ca_mode) && api_is_playing_tv())
    {
        get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag, NULL);
        if(( SIGNAL_STATUS_LOCK == lock_flag ) && ( SIGNAL_STATUS_UNSCRAMBLED == scramble_flag )
            && prog_start_time_tick && (prog_end_time_tick > (prog_start_time_tick + 3000))
#ifdef DVR_PVR_SUPPORT
            && api_pvr_is_live_playing() && !freeze_for_pvr
#endif
            )
        {
            if ( (VDEC_PIC_NO_SHOW == pic_status)  || (VDEC_PIC_FREEZE == pic_status ))
            {
                if (cur_time >= (pic_time + 6000))
                {
                    if (scramble && cur_time >= (scramble_time + 300))
                    {
                        //libc_printf("normal play no pic and scrambled\n");
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN, MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                        key_update_video_status_start_time();
                    }
                    else if (!scramble && cur_time >= (scramble_time + 5000))
                    {
#ifdef CI_SUPPORT
                        //libc_printf("normal play no pic and not scramble\n");
                        if (api_ci_attached()
#ifdef DVR_VR_SUPPORT
                        && cc_tsg_task_is_running()
#endif
                        )
                        {
                            if (m_ci_reset_count < TIME_COUNT_3)
                            {
#ifdef DVR_PVR_SUPPORT
                                cc_tsg_ci_slot_switch(FALSE);
#endif
                                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,
                                            MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                                m_ci_reset_count++;
                            }
                            else
                            {
#ifdef DVR_PVR_SUPPORT
                                cc_tsg_ci_slot_switch(FALSE);
#endif
                                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,
                                            MSG_SCRIMBLE_SIGN_SEND_CA_PMT, FALSE);
                            }
                        }
                        else
#endif
                        {
                            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,
                                        MSG_SCRIMBLE_SIGN_SEND_CA_PMT, FALSE);
                        }
                        key_update_video_status_start_time();
                    }
                }
            }
            else if (pic_status == VDEC_PIC_NORMAL)
            {
                m_ci_reset_count = TIME_COUNT_0;
#ifdef CI_SUPPORT
                if (FALSE == api_ci_attached())
                {
#endif
                    set_signal_scramble_fake(1);
                    if (get_signal_stataus_show())
                    {
                        //ShowSignalStatusOSDOnOff(0);
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,MSG_SCRIMBLE_SIGN_NO_OSD_SHOW,FALSE);
                    }
#ifdef CI_SUPPORT
                }
#endif
            }
        }
        else if(( SIGNAL_STATUS_LOCK == lock_flag ) && ( SIGNAL_STATUS_SCRAMBLED == scramble_flag)
#ifdef DVR_PVR_SUPPORT
            && api_pvr_is_live_playing() && !freeze_for_pvr
#endif
#ifdef CI_SUPPORT
            && (api_ci_attached())
#endif
            )
        {
#ifdef  CC_USE_TSG_PLAYER
            if (cam_pmt_ok)
            {
                ca_pmt_ok_cnt = ap_ci_get_ca_pmt_ok_cnt();
                if (ca_pmt_ok_cnt)
                {
                    if ( (VDEC_PIC_NO_SHOW == pic_status)  || (VDEC_PIC_FREEZE == pic_status))
                    {
                    #ifdef CI_SUPPORT
                        if (cur_time >= (pic_time + 1000))
                        {
                            if (scramble && (cur_time >= (scramble_time + 200))
#ifdef DVR_PVR_SUPPORT
                    && cc_tsg_task_is_running()
#endif
                )
                            {
#ifdef DVR_PVR_SUPPORT
                                if (!api_pvr_is_recording_cur_prog() || g_ca_prog_reset_flag < CA_PROG_ABOR_RESET_COUNT)
#endif
                                {
                                    //libc_printf("reset normal player because scrambled %d\n", cur_time);
#ifdef DVR_PVR_SUPPORT
                                    if (g_ca_prog_reset_flag >= 1)
                                    {
                                        cc_tsg_ci_slot_switch(TRUE);
                                    }
#endif
                                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN,
                                                MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                                }
                                ap_ci_reset_ca_pmt_cnt();
                                ap_ci_reset_ca_pmt_status();
                                return;
                            }
                            else if (!scramble && cur_time >= (scramble_time + 3000))
                            {
                                ap_ci_reset_ca_pmt_cnt();
                                ap_ci_reset_ca_pmt_status();
                                return;
                            }
                        }
                    #endif
                    }
                }
            }
#endif
            if ( (VDEC_PIC_NO_SHOW == pic_status)  || (VDEC_PIC_FREEZE == pic_status) )
            {
                if (cur_time >= (pic_time + 5000))
                {
                    if (scramble && cur_time >= (scramble_time + 5000))
                    {
#ifdef DVR_PVR_SUPPORT
                        if (cc_tsg_ci_slot_switch(FALSE))
#endif
                        {
                            libc_printf("normal play no pic and scrambled\n");
                            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCRIMBLE_SIGN, MSG_SCRIMBLE_SIGN_RESET_PLAYER, FALSE);
                        }
                        key_update_video_status_start_time();
                    }
                }
            }

        }
    }
}
#endif
#endif

void key_check_ddplus_stream_changed(UINT16 *p_audio_pid)
{

	if(NULL == p_audio_pid)
	{
		return ;
	}

    if (m_ddplus_dynamic_pid) //&& m_dd_ddplus_changed)
    {
        if ((*p_audio_pid & 0x1fff) == (m_ddplus_dynamic_pid & 0x1fff))
        {
            *p_audio_pid = m_ddplus_dynamic_pid;
            //libc_printf("DD+ audio type changed: 0x%04x\n", m_ddplus_dynamic_pid);
            m_dd_ddplus_changed = 0;
            return ;//TRUE;
        }
        m_dd_ddplus_changed = 0;
    }

    return ;//FALSE;
}

#if defined( SUPPORT_CAS9)
static void key_check_mmi(void)
{
    UINT8 av_ok = 0;
    UINT8 i = 0;
    UINT8 av_try = 0;
#ifdef CAS9_V6 //mmi_0904
    INT32 diff_tick = 0;
    INT32 card_msg_dur_timer = 7000 ;
    UINT8 scramble_type=0;
#endif
    UINT32 duration = 0;
    UINT32 set_duration = 0;
    UINT32 vde_dipy_idx = 0;
    struct vdec_status_info curstatus;
    struct vdec_device *vdec_handle = NULL;
    sys_state_t cur_sys_state = SYS_STATE_NORMAL;

    MEMSET(&curstatus, 0, sizeof(struct vdec_status_info));
    if(FALSE == need_to_check_mmi())
    {
        return;
    }

    if(api_get_ca_mmi_new_msg_flag())
    {
        api_set_ca_mmi_new_msg_flag(FALSE);
        mmi_tick = osal_get_tick();
    }

    duration = osal_get_tick() - mmi_tick;
    if(CA_MMI_PRI_01_SMC== get_mmi_showed() )
    {
        set_duration=3000;//6000;
    }
    else if( CA_MMI_PRI_06_BASIC== get_mmi_showed() )
    {
        set_duration=300;
    }

    vdec_handle = get_selected_decoder();
    cur_sys_state = api_get_system_state();
    if(duration>=set_duration)
    {
        if(SYS_STATE_USB_MP != cur_sys_state)
        {
            if(FALSE == ca_is_card_inited()) //no card inserted
            {
                if(FALSE == get_mainmenu_open_stat())
                {
                    vdec_start(vdec_handle); //fixed bug:can play unscramble stream when pull out card in scramble stream.
                }
            }
            else if(ca_is_card_inited() && (CA_MMI_PRI_01_SMC == get_mmi_showed())) 
            {
                //for vsc
                if((TRUE == api_is_displaying_blackscreen())
                    && (0==cur_channel_is_scrambled(&scramble_type,FALSE)))
                {
                    vdec_start(vdec_handle);
                }
            }
        }
        vdec_io_control(vdec_handle, VDEC_IO_GET_STATUS, (UINT32)&curstatus);
        if( TRUE == curstatus.u_first_pic_showed )
        {
            //libc_printf("2\n");
            av_try = TIME_COUNT_3;
            for(i=0;i<=av_try;i++)
            {
                vde_dipy_idx = curstatus.display_idx;

                osal_task_sleep(200);
                vdec_io_control(vdec_handle, VDEC_IO_GET_STATUS, (UINT32)&curstatus);

                if(vde_dipy_idx == curstatus.display_idx)
                {
                    break;
                }
            }
            if(i>av_try)
            {
                av_ok=1;
                 g_hint_sc_error=0;   //vicky140318#1 fix #19660
                 g_av_ng_tick=0;
            }
            else
            {
                //av _ng  //vicky140318#1 fix #19660
                if(0==g_av_ng_tick)
                {
                    g_av_ng_tick=osal_get_tick();
                }
            }
        }
        if( 1 == av_ok )
        {
           #ifdef CAS9_V6 //mmi_0904
             diff_tick = osal_get_tick() - get_cardproblem_tick();

           /* let msg display over 7 sec. */
             if ((CA_MMI_PRI_01_SMC== get_mmi_showed() ) && ( diff_tick >= card_msg_dur_timer ))
           #else
                 if ( CA_MMI_PRI_01_SMC== get_mmi_showed() )
           #endif
                {
                	#ifdef CAS9_V6  //Fix compiler error when CAS9_V6 disabled
                    //vicky140318#1 A.63, Newglee PT, MMI-card problem, no time out.
                    if((RS_CONAX_INCORRECT_CARD==api_get_ca_card_msg_id()) ||
                        (RS_CONAX_CARD_PROBLEM==api_get_ca_card_msg_id())
                        )
                    {
                        reset_cardproblem_tick();
                    }
                    else
                    //End
                  #endif
                    {
                        MMI_PRINTF("video ok, MCAS_DISP_CARD_IN\n");
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_IN<<16, FALSE);
                    }
                }
                else if ( CA_MMI_PRI_06_BASIC== get_mmi_showed())
                {
                    #ifdef CAS9_V6
                    if ( ((TRUE==conax_get_access_granted()) && (TRUE!=api_is_stream_scrambled()))
                    || (0==cur_channel_is_scrambled(&scramble_type,FALSE)))
                    #endif
                    {
                        MMI_PRINTF("video ok, MCAS_DISP_NO_ACCESS_CLR\n");
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_NO_ACCESS_CLR<<16, FALSE);
                    }
                }
                //1229 INCORRECT CARD HANDER.        
                if ( CA_MMI_PRI_01_SMC== get_mmi_showed() \
                    && (RS_CONAX_INCORRECT_CARD==api_get_ca_card_msg_id()))
                {
                    //if(0==cur_channel_is_scrambled(&scramble_type,FALSE))//FTA program
                    {
                        MMI_PRINTF("video ok, MCAS_DISP_CARD_IN to clear incorrect msg\n");
                        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, (MCAS_DISP_CARD_IN<<16|1), FALSE);
                    }
                }
        }

        #ifdef CAS9_V6  //Fix compiler error when CAS9_V6 disabled
        //vicky140318#1 fix #19660
        if( (1!=av_ok) && (0==g_hint_sc_error))
        {
            //libc_printf("dt-%d,showed(%d)\n",osal_get_tick()-g_av_ng_tick); //vicky140318#1
            if((osal_get_tick()-g_av_ng_tick)>3000)
            {
                if((TRUE==cur_channel_is_scrambled(&scramble_type,FALSE))
                    && ( FALSE==ca_is_card_inited()))
                {
                    //libc_printf("%s-%d: Force show sc error again\n",__FUNCTION__,__LINE__); //vicky140318#1
                    //ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_ERROR<<16, FALSE);
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0,FALSE);
                    g_hint_sc_error=1;
                }
            }
        }
        #endif
    }
}

#endif


#if(defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
BOOL c1700a_key_existed(void)
{
        return c1700a_key_exist;
}

void c1700a_key_is_existed(void)
{
        c1700a_key_exist = TRUE;
}

UINT32 c1700a_upgrade_monitor_com(void)
{
    UINT8 i = 0;
    UINT8 ch = 0xff;
    UINT8 *comtest_command = "sertest"; //"comtest"
    INT32 comtest_command_len = STRLEN(comtest_command) - 1;
    UINT32 timeout = 1000;

    while (sci_read_tm(SCI_FOR_RS232, &ch, timeout) == SUCCESS)
    {
        if ((i <= comtest_command_len) && (comtest_command[i] == ch))
        {
            timeout = 1000 * 100;
            sci_write(SCI_FOR_RS232, ch);
            i++;
            if (i > comtest_command_len)
             /* Is a comtest command string, wait for upgrade */
            {
                return 1;
            }
        }
        else
            break;
    }

    return 0;
}
#endif

#ifdef _C0200A_CA_ENABLE_
UINT32 c0200a_upgrade_monitor_com(void)
{
    UINT8 i = 0;
    UINT8 ch = 0xff;
    UINT8 *comtest_command = "comtest"; //"comtest"
    INT32 comtest_command_len = STRLEN(comtest_command) - 1;
    UINT32 timeout = 1000;

    while (sci_read_tm(SCI_FOR_RS232, &ch, timeout) == SUCCESS)
    {
        if ((i <= comtest_command_len) && (comtest_command[i] == ch))
        {
            timeout = 1000 * 100;
            sci_write(SCI_FOR_RS232, ch);
            i++;
            if (i > comtest_command_len)
             /* Is a comtest command string, wait for upgrade */
            {
                return 1;
            }
        }
        else
            break;
    }

    return 0;
}
#endif

static void patch_for_qam(void)
{
    UINT8 lock_flag= 0;
    UINT32 temp = 0;
    UINT32 per= 0;
    UINT32 dis_count = 0;
    UINT32 pkt_num = 0;
    UINT32 con_dep_dmx_pkt_num = 10000 ;
    UINT32 dmx_discon_count_num = 1000;
    UINT32 nim_need_reset_per = 100;
    struct dmx_device *dev = NULL;
    struct nim_device *nim = NULL;

     //add for S3281 QAM patch
     dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
     nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

     nim_get_lock(nim, &lock_flag);
     if(lock_flag)
     {
    dmx_io_control(dev, IO_DMX_GET_PACKET_NUM, (UINT32)(&pkt_num));
    qam_patch_printf("pkt_num is %d\n", pkt_num);
    if(pkt_num >= con_dep_dmx_pkt_num)
    {
        dmx_io_control(dev, IO_GET_DISCONTINUE_COUNT, (UINT32)(&dis_count));
        qam_patch_printf("dis_count is %d\n", dis_count);
        dmx_io_control(dev, IO_DMX_GLEAR_PACKET_NUM, 0);
        if(dis_count > dmx_discon_count_num)
        {
            dmx_io_control(dev, IO_CLEAR_DISCONTINUE_COUNT, 0);
            nim_io_control(nim, NIM_DRIVER_READ_RSUB, (UINT32)(&per));
            qam_patch_printf("per is %d\n", per);
            if(per < nim_need_reset_per)
            {
                nim_ioctl_ext(nim, NIM_DRIVER_RESET_QAM_FSM, (void *)(&temp));
                qam_patch_printf("QAM reset done !\n");
            }
        }
    }
       }
    //add end

    return ;
}

#ifdef GPIO_RGB_YUV_SWITCH
static void set_rgb_yuv_switch_flag(void)
{
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    UINT8 rgb_yuv_sw = 0;

    system_state = api_get_system_state();
    if (system_state == SYS_STATE_NORMAL)
    {
        if (g_rgb_yuv_changed)
        {
            continue;
        }

        rgb_yuv_sw = sys_get_rgb_yuv_switch();

        if (m_rgb_yuv_changing == 0)
        {
            if (g_rgb_yuv_switch != rgb_yuv_sw)
            {
                m_rgb_yuv_changing = 1;
            }
        }
        else
        {
            m_rgb_yuv_changing = 0;
            if (g_rgb_yuv_switch != rgb_yuv_sw)
            {
                g_rgb_yuv_switch = rgb_yuv_sw;
                g_rgb_yuv_changed = 1;
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH, rgb_yuv_sw, FALSE);
            }
        }
    }

}
#endif


static void key_task(void)
{
    UINT32  ret= 0;
    UINT32  msg_code= 0;
    UINT32 hkey1 = INVALID_HK;
    board_cfg *cfg = NULL;
    struct pan_key key_struct;

#ifdef CI_SLOT_DYNAMIC_DETECT
    BOOL ca_pmt_ok = FALSE;
#endif
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    ret = 0;
    MEMSET(&key_struct, 0, sizeof(struct pan_key));
    signal_detect_flag = SIGNAL_CHECK_RESET;
    key_device_init();
    while(1)
    {
        system_state = api_get_system_state();
        osal_task_sleep(KEY_DETECT_INTERVAL);
        if( SYS_STATE_INITIALIZING == system_state  \
            || SYS_STATE_POWER_OFF ==  system_state  \
            || SYS_STATE_UPGRAGE_HOST == system_state )
            {
                continue;
            }
#ifdef _C0200A_CA_ENABLE_
        /* Host software upgrade detecting. */
        ret = c0200a_upgrade_monitor_com();
        if ( 1 == ret )
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
        }
#endif            
#if(defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
        /* Host software upgrade detecting. */
        if( FALSE == c1700a_key_existed()  &&  FALSE == c1700a_upg_send )
        {
            ret = c1700a_upgrade_monitor_com();
            if ( 1 == ret )
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
            }
              c1700a_upg_send = TRUE;
        }
#endif
#ifdef SFU_TEST_SUPPORT
        if(sfutesting_is_turn_on())
        {
            osal_task_sleep(10);
            continue;
        }
#endif
        /* Host software upgrade detecting. */
#ifndef CAS9_VSC_RAP_ENABLE
#if(defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
        if( TRUE == c1700a_key_existed() )
        {
            ret = key_monitor_com();
            if((UPGRADE_HOST_MODE == ret ) || (UPGRADE_SLAVE_MODE == ret))
            {
                if(UPGRADE_HOST_MODE == ret )
                {
           #ifdef UPGRADE_HOST_MONITOR
                        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE, 0, TRUE);
          #else
                    ;
          #endif
               }
                else
               {
                    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
               }
            }
        }
#else /* else of defined(MULTI_CAS) && defined(SUPPORT_CAS_A) */

#ifndef FPGA_TEST
        /* Host software upgrade detecting. */
        ret = key_monitor_com();
        if( (UPGRADE_HOST_MODE == ret) || (UPGRADE_SLAVE_MODE == ret))
        {
            if(ret != UPGRADE_HOST_MODE)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
            }
#ifdef UPGRADE_HOST_MONITOR
            else
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE, 0, TRUE);
            }
#endif
        }
#endif
#endif
#else
       vsc_rap_receive ();
#endif
        if(ap_is_key_task_get_key() && key_get_key(&key_struct, 1)
#ifdef CI_PLUS_SUPPORT
            && (FALSE == ap_ciplus_is_upgrading())
#endif
        )
        {
#ifdef DUMP_KEY_CODE
            if(PAN_KEY_PRESSED== key_struct.state)
            {
                libc_printf("key = 0x%08x\n", key_struct.code);
            }
            continue;
#endif
            msg_code = scan_code_to_msg_code(&key_struct);
#ifdef _INVW_JUICE
#if 1 // ndef NDEBUG
          {
          if (key_struct.state)
            {
              kcnt++;
                 libc_printf(" ++++ %s: key = (%01X %01X %02X %04X) count = %3d\n",
                 __FUNCTION__,
                 key_struct.type,
                 key_struct.state,
                 key_struct.count,
                 SET_IRCODE(key_struct.code),
                 kcnt);
            }
          }
#endif
#endif
            if (m_pfn_key_notify)
            {
                m_pfn_key_notify(msg_code);
            }
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
        }
    key_check_picture_status();

#ifdef SFU_TEST_SUPPORT
//        extern volatile BOOL g_b_is_open_sfu;
//        if((!g_b_is_open_sfu)||(!SFUTestingIsTurnOn()))
        if( !sfutesting_is_turn_on() )
#endif
        {
        key_check_signal_status();
#if defined( SUPPORT_CAS9)
        key_check_mmi();
#endif
#if defined( SUPPORT_CAS7)
        cas7_check_mmi();
#endif
        }
#ifdef DVR_PVR_SUPPORT
        pvr_monitor_task();
#endif
#ifndef _BUILD_OTA_E_
#ifdef CI_SLOT_DYNAMIC_DETECT
        ca_pmt_ok = ap_ci_is_ca_pmt_ok();
        key_check_ca_prog((UINT8)ca_pmt_ok);
#endif
#endif

#ifdef GPIO_RGB_YUV_SWITCH
     set_rgb_yuv_switch_flag();
#endif
        cfg = get_board_cfg();
        if( cfg && (cfg->fp_standby_key ))
        {
            if( chip_gpio_get(cfg->fp_standby_key) )//the standby key is being pressed.
            {
                DELAY_MS(50);
                if( chip_gpio_get(cfg->fp_standby_key) )//the standby key is still being pressed.
                {
                    ap_vk_to_hk(0,V_KEY_POWER,&hkey1);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey1, FALSE);
                }
            }
        }
    patch_for_qam();
#ifdef AUTOMATIC_STANDBY
    automatic_standby_loop();
#endif
    }//while(1)
}

//-----------------------------------------------------------------------------
// FUNCTION:    key_init:
//
// DESCRIPTION: init the keyboard devices(including frontpanel and remote controller) hardware
//
// RETURN:      True    Successful
//              False   Error when init the hardware
//
// NOTES:
//
//-----------------------------------------------------------------------------
UINT8 key_init(void)
{
    OSAL_T_CTSK t_ctsk;

//  ir_key_map= &ir_key_maps[0];

//  for(i=0;i<SUPPORT_IR_NUM;i++)
//  {
//      if(ir_key_maps[i].irp_moudle == SYS_IRP_MOUDLE)
//      {
//          ir_key_map = &ir_key_maps[i];
//          break;
//      }
//  }
    MEMSET(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
    t_ctsk.stksz    =       INPUT_TASK_STACKSIZE * 4;
    t_ctsk.quantum  =   INPUT_TASK_QUANTUM;
    t_ctsk.itskpri  =       INPUT_TASK_PRIORITY;
    t_ctsk.name[0]  = 'K';
    t_ctsk.name[1]  = 'E';
    t_ctsk.name[2]  = 'Y';
    t_ctsk.task = (FP)key_task;

    INPUT_TASK_ID = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID==INPUT_TASK_ID)
    {
        return FALSE;
    }

    //rcu_fp_type.rcu_sw_pos = sys_data_get()->rcupos;
    return TRUE;
}

void key_pan_display(char *data, UINT32 len)
{
    UINT8 str[10] = {0};
    INT32 idx= 0;

    if ( NULL == data )
    {
        libc_printf(" %s : data is NULL ! \n", __FUNCTION__);
        return ;
    }

    if((WRITE_LED_BUF == led_display_flag)  || (WRITE_LED == led_display_flag))
    {
        str[0] = data[0];
        idx = 1;
        str[idx] = data[1];
        idx++;
        str[idx] = data[2];
        idx++;
        str[idx] = data[3];
        idx++;
        if(TIME_COUNT_5 == len )
        {
            str[idx] = data[4];
        }

        pan_display(g_pan_dev,(char *)str,idx);
    }

    if( (WRITE_LED_BUF == led_display_flag)  ||(WRITE_BUF ==  led_display_flag) )
    {
        MEMCPY(led_display_data,data,len);
        led_display_data[len] = '\0';
    }
}

void key_pan_display_channel(UINT16 channel)
{
    char program[6]  = {0};
    UINT8 frontend_kind = 0;
    UINT32 prog_num = 0;
    P_NODE p_node;
    T_NODE t_node;

	if(0 == frontend_kind)
	{
		;
	}
	MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    if ( NULL == g_pan_dev )
    {
        return;
    }

    frontend_kind = get_combo_config_frontend();
    prog_num = channel+1;

    get_prog_at(channel, &p_node);
    get_tp_by_id(p_node.tp_id, &t_node);
#ifdef _LCN_ENABLE_
    if( TIME_COUNT_2 == frontend_kind )
    {
        //it's two different kind frontend
        if(sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 1) && sys_data_get_lcn())
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%02d%1d", t_node.remote_control_key_id, (p_node.prog_number & 0x07)+1);
            }
            else
            {
                snprintf(program,6,"%02d%01d%1d", t_node.remote_control_key_id, (p_node.prog_number & 0x1f)>>3,\
                (p_node.prog_number & 0x07)+1);
            }
        }
        else if(sys_data_get_cur_group_sat_id() == get_frontend_satid(FRONTEND_TYPE_T, 0) && sys_data_get_lcn())
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%03d",p_node.LCN);
            }
            else
            {
                snprintf(program,6,"%04d",p_node.LCN);
            }
        }
        else
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%03lu",prog_num);
            }
               else
               {
                snprintf(program,6,"%04lu",prog_num);
               }
        }
    }
    else if( TIME_COUNT_1 == frontend_kind )
    {
        //only one kind frontend
        //this is ISDBT
        if(check_frontend_type(FRONTEND_TYPE_ISDBT, 1) && sys_data_get_lcn())
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%02d%1d", t_node.remote_control_key_id, (p_node.prog_number & 0x07)+1);
            }
            else
            {
                snprintf(program,6,"%02d:%01d%1d", t_node.remote_control_key_id, (p_node.prog_number & 0x1f)>>3, \
                            (p_node.prog_number & 0x07)+1);
            }
        }
        //this is DVBT
        else if(check_frontend_type(FRONTEND_TYPE_T, 0) && sys_data_get_lcn())
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%03d",p_node.LCN);
            }
            else
            {
                snprintf(program,6,"%04d",p_node.LCN);
            }
        }
        //this is DVBC
	else if(check_frontend_type(FRONTEND_TYPE_C, 0)&& sys_data_get_lcn())
	{
		if (g_pan_dev->led_num == 3)
			sprintf(program,"%03d",p_node.LCN);
		else
			sprintf(program,"%04d",p_node.LCN);
	}
        else
        {
            if(TIME_COUNT_3 == g_pan_dev->led_num)
            {
                snprintf(program,6,"%03lu",prog_num);
            }
            else
            {
                snprintf(program,6,"%04lu",prog_num);
            }
        }

    }
    else
#endif
    {
        if( TIME_COUNT_3 == g_pan_dev->led_num )
            {
            snprintf(program,6,"%03lu",prog_num);
            }
        else
            {
            snprintf(program,6,"%04lu",prog_num);
            }
    }
    key_pan_display(program, g_pan_dev->led_num+1);

}

void key_pan_display_lock(UINT8 flag)
{
    UINT8 uchannel[4] = {0};
    UINT8 backup_show_state= 0;

    if(flag != lock_status)
    {
        uchannel[0] = 27;
        uchannel[1] = PAN_ESC_CMD_LBD;
        uchannel[2] = PAN_ESC_CMD_LBD_LOCK ;
        if(flag)
            {
            uchannel[3] = PAN_ESC_CMD_LBD_ON;
            }
        else
            {
            uchannel[3] = PAN_ESC_CMD_LBD_OFF;
            }
        backup_show_state = led_display_flag;
        led_display_flag  = 1;
        key_pan_display((char *)uchannel, 4);
        led_display_flag = backup_show_state;
        lock_status = flag;
    }
}

void key_pan_display_lock2(UINT8 flag)
{
    UINT8 uchannel[4] = {0};
    UINT8 postion=3; // for second tuner lock led.
    UINT8 backup_show_state= 0;

    if(flag != lock_status2)
    {
        uchannel[0] = 27;
        uchannel[1] = PAN_ESC_CMD_LBD;
        uchannel[2] = PAN_ESC_CMD_LBD_LOCK ;
        if(flag)
            {
            uchannel[3] = PAN_ESC_CMD_LBD_ON|(postion<<4);
            }
        else
            {
            uchannel[3] = PAN_ESC_CMD_LBD_OFF|(postion<<4);
            }
        backup_show_state = led_display_flag;
        led_display_flag  = 1;
        key_pan_display((char *)uchannel, 4);
        led_display_flag = backup_show_state;
        lock_status2 = flag;
    }
}


void key_pan_display_standby(UINT8  flag)
{
    UINT8 uchannel[4] = {0};

    if(flag != standby_status)
    {
        uchannel[0] = 27;
        uchannel[1] = PAN_ESC_CMD_LBD;
        uchannel[2] = PAN_ESC_CMD_LBD_FUNCA;
        if(flag)
            {
            uchannel[3] = PAN_ESC_CMD_LBD_ON;
            }
        else
            {
            uchannel[3] = PAN_ESC_CMD_LBD_OFF;
            }
        key_pan_display((char *)uchannel, 4);
        standby_status = flag;
    }
}

#if 0
static void key_pan_display_power(UINT8 flag)
{
    UINT8   uchannel[4] = {0};

    uchannel[0] = 27;
    uchannel[1] = PAN_ESC_CMD_LBD;
    uchannel[2] = PAN_ESC_CMD_LBD_POWER;
    if(flag)
    {
        uchannel[3] = PAN_ESC_CMD_LBD_ON;
    }
    else
    {
        uchannel[3] = PAN_ESC_CMD_LBD_OFF;
    }
    key_pan_display(uchannel, 4);
}
#endif

void key_pan_set_key_table(ir_key_map_t *key_tab)
{
    UINT32  i = 0;
    UINT32  address = 0;

    if ( NULL == key_tab)
    {
        libc_printf( " %s : key_tab is NULL ! \n", __FUNCTION__);
        return ;
    }
/*
    #define SUPPORT_IR_NUM 0

    for(i=0;i<SUPPORT_IR_NUM;i++)
    {
        if(key_tab[i].irp_moudle == SYS_IRP_MOUDLE)
        {
            break;
        }
    }
*/
    //ASSERT(i < SUPPORT_IR_NUM);
    address = key_tab[i].irkey_addr;//LV_IR008_Address[rcu_sw_pos];
    switch(key_tab->irp_protocal)
    {
        case IRP_NEC:
            key_address = address << 16;
            key_address_mask = IR_NEC_KEY_MASK;
            break;
        case IRP_ITT:
        case IRP_NRC17:
        case IRP_SHARP:
        case IRP_SIRC:
        case IRP_RC5:
        case IRP_RC6:
        default:
            return;//hwIrKey = ((0xFF - address) << 24) + (address << 16) + ((0xFF - code) << 8) + code;
    }
}

#ifdef WATCH_DOG_SUPPORT
static void dog_task(void)
{
    UINT32 audio_str_type = 0;

    dog_m3327e_attach(1);
    dog_mode_set(0, DOG_MODE_WATCHDOG, 60000000, NULL);

    while(1)
    {
        dog_set_time(0, 0);
        if(g_deca_dev)
            deca_io_control(g_deca_dev, DECA_GET_STR_TYPE, (UINT32)&audio_str_type);//remote call
        #ifdef _MHEG5_SUPPORT_
        osal_task_sleep(500);
        #else
        osal_task_sleep(100);
        #endif
    }

    return ;
}
#endif

UINT8 dog_init(void)
{
#ifdef WATCH_DOG_SUPPORT
    OSAL_T_CTSK     t_ctsk;

    MEMSET(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
    t_ctsk.stksz    =       INPUT_TASK_STACKSIZE;
    t_ctsk.quantum  =   INPUT_TASK_QUANTUM;
    #ifdef _MHEG5_SUPPORT_
    t_ctsk.itskpri  =       OSAL_PRI_HIGH;
    #else
    t_ctsk.itskpri  =       INPUT_TASK_PRIORITY;
    #endif
    t_ctsk.name[0]  = 'D';
    t_ctsk.name[1]  = 'O';
    t_ctsk.name[2]  = 'G';
    t_ctsk.task = (FP)dog_task;

    DOG_TASK_ID = osal_task_create(&t_ctsk);

    if(OSAL_INVALID_ID == DOG_TASK_ID)
    {
        return FALSE;
    }
#else
    dog_stop(0);
#endif
    return TRUE;
}

