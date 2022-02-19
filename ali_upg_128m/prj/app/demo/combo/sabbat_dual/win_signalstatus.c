/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_signalstatus.c
*
*    Description: signal status menu.
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
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hal/hal_gpio.h>

#ifdef SAT2IP_SUPPORT
#include <api/libsat2ip/satip_client_config.h>
#endif

#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif

#ifdef SUPPORT_CAS9
#include "hld/smc/smc.h"
#endif
#include <api/libosd/osd_lib.h>

#include "images.id"
#include "string.id"

#include "osdobjs_def.h"

#include "osd_config.h"
#include "win_com.h"

#include "win_signalstatus.h"
#include "ctrl_util.h"

#include "key.h"
#include "win_pvr_ctrl_bar.h"

#ifdef SUPPORT_BC
#include "bc_ap/bc_cas.h"
#endif

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

//struct vdec_device * get_selected_decoder(void);

#define ICON_ID_SIGNAL_NO               IM_SIGNAL_NO
#define ICON_ID_SIGNAL_LNBSHORT     IM_SIGNAL_NO
#define ICON_ID_SIGNAL_SCRAMBLED        IM_SIGNAL_NO
#define ICON_ID_SIGNAL_PARRENTLOCK  IM_SIGNAL_NO


#define STRING_ID_SIGNAL_NO             RS_MSG_NO_SIGNAL
#define STRING_ID_SIGNAL_LNBSHORT       RS_SYSTEM_KEY_AUDIO_WARNING
#define STRING_ID_SIGNAL_SCRAMBLED      RS_SYSTME_CHANNEL_SCRAMBLED
#define STRING_ID_SIGNAL_PARRENTLOCK        RS_SYSTME_PARENTAL_LOCK

#define UNLOCK_TIMES_3 3
#define UNLOCK_TIMES_10 10
#define DESC_DET_TIME_3 3
#define CA_PMT_CMD_CNT_2 2
/*******************************************************************************
* WINDOW's objects declaration
*******************************************************************************/

/*******************************************************************************
*   WINDOW's objects defintion MACRO
*******************************************************************************/
#define WIN_NOSIG_IDX       WSTL_TRANS_IX //WSTL_TRANS_IX
#ifdef OSD_16BIT_SUPPORT
    #define NOSIG_TXT_IDX   WSTL_TEXT_10_HD //WSTL_MIXBACK_WHITE_IDX
#else
    #define NOSIG_TXT_IDX   WSTL_MIXBACK_IDX_02_8BIT //WSTL_MIXBACK_WHITE_IDX
#endif
#define NOSIG_BMP_IDX       WSTL_MIXBACK_IDX_06_8BIT //WSTL_MIXBACK_BLACK_IDX

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)  \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, NOSIG_TXT_IDX, NOSIG_TXT_IDX, NOSIG_TXT_IDX,NOSIG_TXT_IDX,   \
        NULL, NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)       \
    DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, NOSIG_BMP_IDX,NOSIG_BMP_IDX,NOSIG_BMP_IDX,NOSIG_BMP_IDX,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, WSTL_TRANS_IX,WSTL_TRANS_IX,WSTL_TRANS_IX,WSTL_TRANS_IX,   \
        NULL,NULL,  \
        nxt_obj, focus_id,0)

#ifndef SD_UI
#define W_W 200
#define W_H  140
#define W_L  GET_MID_L(W_W) //404//362    //498 //204//192
#define W_T  GET_MID_T(W_H) // 208    //208 //90

#define POP_OFFSET_L    60

#define BMP1_L  W_L//548 //246
#define BMP1_T  W_T//208 //90
#define BMP1_W  W_W//98  //94
#define BMP1_H  98  //94

#define TXT_L   W_L //192
#define TXT_T   (W_T + 100)  //308 //188
#define TXT_W   W_W
#define TXT_H   40

#else
#define W_L  204//362    //498 //204//192
#define W_T  90    //208 //90
#define W_W 200
#define W_H  128

#define POP_OFFSET_L    60

#define BMP1_L  257//548 //246
#define BMP1_T  90//208 //90
#define BMP1_W  94//98  //94
#define BMP1_H  94  //94

#define TXT_L   W_L //192
#define TXT_T   188  //308 //188
#define TXT_W   200
#define TXT_H   26
#endif
/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
LDEF_BMP(&g_win_sigstatus, sigstatus_bmp1, &sigstatus_txt,  BMP1_L, BMP1_T, BMP1_W, BMP1_H, ICON_ID_SIGNAL_NO)
LDEF_TXT(&g_win_sigstatus,sigstatus_txt,NULL, TXT_L, TXT_T,TXT_W, TXT_H, STRING_ID_SIGNAL_NO, NULL)

LDEF_WIN(g_win_sigstatus, &sigstatus_bmp1, W_L, W_T, W_W, W_H, 1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
#define WIN_SIGNAL_SHOW_MASK    0xffff  // 0xffff -- 16 times
#ifdef SUPPORT_BC
#define WIN_SCRAMBLE_SHOW_MASK    0x0003  // 0x0003 -- 2 times
#endif

#if(SYS_LNB_SHORT_DET == SYS_FUNC_ON)
#define LNB_POWER_RESET_WAIT_TIME 100
#endif

#if(defined(SYS_12V_SWITCH) && defined(SYS_12V_SHORT_DET))
#if(SYS_12V_SWITCH==SYS_FUNC_ON && SYS_12V_SHORT_DET == SYS_FUNC_ON)
#define V12_POWER_RESET_WAIT_TIME 100
#endif
#endif

BOOL is_no_signal_status = FALSE;
#ifdef DVR_PVR_SUPPORT
static UINT8 singal_play_chan_nim_busy = 0;
#ifdef CI_SUPPORT2
static UINT32 last_ca_pmt_cmd_time = 0;
#endif
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC) || defined CI_SUPPORT2
static UINT8 ca_pmt_cmd_cnt = 0;
#endif

static signal_lock_status      lv_lock_status      = SIGNAL_STATUS_LOCK;
static signal_scramble_status  lv_scramble_status  = SIGNAL_STATUS_UNSCRAMBLED;
static signal_lnbshort_status  lv_lnbshort_status  = SIGNAL_STATUS_LNBNORMAL;
static signal_parentlock_status lv_parrentlock_status = SIGNAL_STATUS_PARENT_UNLOCK;
#ifdef PARENTAL_SUPPORT
static signal_ratinglock_status lv_ratinglock_status = SIGNAL_STATUS_RATING_UNLOCK;
static signal_pvr_ratinglock_status lv_pvr_ratinglock_status = SIGNAL_STATUS_PVR_RATING_UNLOCK;
#endif
UINT8  lv_scramble_fake = 0;

UINT32 g_signal_scram_flag = 0;
UINT32 g_signal_unlock_flag = 0;

#if(SYS_LNB_SHORT_DET == SYS_FUNC_ON)
UINT8 lnb_power_short = SIGNAL_STATUS_LNBNORMAL;
#endif
#ifdef SYS_12V_SHORT_DET
#if(SYS_12V_SHORT_DET== SYS_FUNC_ON)
UINT8 v12_power_short = SIGNAL_STATUS_LNBNORMAL;
#endif
#endif

static BOOL signal_stataus_showed = FALSE;
static UINT8   lnb_power_detect_start = 0;
static UINT16 prechan = 0;
static UINT32 desc_det_time = 0;
static BOOL show_signal_status_enabled = TRUE;
/*******************************************************************************
*  WINDOW's  keymap, proc and callback
*******************************************************************************/

/*******************************************************************************
*   Menu Item ---  callback, keymap and proc
*******************************************************************************/

/*******************************************************************************
*   Window handle's open,proc and handle
*******************************************************************************/

static BOOL win_signal_status_show_unlocked(void)
{
    return (WIN_SIGNAL_SHOW_MASK == (g_signal_unlock_flag&WIN_SIGNAL_SHOW_MASK));
}

static BOOL win_signal_status_show_scrambled(void)
{
#ifdef SUPPORT_BC
    return (WIN_SCRAMBLE_SHOW_MASK == (g_signal_scram_flag & WIN_SCRAMBLE_SHOW_MASK));
#else
    return (WIN_SIGNAL_SHOW_MASK == (g_signal_scram_flag&WIN_SIGNAL_SHOW_MASK));
#endif
}

static void set_singnal_str(void)
{
    TEXT_FIELD *txt = NULL;
    BITMAP *bmp = NULL;

    UINT16 str_id = 0;
    UINT16 image_id = 0;

    txt = &sigstatus_txt;
    bmp = &sigstatus_bmp1;

    if(SIGNAL_STATUS_PARENT_LOCK == lv_parrentlock_status)
    {
        image_id = ICON_ID_SIGNAL_PARRENTLOCK;
        str_id = STRING_ID_SIGNAL_PARRENTLOCK;
    }
#ifdef PARENTAL_SUPPORT
    else if (SIGNAL_STATUS_RATING_LOCK == lv_ratinglock_status)
    {
        image_id = ICON_ID_SIGNAL_PARRENTLOCK;
        str_id = STRING_ID_SIGNAL_PARRENTLOCK;
    }
#endif
    else if (win_signal_status_show_unlocked())
    {
        image_id = ICON_ID_SIGNAL_NO;
        str_id = STRING_ID_SIGNAL_NO;
    }
    else if (win_signal_status_show_scrambled())
    {
        image_id = ICON_ID_SIGNAL_SCRAMBLED;
        str_id = STRING_ID_SIGNAL_SCRAMBLED;
    }
    else if((SIGNAL_STATUS_LNBSHORT == lv_lnbshort_status)
#ifdef SYS_12V_SHORT_DET
#if(SYS_12V_SHORT_DET== SYS_FUNC_ON)
        || (SIGNAL_STATUS_LNBSHORT == v12_power_short)
#endif
#endif
            )
    {
        image_id = ICON_ID_SIGNAL_LNBSHORT;
        str_id = STRING_ID_SIGNAL_LNBSHORT;
    }
    else
    {
        image_id = 0;
        str_id = 0;
    }

    osd_set_text_field_content(txt, STRING_ID, str_id);
    osd_set_bitmap_content(bmp, image_id);
}

static void set_signal_lock_status(signal_lock_status lock_flag)
{
    lv_lock_status = lock_flag;
    g_signal_unlock_flag <<= 1;
    if (SIGNAL_STATUS_UNLOCK == lock_flag)
    {
        g_signal_unlock_flag++;
    }
    set_singnal_str();
}

static void set_signal_scramble_status(signal_scramble_status lnbshort_flag)
{
    lv_scramble_status = lnbshort_flag;
    g_signal_scram_flag <<= 1;
    if (SIGNAL_STATUS_SCRAMBLED == lnbshort_flag)
    {
        g_signal_scram_flag++;
    }
    set_singnal_str();
}

static void set_signal_lnbshort_status(signal_lock_status lnbshort_flag)
{
    lv_lnbshort_status = lnbshort_flag ;
    set_singnal_str();
}

void set_channel_parrent_lock(signal_parentlock_status parrentlock_flag)
{
    lv_parrentlock_status = parrentlock_flag;
    set_singnal_str();
}

BOOL get_channel_parrent_lock(void)
{
    return (SIGNAL_STATUS_PARENT_LOCK == lv_parrentlock_status) ? TRUE : FALSE;
}

#ifdef PARENTAL_SUPPORT
void set_rating_lock(signal_ratinglock_status ratinglock_flag)
{
    lv_ratinglock_status = ratinglock_flag;
    set_singnal_str();
}
BOOL get_rating_lock(void)
{
    return (SIGNAL_STATUS_RATING_LOCK == lv_ratinglock_status) ? TRUE : FALSE;
}
void set_pvrrating_lock(signal_pvr_ratinglock_status ratinglock_flag)
{
    lv_pvr_ratinglock_status = ratinglock_flag;
    set_singnal_str();
}
BOOL get_pvrrating_lock(void)
{
    return (SIGNAL_STATUS_PVR_RATING_LOCK == lv_pvr_ratinglock_status)?TRUE:FALSE;
}
#endif
BOOL get_channel_scramble(void)
{
    return (SIGNAL_STATUS_SCRAMBLED == lv_scramble_status) ? TRUE : FALSE;
}

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
static BOOL fprescrambled=FALSE;
static UINT8 get_bc_descramble_state(UINT8 scrabled, UINT32 ca_mode, struct vdec_status_info *cur_status,UINT8 lock)
{
    BOOL fbcscrambled=FALSE;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    UINT32 freeze_flag = 0;

    system_state = api_get_system_state();

    #ifdef SUPPORT_BC_STD
    if (get_ca_descrambling_state() && (ca_mode)
    #else
    if (get_ca_descrambling_state(bc_get_viewing_service_idx()) && (ca_mode)
    #endif
        && (!scrabled) && lock && (SCREEN_BACK_VIDEO == screen_back_state) && (VIEW_MODE_FULL == hde_get_mode()))
    {
        if(NULL == cur_status)
        {
            return 0;
        }
        freeze_flag = cur_status->display_idx;

        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
        if((cur_status->u_first_pic_showed)&&(VDEC_DECODING == cur_status->u_cur_status))
                    //for scramble prog and freeze screen(show black screen and scramble osd)
        {
            osal_task_sleep(100);
            vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
            if(freeze_flag == cur_status->display_idx)
            {
                osal_task_sleep(100);
                vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
                if(freeze_flag == cur_status->display_idx) // freeze screen state!
                {
                    fbcscrambled = 1;
                }
                else
                {
                    ca_pmt_cmd_cnt = 0;
                }
            }
            else
            {
                ca_pmt_cmd_cnt = 0;
            }
        }
        else if(!cur_status->u_first_pic_showed) //for scramble prog and black screen
        {
            fbcscrambled = 1;
            ca_pmt_cmd_cnt = 0;
        }
    }
    if(fbcscrambled)
    {
        if(FALSE == fprescrambled)
        {
            if(SYS_STATE_USB_PVR != system_state)
            {
                //libc_printf("%s():call api_disp_blackscreen @ line %d\n", __func__, __LINE__);
                api_disp_blackscreen(TRUE, FALSE);
                api_disp_blackscreen(FALSE, FALSE);
            }
            fprescrambled = TRUE;
        }
    }
    else
    {
        fprescrambled = FALSE;
    }
    scrabled |=fbcscrambled;
    return scrabled;
}
#endif

#ifdef CI_SUPPORT2
static UINT8 get_ci2_descramble_state(UINT8 scrabled,P_NODE *p_node, struct vdec_status_info *cur_status)
{
    UINT32 freeze_flag = 0;

    if(NULL == cur_status)
    {
        return 0;
    }
    freeze_flag = cur_status->display_idx;

    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
    if((cur_status->u_first_pic_showed)&&(VDEC_DECODING == cur_status->u_cur_status))
    //for scramble prog and freeze screen (show black screen and scramble osd)
    {
        osal_task_sleep(100);
        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
        if(freeze_flag == cur_status->display_idx)
        {
            osal_task_sleep(100);
            vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)cur_status);
            if(freeze_flag == cur_status->display_idx) // freeze screen state!
            {
                scrabled = 1;
#ifdef DVR_PVR_SUPPORT
                //add to simulate monitor's function for video hold state because monitor is off under DVR project!
                if((!last_ca_pmt_cmd_time)||(last_ca_pmt_cmd_time && (osal_get_tick() > last_ca_pmt_cmd_time + 6000)))
                {
                    if(ca_pmt_cmd_cnt >= CA_PMT_CMD_CNT_2)
                    {
                        uich_chg_play_prog(0,CC_CMD_RESET_CRNT_CH);
                        ca_pmt_cmd_cnt = 0;
                    }
                    else
                    {
                        cc_send_ca_pmt(p_node->prog_id);//cc_send_ca_pmt();
                        ca_pmt_cmd_cnt++;
                    }
                    last_ca_pmt_cmd_time = osal_get_tick();
                }
#endif
            }
            else
            {
                ca_pmt_cmd_cnt = 0;
            }
        }
        else
        {
            ca_pmt_cmd_cnt = 0;
        }
    }
    else if(!cur_status->u_first_pic_showed) //for scramble prog and black screen
    {
        scrabled = 1;
        ca_pmt_cmd_cnt = 0;
    }
    return scrabled;
}
#endif

#ifdef _MHEG5_SUPPORT_
void mheg5_signal_process(UINT8 lock,UINT8 intensity, UINT8 quality)
{
    struct deca_device * deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    static UINT32 unlock_start_tick = 0;
    static UINT32 need_resume = FALSE;
    static UINT32 mheg_prog_id = 0;

    P_NODE p_node;
    ap_get_playing_pnode(&p_node);
    
    if(lock)
    {
        unlock_start_tick = 0;
        if(need_resume)
        {
            if(p_node.prog_id == mheg_prog_id)
            {
                api_play_channel(p_node.prog_id,TRUE,FALSE,TRUE);
            }
            need_resume = FALSE;            
        }
    }
    else
    {
        if(!unlock_start_tick)
        {
            unlock_start_tick = osal_get_tick();
        }
        if((osal_get_tick() -unlock_start_tick ) > 3000)
        {
            if(mheg_app_avaliable())
            {
                api_disp_blackscreen(TRUE,TRUE);
                deca_io_control(deca_dev, DECA_STR_STOP,0);
                deca_stop(deca_dev,0,ADEC_STOP_IMM);
                mheg_hook_proc(UI_MENU_OPEN,NULL, NULL);                
                mheg_prog_id = p_node.prog_id;
                //mheg_running_app_kill(0);                
                unlock_start_tick = 0;
                need_resume= TRUE;
            }
            else
            {
                unlock_start_tick = 0;
            }
        }
    }
}
#endif

BOOL get_signal_status(signal_lock_status *lock_flag, signal_scramble_status *scramble_flag,
                     signal_lnbshort_status *lnbshort_flag, signal_parentlock_status *parrentlock_flag
#ifdef PARENTAL_SUPPORT
                        ,signal_ratinglock_status *ratinglock_flag
#endif
                        )
{
    UINT8 lock = 0;
    struct dmx_device *dmx_dev=NULL;
    signal_lnbshort_status lnbshort = SIGNAL_STATUS_LNBNORMAL;
    //SYSTEM_DATA *sys_data = sys_data_get();
    UINT8 scrabled = 0;
    P_NODE p_node;
    UINT16 cur_channel = 0;
    struct vdec_status_info cur_status;
    struct nim_device *nim_dev=NULL;
#ifdef NEW_DEMO_FRAME
    INT32 ts_route_id = 0;
    struct ts_route_info ts_route;
#endif    
#if(defined(SUPPORT_C2000ACA))
    static UINT32 unlock_time=0;
#endif 


	if(NULL == dmx_dev)
	{
		;
	}
    MEMSET(&p_node,0,sizeof(P_NODE));
    MEMSET(&cur_status,0,sizeof(struct vdec_status_info));
    nim_dev = (0 == cur_tuner_idx) ? g_nim_dev : g_nim_dev2;
#ifdef NEW_DEMO_FRAME
    MEMSET(&ts_route,0,sizeof(struct ts_route_info));
    if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
    {
        nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id);
    }
    else
    {
        nim_dev = (0 == cur_tuner_idx) ? g_nim_dev : g_nim_dev2;
    }
#endif
    lnbshort = SIGNAL_STATUS_LNBNORMAL;
#if(SYS_LNB_SHORT_DET == SYS_FUNC_ON)
    lnbshort = lnb_power_short;
#elif(SYS_12V_SHORT_DET == SYS_FUNC_ON)
    lnbshort = v12_power_short;
#endif
    if(lnbshort_flag != NULL)
    {
        *lnbshort_flag = lnbshort;
    }
    set_signal_lnbshort_status(lnbshort);
//    if(*lnbshort_flag  == SIGNAL_STATUS_LNBSHORT)  return TRUE;
#ifdef NEW_DEMO_FRAME
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
#else
    dmx_dev = g_dmx_dev;
#endif
#ifdef DVR_PVR_SUPPORT
    if(singal_play_chan_nim_busy)
    {
        lock = SIGNAL_STATUS_UNLOCK;
    }
    else
#endif
    {
        nim_get_lock(nim_dev,&lock);
    }
#ifdef _BUILD_LOADER_COMBO_
	lock=SIGNAL_STATUS_LOCK;//dont check signal lock status in ota
#endif
    if (NULL == parrentlock_flag)
    {
        lv_lock_status = lock? SIGNAL_STATUS_LOCK : SIGNAL_STATUS_UNLOCK;
    }
    else
    {
        set_signal_lock_status( lock? SIGNAL_STATUS_LOCK : SIGNAL_STATUS_UNLOCK);
    }
#ifdef _MHEG5_SUPPORT_
    mheg5_signal_process(lock,0, 0);
#endif
#if(defined(SUPPORT_C2000ACA))
        if(!lock)
        {
            unlock_time++;
        }
        else
        {
            unlock_time=0;
        }
        if(unlock_time>=UNLOCK_TIMES_3)
        {
            ap_set_signal_status(TRUE);
        }
        else
        {
            ap_set_signal_status(FALSE);
        }
#endif
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(cur_channel,&p_node);
#if 0
    scrabled = 0;
    if(dmx_io_control(dmx_dev,IS_AV_SCRAMBLED, (UINT32)(&scrable_typ)) == RET_SUCCESS )
        if(scrable_typ & (VDE_TS_SCRBL|VDE_PES_SCRBL))
            scrabled = 1;
        else
            scrabled = 0;
#else
    scrabled = key_get_dmx0_scramble(NULL);
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    scrabled = get_bc_descramble_state(scrabled, p_node.ca_mode, &cur_status,lock);
#endif
#ifdef CI_SUPPORT2
    if ((NULL == parrentlock_flag) && (p_node.ca_mode) && (!scrabled)&&lock && \
        (SCREEN_BACK_VIDEO == screen_back_state)&&(VIEW_MODE_FULL==hde_get_mode())
#ifdef DVR_PVR_SUPPORT
        && !freeze_for_pvr //black screen result in pvr state trans!
        && api_pvr_is_live_playing()
#endif
        )
        {
            scrabled = get_ci2_descramble_state(scrabled, *p_node, &cur_status)
        }
#endif
    if (parrentlock_flag)
    {
        if (scrabled)
        {
            if (lv_scramble_fake)
            {
                scrabled = 0;
            }
        }
        set_signal_scramble_status(scrabled? SIGNAL_STATUS_SCRAMBLED : SIGNAL_STATUS_UNSCRAMBLED);
    }
    else
    {
        if ((NULL == lock_flag) && (NULL == lnbshort_flag))// special case, just return the CA flag
        {
            scrabled = p_node.ca_mode;
        }
        lv_scramble_status = scrabled? SIGNAL_STATUS_SCRAMBLED : SIGNAL_STATUS_UNSCRAMBLED;
    }
    if((lock) && (!scrabled) && (hde_get_mode()!=VIEW_MODE_MULTI) )/*Never in In multi-view mode*/
    {
        if(p_node.ca_mode && (TV_CHAN == sys_data_get_cur_chan_mode()))
        {
            if(!cur_status.u_first_pic_showed && cur_status.u_cur_status!=VDEC_PAUSED)
            {
                desc_det_time++;
                if(1 == desc_det_time)
                {
                    prechan = cur_channel;
                }
                if((desc_det_time >= DESC_DET_TIME_3) && (prechan == cur_channel))
                {
                    scrabled = 1;
                }
                else
                {
                    desc_det_time = 0;
                }
            }
            else
            {
                desc_det_time = 0;
            }
        }
    }
    if(lock_flag != NULL)
    {
        *lock_flag = lv_lock_status;
    }
    if(scramble_flag != NULL)
    {
        *scramble_flag = lv_scramble_status;
    }
    if(parrentlock_flag != NULL)
    {
        *parrentlock_flag = lv_parrentlock_status;
    }
#ifdef PARENTAL_SUPPORT
    if(ratinglock_flag != NULL)
    {
        *ratinglock_flag = lv_ratinglock_status;
    }
#endif
    if((SIGNAL_STATUS_LOCK == lv_lock_status) && (SIGNAL_STATUS_UNSCRAMBLED == lv_scramble_status)
        && (SIGNAL_STATUS_LNBNORMAL == lv_lnbshort_status) && (SIGNAL_STATUS_PARENT_UNLOCK == lv_parrentlock_status)
#ifdef PARENTAL_SUPPORT
        && (SIGNAL_STATUS_RATING_UNLOCK == lv_ratinglock_status)
#endif
    )
    {
        return FALSE;
    }
    else
    {
    #if 0 //def SAT2IP_CLIENT_SUPPORT // no need to show no singnal when play SAT2IP program.
        if (api_cur_prog_is_sat2ip() && lv_lock_status == SIGNAL_STATUS_UNLOCK )
            return FALSE;
    #endif
        return TRUE;
    }
}

BOOL get_signal_stataus_show(void)
{
    return signal_stataus_showed;
}

void show_signal_status_osdon_off(UINT8 flag)
{
    OBJECT_HEAD *obj=NULL;
    POBJECT_HEAD topmenu =NULL;
    BOOL  b_if_clear = 1;
    struct osdrect relative_rect;

    MEMSET(&relative_rect,0, sizeof(struct osdrect));
 #if(defined(SUPPORT_C2000ACA))
        return;
#endif
#ifdef _INVW_JUICE
    if (inview_is_running())
    {
      return;
    }
#endif
    obj = (OBJECT_HEAD*)&g_win_sigstatus;

#if(defined(SUPPORT_CAS_A))
    if(api_c1700a_osd_get_cas_msg_num()&&flag)
    {

        if(TRUE == signal_stataus_showed)
        {
            osd_clear_object(obj, 0);
            signal_stataus_showed=FALSE;
        }
        return;
    }
#endif

    topmenu =(POBJECT_HEAD)menu_stack_get_top();
    if(topmenu && (topmenu != obj))
    {
        relative_rect.u_width = 0;
        relative_rect.u_height = 0;
        osd_get_rects_cross(&(topmenu->frame),&(obj->frame),&relative_rect);
        if((relative_rect.u_height != 0) && (relative_rect.u_width != 0))
        {
            b_if_clear = 0;
        }
    }

    #ifndef _BUILD_OTA_E_
    //rec_poplist_display  has been linked to top window in some case.So there
    // are more than one windows are showing on the screen at the same time.
   if((b_if_clear) &&  (is_rec_poplist_displaying()))
   {
//    extern CONTAINER    g_win_pvr_recpoplist;
        topmenu = (POBJECT_HEAD)&g_win_pvr_recpoplist;
        relative_rect.u_width = 0;
        relative_rect.u_height = 0;
        osd_get_rects_cross(&(topmenu->frame),&(obj->frame),&relative_rect);
        if((relative_rect.u_height != 0) && (relative_rect.u_width != 0))
        {
            b_if_clear = 0;
        }
   }
   #endif

    if(flag)
    {
        osd_draw_object(obj, C_UPDATE_ALL);

    }
    else
        //osd_clear_object(obj, 0);
    {
        b_if_clear ? osd_clear_object(obj, 0):NULL;
    }
    signal_stataus_showed = flag ? TRUE : FALSE;
}

void show_signal_status_enable(void)
{
    show_signal_status_enabled = TRUE;
}

void show_signal_status_disable(void)
{
    show_signal_status_enabled = FALSE;
}

//void conax_remove_mutex();
void show_signal_status_on_off(void)
{
    UINT8 flag = 0;
    signal_lock_status lock_flag = SIGNAL_STATUS_LOCK;
    signal_scramble_status scramble_flag = SIGNAL_STATUS_UNSCRAMBLED;
    signal_lnbshort_status lnbshort_flag = SIGNAL_STATUS_LNBNORMAL;
    signal_parentlock_status parrentlock_flag = SIGNAL_STATUS_PARENT_UNLOCK;
    __MAYBE_UNUSED__ struct smc_device *smc_dev = NULL;

#ifdef SUPPORT_C0200A
    if (FALSE == show_signal_status_enabled)
    {
        return ;
    }
#endif

#ifdef CI_PLUS_SUPPORT
    if(is_ciplus_menu_exist())
        return;
#endif

#ifdef _MHEG5_SUPPORT_
    if(mheg_app_avaliable())
    {
        return;
    }
#endif

#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag = SIGNAL_STATUS_RATING_UNLOCK;
    if(FALSE == get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag))
#else
    if(FALSE == get_signal_status(&lock_flag,&scramble_flag,&lnbshort_flag,&parrentlock_flag))
#endif
    {
        flag = 0;
        is_no_signal_status= FALSE;
    }
    else
    {
        flag = 1;
        is_no_signal_status= TRUE;
    }

#ifdef SUPPORT_CAS9
    smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
    if((SUCCESS == smc_card_exist(smc_dev)) && flag  && (STRING_ID_SIGNAL_SCRAMBLED == sigstatus_txt.w_string_id))
    {        
        return;
    }
    else if((SUCCESS != smc_card_exist(smc_dev)) && flag  && (STRING_ID_SIGNAL_SCRAMBLED == sigstatus_txt.w_string_id))
    {    
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_NON_EXIST<<16, FALSE);    
        return;
    }
#endif
#ifdef SUPPORT_C0200A
	if ((flag == 1) && (scramble_flag == SIGNAL_STATUS_SCRAMBLED))
	{
		return;
	}
#endif
	if(lock_flag == SIGNAL_STATUS_LOCK)
		flag = 0;
	else
		flag = 1;	
    show_signal_status_osdon_off(flag);
}

#if(SYS_LNB_SHORT_DET == SYS_FUNC_ON)
static void lnb_power_short_detect(UINT32 nouse)
{
    SYSTEM_DATA *sys_data = NULL;
    static UINT32 count = 0;
    UINT32 lnb_short = 0;
    static UINT8 lnb_short_cut =0 ;
    static UINT8 det_start = 0;

    if(!lnb_power_detect_start && !det_start)
    {
        return;
    }
    det_start = 1;

    sys_data = sys_data_get();

    if(LNB_POWER_ON == sys_data->b_lnb_power)
    {
        if(!lnb_short_cut)  /* If not cut it forcedly*/
        {
            lnb_short = board_lnb_state(0);
            if(0 == lnb_short)
            {
                lnb_short = board_lnb_state(1);
            }
            if(1 == lnb_short)  /* LNB short detected*/
            {
                /* Cut LNB Power*/
                board_lnb_power(0, 0);
                board_lnb_power(1, 0);

                /* Set corresponding flags*/
                lnb_power_short = SIGNAL_STATUS_LNBSHORT;
                lnb_short_cut = 1;
                count = 0;

                /* Set LED display */
                led_display_flag = 1;
                key_pan_display("LnbP",4);
                led_display_flag = 2;
            }
            else
            {
                if((SIGNAL_STATUS_LNBSHORT == lnb_power_short) || (led_display_flag != 0))
                {
                    /* Restore LED display*/
                    led_display_flag = 1;
                    key_pan_display(led_display_data,led_data_len);
                    led_display_flag = 0;
                    lnb_power_short = SIGNAL_STATUS_LNBNORMAL;
                }

            }
        }
        else    /* If have already cut it forcedly*/
        {
            count++;
            if(count >= LNB_POWER_RESET_WAIT_TIME)
            {
                /* Restore LNB Power*/
                api_lnb_power(sys_data->b_lnb_power);
                lnb_short_cut = 0;
                count = 0;
            }
        }
    }
    else    /* User set LNB Power off */
    {
        if((SIGNAL_STATUS_LNBSHORT == lnb_power_short) || (led_display_flag != 0))
        {
            /* Restore LED display*/
            led_display_flag = 1;
            key_pan_display(led_display_data,led_data_len);
            led_display_flag = 0;
            lnb_power_short = SIGNAL_STATUS_LNBNORMAL;
        }
        lnb_short_cut = 0;
        count = 0;
    }
}
#endif

#ifdef SYS_12V_SWITCH
#if(SYS_12V_SWITCH == SYS_FUNC_ON)
    volatile int v12_detect = 0;
#endif
#endif

#ifdef SYS_12V_SWITCH
#if(SYS_12V_SWITCH == SYS_FUNC_ON)
static void v12_set_callback(UINT32 v12)
{
    v12_detect = v12;
}
#endif
#endif

#if(defined(SYS_12V_SWITCH) && defined(SYS_12V_SHORT_DET))

#if(SYS_12V_SWITCH==SYS_FUNC_ON && SYS_12V_SHORT_DET == SYS_FUNC_ON)
static void v12_power_short_detect(UINT32 nouse)
{
    INT32 v12 = 0;
    INT32 v12_short = 0;
    static INT32 oldv12 = -1;
    static UINT8 v12_short_cut =0 ;
    static UINT32 count = 0;

    if(!v12_short_cut)
    {
        if(!v12_detect)
        {
            return;
        }
        HAL_GPIO_BIT_DIR_SET(SYS_12V_SHORT_DET_GPIO, HAL_GPIO_I_DIR);
        v12_short = HAL_GPIO_BIT_GET(SYS_12V_SHORT_DET_GPIO);

        v12_short = v12_short? 0 : 1;   /* Polarity reverse */

        if(1 == v12_short)  /* V12 short detected*/
        {
            /* Cut V12 Power*/
            HAL_GPIO_BIT_DIR_SET(SYS_12V_GPIO, HAL_GPIO_O_DIR);
            HAL_GPIO_BIT_SET(SYS_12V_GPIO,0);

            /* Set corresponding flags*/
            v12_power_short = SIGNAL_STATUS_LNBSHORT;
            v12_short_cut = 1;
            count = 0;

            /* Set LED display */
            led_display_flag = 1;
            key_pan_display("12SP",4);
            led_display_flag = 2;
        }
        else
        {
RESET_V12_STATUS:
            if((SIGNAL_STATUS_LNBSHORT == v12_power_short) || (led_display_flag != 0))
            {
                /* Restore LED display*/
                led_display_flag = 1;
                key_pan_display(led_display_data,led_data_len);
                led_display_flag = 0;
                v12_power_short = SIGNAL_STATUS_LNBNORMAL;
            }

        }
    }
    else
    {
        if(!v12_detect)/* User have re-set V12 */
        {
            v12_short_cut = 0;
            goto RESET_V12_STATUS;
        }
        count++;
        if(count >= V12_POWER_RESET_WAIT_TIME)
        {
            /* Restore LNB Power*/
            HAL_GPIO_BIT_DIR_SET(SYS_12V_GPIO, HAL_GPIO_O_DIR);
            HAL_GPIO_BIT_SET(SYS_12V_GPIO,v12_detect);
            v12_short_cut = 0;
            count = 0;
        }
    }
}
#endif
#endif

void set_lnbshort_detect(UINT32 flag)
{

#if(SYS_LNB_SHORT_DET == SYS_FUNC_ON)
    static UINT8 set = 0;

    if(!set && flag)
    {
        osal_interrupt_register_lsr(7, lnb_power_short_detect, 0);
        set = 1;
    }
    else if(set && !flag)
    {
        osal_interrupt_unregister_lsr(7, lnb_power_short_detect);
        lnb_power_short = SIGNAL_STATUS_LNBNORMAL;
        led_display_flag = 0;
        set = 0;
    }
#endif
}

void set_lnbshort_detect_start(UINT32 flag)
{
    lnb_power_detect_start = (UINT8)flag;
}

void get_signal_status_rect(OSD_RECT *rect)
{
    POBJECT_HEAD obj = NULL;

    if(NULL == rect)
    {
        return ;
    }
    obj = (POBJECT_HEAD)&g_win_sigstatus;
    *rect = obj->frame;
}

void set_channel_nim_busy(UINT32 flag)
{
#ifdef DVR_PVR_SUPPORT
    singal_play_chan_nim_busy = (UINT8)flag;
#endif
}


void set12vshort_detect(UINT32 flag)
{
#if(defined(SYS_12V_SWITCH) && defined(SYS_12V_SHORT_DET))

#if(SYS_12V_SWITCH==SYS_FUNC_ON && SYS_12V_SHORT_DET == SYS_FUNC_ON)
    static UINT8 set = 0;

    uich_chg_set12vset_cb(v12_set_callback);

    if(!set && flag)
    {
        osal_interrupt_register_lsr(7, v12_power_short_detect, 0);
        set = 1;
    }
    else if(set && !flag)
    {
        osal_interrupt_unregister_lsr(7, v12_power_short_detect);
        v12_power_short = SIGNAL_STATUS_LNBNORMAL;
        led_display_flag = 0;
        set = 0;
    }
#endif
#endif
}

void shift_signal_osd(void)
{
    CONTAINER *signal_con = &g_win_sigstatus;
    TEXT_FIELD *signal_txt = &sigstatus_txt;
    BITMAP *signal_bmp = &sigstatus_bmp1;

    signal_con->head.frame.u_left = (signal_con->head.frame.u_left)+POP_OFFSET_L;
    signal_txt->head.frame.u_left = (signal_txt->head.frame.u_left)+POP_OFFSET_L;
    signal_bmp->head.frame.u_left = (signal_bmp->head.frame.u_left)+POP_OFFSET_L;
}
void restore_signal_osd(void)
{
    CONTAINER *signal_con = &g_win_sigstatus;
    TEXT_FIELD *signal_txt = &sigstatus_txt;
    BITMAP *signal_bmp = &sigstatus_bmp1;

    signal_con->head.frame.u_left = (signal_con->head.frame.u_left)-POP_OFFSET_L;
    signal_txt->head.frame.u_left = (signal_txt->head.frame.u_left)-POP_OFFSET_L;
    signal_bmp->head.frame.u_left = (signal_bmp->head.frame.u_left)-POP_OFFSET_L;
}

void set_signal_scramble_fake(UINT8 b_fake)
{
    lv_scramble_fake = b_fake;
}

UINT8 get_signal_scramble_fake(void)
{
    return 0;
    //return lv_scramble_fake;
}

BOOL api_is_stream_scrambled(void)
{
    UINT8 scrabled = 0;
    P_NODE playing_pnode;
    //BOOL   bret = FALSE;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);

    if(playing_pnode.prog_id != 0)
    {
#ifdef _INVW_JUICE
#else
        scrabled = ts_route_is_av_scrambled(g_dmx_dev, playing_pnode.video_pid, \
                                            playing_pnode.audio_pid[playing_pnode.cur_audio]);
#endif
    }

    return scrabled;
}

