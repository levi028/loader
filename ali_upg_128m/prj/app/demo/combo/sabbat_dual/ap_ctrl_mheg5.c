#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libsi/sie_monitor.h>
#include <bus/tsi/tsi.h>
#include "copper_common/com_api.h"
#include "ap_ctrl_mheg5.h"
#include "control.h"
#include "ap_ctrl_display.h"
#include "menus_root.h"
#include "win_prog_name.h"

#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_   //MHEG5 enable, reserve 8MB

// CI+ MHEG5 browser
#include <mh5_api/mh5_api.h>
#include <ssf/ssf_api.h>    //Generic_Browser debugging
#endif
#endif
#ifdef CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_   //MHEG5 enable, reserve 8MB
static UINT32               mheg_pat_monitor_id = SIM_INVALID_MNI_ID;
static UINT32               mheg_pmt_monitor_id = SIM_INVALID_MNI_ID;
static BOOL                 b_subt_status = FALSE;
static BOOL                 b_ciplus_menu_exist = FALSE;
UINT8                       g_mheg5_app_domain_identifier[256];
UINT8                       g_mheg_app_domain_ident_len = 0;
UINT8                       g_initial_object[256];
UINT8                       g_ci_plus_file[0x10000];
int                         g_index = 0;
static app_init_object_t    app_init_obj_info;
void                        ciplus_browser_exit(void);
static void mheg_receive_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct sim_cb_param *cb_param = (struct sim_cb_param *)param;
    if (section != NULL)
    {
        filter_callback(cb_param->sec_pid, section, length, PAT);
    }
}

static INT32 mheg_receive_pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct sim_cb_param *cb_param = (struct sim_cb_param *)param;
    filter_callback(cb_param->sec_pid, section, length, PMT);
}

INT32 mheg_monitor_start(void)
{
    struct dmx_device   *dmx;
    P_NODE              p_node;
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    UINT16  prog_pos = sys_data_get_cur_group_cur_mode_channel();
    if (get_prog_at((UINT16) prog_pos, &p_node) != SUCCESS)
    {
        //        libc_printf("%s :  program not find\n", __FUNCTION__);
    }

    mheg_pat_monitor_id = sim_start_monitor(dmx, MONITE_TB_PAT, PSI_PAT_PID, 0);
    mheg_pmt_monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, p_node.pmt_pid, p_node.prog_number);
    if ((SIM_INVALID_MNI_ID == mheg_pat_monitor_id) || (SIM_INVALID_MNI_ID == mheg_pmt_monitor_id))
    {
        //        libc_printf("%s fail\n", __FUNCTION__);
        return !SUCCESS;
    }

    sim_register_scb(mheg_pat_monitor_id, mheg_receive_pat_callback, NULL);
    sim_register_scb(mheg_pmt_monitor_id, mheg_receive_pmt_callback, NULL);
    return SUCCESS;
}

void mheg_monitor_stop(void)
{
    if (mheg_pat_monitor_id != SIM_INVALID_MNI_ID)
    {
        sim_stop_monitor(mheg_pat_monitor_id);
        mheg_pat_monitor_id = SIM_INVALID_MNI_ID;
    }

    if (mheg_pmt_monitor_id != SIM_INVALID_MNI_ID)
    {
        sim_stop_monitor(mheg_pmt_monitor_id);
        mheg_pmt_monitor_id = SIM_INVALID_MNI_ID;
    }
}

static void api_mheg5_callback(mheg5callback_msg parameter, UINT32 code)
{
    set_menu_exit_from_ci_browser(FALSE);

    POBJECT_HEAD    first_menu = NULL;
    UINT16          prog_num = get_prog_num(VIEW_ALL | sys_data_get_cur_chan_mode(), 0);
    if (MHEG5_ENG_EXIT == parameter)
    {
        if (prog_num == 0)
        {
            //#ifdef SHOW_WELCOME_SCREEN
            //first_menu = (POBJECT_HEAD)&win_lan_con;/* Main menu */
            //set_win_lang_as_welcome(TRUE);
#ifdef SHOW_WELCOME_FIRST
            first_menu = ((POBJECT_HEAD) & g_win_welcom);
#else
            first_menu = MAIN_MENU_HANDLE;      /* Main menu */
#endif
        }
        else
        {
            show_and_playchannel = 1;

            //set_isciplus_menu_exist(FALSE);
            first_menu = CHANNEL_BAR_HANDLE;    /* Channel bar menu */
        }

        if (first_menu != NULL)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) first_menu, TRUE);
        }
    }
}

BOOL is_ciplus_menu_exist(void)
{
    return b_ciplus_menu_exist;
}

static void set_isciplus_menu_exist(BOOL exist)
{
    b_ciplus_menu_exist = exist;
}

void ciplus_browser_enter(void)
{
    ciplus_browser_exit();
    mheg5_set_spin(SPIN_MHEG_MENU);

    UINT16  prog_pos = sys_data_get_cur_group_cur_mode_channel();
    mheg5_receiver_start_applications((prog_pos > 0xFFF0) ? (UINT32) si_get_cur_channel() : prog_pos);
#if (SUBTITLE_ON == 1)
    b_subt_status = api_get_subt_onoff();
    subt_enable(FALSE);
    api_subt_show_onoff(FALSE);
#endif
    mheg5set_callback(api_mheg5_callback);
    ap_clear_all_menus();
    win_compopup_close();
    win_msg_popup_close();
    mheg5_osdenter();
    osddrv_show_on_off((HANDLE) dev_get_by_id(HLD_DEV_TYPE_OSD, 1), OSDDRV_ON);
    set_isciplus_menu_exist(TRUE);
}

void ciplus_browser_exit(void)
{
    if (is_ciplus_menu_exist())
    {
        mheg5_clr_spin(SPIN_MHEG_MENU);
        mheg5_receiver_stop_applications();
        mheg5set_callback(NULL);
        subt_enable(TRUE);
        api_subt_show_onoff(b_subt_status);
        osddrv_show_on_off((HANDLE) dev_get_by_id(HLD_DEV_TYPE_OSD, 1), OSDDRV_OFF);
        mheg5_osdexit();
        set_menu_exit_from_ci_browser(FALSE);   //terminor CI browser app.
    }

    set_isciplus_menu_exist(FALSE);
}

void ciplus_reenter_browser(void)
{
    osddrv_show_on_off((HANDLE) dev_get_by_id(HLD_DEV_TYPE_OSD, 1), OSDDRV_ON);
}
#endif
#endif
