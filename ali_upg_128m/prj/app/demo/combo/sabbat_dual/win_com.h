/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com.h
*
*    Description: The common function of UI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COM_H_
#define _WIN_COM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _INVW_JUICE
#include <sys_config.h>
#endif
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
//modify bomur 2011 09 19
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_as.h>
#include <api/libpub29/lib_nim_manage.h>
//modify end
#else
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_pub.h>
//modify bomur 2011 09 19
#include <bus/tsi/tsi.h>
#include <api/libpub/lib_as.h>
#include <api/libpub/lib_hde.h>
//modify end
#include <api/libpub/lib_frontend.h>
#endif
#include <api/libosd/osd_lib.h>
#include <api/libsi/sie_monitor.h>

#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/menu_api.h"
#include "copper_common/com_epg.h"
#include "copper_common/system_data.h"
#include "pvr_control.h"
#include "vkey.h"

#include "win_submenu.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include <bus/dog/dog.h>

#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
#ifdef SD_UI
//win_light_chan_list.c
#define W_LCL_L 14
#define W_LCL_T 20
#define W_LCL_W 255
#define W_LCL_H 425
#endif
#endif

#define GET_MID_L(w)    ((OSD_MAX_WIDTH-(w))/2)
#define GET_MID_T(h)    ((OSD_MAX_HEIGHT-(h))/2)


/* Menu Open type & parameter mask */
#define MENU_OPEN_TYPE_MASK     0xFF000000
#define MENU_OPEN_PARAM_MASK    0x000000FF

#define MENU_OPEN_TYPE_KEY      0xFF000000      /* PARAM : vkey         */
#define MENU_OPEN_TYPE_MENU     0xFE000000      /* PARAM : menu item ID */
#define MENU_OPEN_TYPE_STACK    0xFD000000      /* PARAM : null */
#define MENU_OPEN_TYPE_OTHER    0xFC000000      /* PARAM : null */

#define PROG_ID_NOTFOUND (-4)/**/		//by Dean
#define MENU_FOR_ISDBT          0x0000ff00

#define INVALID_SIM_ID (UINT32)(~0)

#define NON_ACTION_LENGTH     16

#define PERCENT_MIN           0
#define PERCENT_MAX           100
#define LENGTH_O_HEX          16
#define LENGTH_O_HEX_HALF     8
#define LENGTH_O_ECIMAL       10
#define HOURS_IN_ONEDAY       24
#define MINUTES_IN_ONEHOUR    60
#define SECONDS_IN_ONEMINUTE  60

extern UINT8 cur_tuner_idx;             // current tuner index
extern UINT8 g_tuner_num;               // total tuner number
extern UINT16 tp_polarity_ids[];        // transponder polarity id
extern UINT16 symb_sub[10];             // symbol rate
extern UINT16 fre_sub[10];              // frequency
#ifdef  _BUILD_OTA_E_
extern UINT16 lnb_sub[10];              // frequency
#endif
extern char symb_pat[];                 // symbol rate pat
extern char fre_pat[];                  // frequency pat
#ifdef  _BUILD_OTA_E_
extern char lnb_pat[];                  // frequency pat
#endif

extern TEXT_FIELD menu_title_txt;       // menu title text

#ifdef SEARCH_DEFAULT_FTAONLY
#define FATONLY_DEFAULT SEARCH_DEFAULT_FTAONLY
#else
#define FATONLY_DEFAULT 1
#endif

typedef struct win_scanch_channel_info_tag
{
    UINT32  vhf_current_ch_no;
    UINT32  vhf_start_ch_no;
    UINT32  vhf_end_ch_no;
    UINT32  uhf_start_ch_no;
    UINT32  uhf_end_ch_no;
    UINT32  uhf_current_ch_no;
}win_scanch_channel_info_t;


struct help_item_resource
{
    UINT16  bmp_id;
    UINT16  str_id;
};
struct help2_item_resource
{
	UINT8 part1_type; //0:txt 1:bmp
	UINT16 part1_id;
	UINT16 part2_id;
};

void wincom_open_title(POBJECT_HEAD rootwin,UINT16 title_strid, UINT16 title_bmpid);
void wincom_close_title(void);

void wincom_open_help(POBJECT_HEAD rootwin,struct help_item_resource *helpinfo, UINT8 item_count);
void wincom_close_help(void);

/* For tuner index : see TUNER_TYPE_T*/
UINT16 get_tuner_strid(UINT32 tuner_idx);
UINT32 get_tuner_cnt(void);
UINT16 get_tuner_sat_cnt(UINT16 tuner_idx);
void   get_tuner_sat(UINT32 tuner_idx, UINT32 sat_idx, S_NODE* snode);
void   get_tuner_sat_name(UINT32 tuner_idx, UINT32 sat_idx, UINT16* unistr);

void   get_tp_name(UINT32 sat_id, UINT32 tp_pos,UINT16* unistr);
UINT32 get_lnb_type_cnt(void);
UINT32 get_lnb_type_index(UINT32 lnb_type, UINT32 lnb_freq1, UINT32 lnb_freq2);
void   get_lnb_type_setting(UINT32 lnb_idx,UINT32* lnb_type, UINT32* lnb_freq1, UINT32* lnb_freq2);
void   get_lnb_name(UINT32 lnb_idx,UINT16* unistr);

UINT32 get_diseqc10_cnt(void);
void   get_diseqc10_name(UINT32 index,char* str, UINT16* unistr,UINT16* str_id);

UINT32 get_diseqc11_cnt(void);
void   get_diseqc11_name(UINT32 val, UINT16* unistr);
UINT32 diseqc11_db_to_ui(UINT32 diseqc11_type, UINT32 diseqc11_port);
void   diseqc11_ui_to_db(UINT32 val, UINT8 *type, UINT8 *port);

UINT16 get_unicable_freq(UINT8 index);
UINT32 get_unicable_freq_index(UINT16 freq);
UINT16 get_if_channel_cnt(void);
UINT16 get_centre_freqs_cnt(void);
void get_if_channel_name(UINT32 channel_idx,UINT16* unistr);
void get_centre_freqs_name(UINT32 freq_idx,UINT16* unistr);

void  get_chan_group_name(char* group_name, int grp_name_size, UINT8 group_idx,UINT8* group_type);
void  get_cur_group_name(char* group_name,int grp_name_size, UINT8* group_type);

#ifndef NEW_DEMO_FRAME
void sat2antenna_ext(S_NODE *sat, struct cc_antenna_info *antenna, UINT32 tuner_idx);
#endif

void remove_menu_item(CONTAINER* root,OBJECT_HEAD* item,UINT32 height);
UINT16 win_load_default_satidx();

void  get_dvbc_freq_str(UINT32 freq, void* str, BOOL uni);

//modify bomur 2011 09 19
void increase_val_inuncode(UINT16 *ptr_base,UINT32 pos);
void decrease_val_inuncode(UINT16 *ptr_base,UINT32 pos);
//modify end

////////////////////////////////////////////////////////
// sub list (SAT / TP / LNB / DisEqC1.0 )

#ifdef PARENTAL_SUPPORT
BOOL rating_check(UINT16 cur_channel, BOOL all_check);
void uiset_rating_lock(BOOL lock);
void clear_pre_ratinglock(void);
#endif

typedef enum
{
    POP_LIST_TYPE_SAT,      // param :  id - > SAT_POP_LIST_PARAM_T,
                            // cur -> current selection or focus
    POP_LIST_TYPE_TP,       // param :  id -> S_NODE's ID
    POP_LIST_TYPE_LNB,      // param:  id -> S_NODE's ID,
    POP_LIST_TYPE_DISEQC10,//NULL
    POP_LIST_TYPE_DISEQC11,//NULL
    POP_LIST_TYPE_CHGRPALL,
    POP_LIST_TYPE_CHGRPFAV,
    POP_LIST_TYPE_CHGRPSAT,
    POP_LIST_TYPE_FAVSET,
    POP_LIST_TYPE_SORTSET,
    POP_LIST_TYPE_RECORDLIST,
#ifdef MULTI_PARTITION_SUPPORT
    POP_LIST_TYPE_VOLUMELIST,
#endif
    POP_LIST_TYPE_CHANLIST,
    POP_LIST_MENU_LANGUAGE_OSD,
    POP_LIST_MENU_LANGUAGE_STREAM,
    POP_LIST_MENU_TVSYS,
    POP_LIST_MENU_OSDSET,
    POP_LIST_MENU_TIME,
    POP_LIST_MENU_KEYLIST,
    POP_LIST_TYPE_IF_CHANNEL,
    POP_LIST_TYPE_CENTRE_FREQ,
    POP_LIST_TYPE_MPSORTSET,
    POP_LIST_TYPE_VIDEOSORTSET,
#ifdef DISPLAY_SETTING_SUPPORT
    POP_LIST_MENU_DISPLAY_SET,
#endif
    POP_LIST_DISK_MODE,
    POP_LIST_DVR_TYPE,
    POP_LIST_PARTITION,
#ifdef DISK_MANAGER_SUPPORT
    POP_LIST_REMOVABLE_DISK_INFO,
#endif

//#ifdef SHOW_WELCOME_SCREEN
    POP_LIST_MENU_COUNTRY_NETWORK,
//#endif
    POP_LIST_MENU_PARENTAL_MENU_LOCK,
    POP_LIST_MENU_PARENTAL_CHANNEL_LOCK,
    POP_LIST_MENU_PARENTAL_CONTENT_LOCK,
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    POP_LIST_SUBT_SET,
#endif
#ifdef IMG_2D_TO_3D
    POP_LIST_IMAGE_SET,
#endif
#ifdef MULTIVIEW_SUPPORT
    POP_LIST_MULTIVIEW_SET,
#endif
#ifdef SAT2IP_SERVER_SUPPORT
    POP_LIST_MENU_SATIP_SLOT_INSTALL,
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
    POP_LIST_MENU_SATIP_SERVER,
#endif
#ifdef NETWORK_SUPPORT
    POP_LIST_MENU_NET_CHOOSE,
#endif
#ifdef TKGS_SUPPORT
    POP_LIST_TKGS_PREFER_SET,
#endif
#ifdef BC_TEST
	POP_LIST_MENU_VMX_TEST,
#endif
    POP_LIST_TYPE_INVALID,

}COM_POP_LIST_TYPE_T;

typedef enum
{
    TUNER_FIRST = 0,
    TUNER_SECOND,
    TUNER_EITHER,   //
    TUNER_EITHER_SELECT,
}TUNER_TYPE_T;

typedef enum
{
    POP_LIST_NOSELECT = 0,
    POP_LIST_SINGLESELECT,
    POP_LIST_MULTISELECT
}COM_POP_LIST_SELECT_TYPE_T;

typedef struct
{
    UINT32 id;              /* Tuner for satlist / sat ID for tplist */
    UINT32 cur;             /* Current selection / focus */
    UINT32 selecttype;      /* COM_POP_LIST_SELECT_TYPE_T for satlist only */
    UINT8 *select_array;
}COM_POP_LIST_PARAM_T;

UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type,OSD_RECT* p_rect, COM_POP_LIST_PARAM_T* param);

#define HELP_TYPE_INFOR     1 // only exit
#define HELP_TYPE_FACTORY   2 // H, 3icon
#define HELP_TYPE_RS232     0
#define HELP_TYPE_OTA       0 // 4 icon
#define HELP_TYPE_OTHER     3 // no OK, 3Icon
#define HELP_TYPE_FAVGROUP  4 // V, 3Icon
#define HELP_TYPE_OSDSET    0 // 4 icon
#define HELP_TYPE_PARENT_LOCK       3 // no OK, 3Icon
#define HELP_TYPE_UPGRAGE   5 // 2icon exit, ok
#define HELP_TYPE_TPLIST    6 // 2icon exit, ok
#define HELP_TYPE_SATLIST   7 // 2icon exit, ok
#define HELP_TYPE_PREVIEW   8 // 4 icon
#define HELP_TYPE_EPG       9
#define HELP_TYPE_MEDIA     10
#define HELP_TYPE_PVRLIST   11

#ifdef NETWORK_SUPPORT
typedef UINT32 (*lib_net_ui_callback)(UINT32 type, UINT32 code);
typedef UINT32 (*lib_net_user_stop)();

typedef struct
{
    IP_REMOTE_CFG rmt_cfg;  // remote config
    IP_LOC_CFG local_cfg;   // local config
    lib_net_ui_callback pfn_ui_connect; // indicator msg to ap
    lib_net_user_stop pfn_user_stop;    // check user cancel net upgrade.
    UINT32 file_addr;                   // get the upgrade file address
    UINT32 file_len;
}LIB_NUC, *PLIB_NUC; // lib net upgrade config
#endif

UINT16 audio_pid2type(UINT16 audio_pid);

extern const char *qam_table[];                  // Qam tables
extern UINT16 yesno_ids[];

void wincom_open_title_ext(POBJECT_HEAD rootwin,UINT8* title_str,UINT16 title_bmpid);

extern UINT8 antset_cur_tuner;      // current tuner

void win_comlist_set_mapcallback_ex(PFN_KEY_MAP list_keymap,
                    PFN_KEY_MAP win_keymap, PFN_CALLBACK lstcallback, PFN_CALLBACK wincallback);
void win_comlst_set_style(POBJECT_HEAD pobj,UINT32 shidx,UINT32 hlidx, UINT32 slidx,UINT32 gyidx);
void win_compopup_refresh(void);
BOOL win_msg_popup_opend(void);
void win_compopup_set_btnstr_ext(UINT32 btn_idx, char *str);
win_popup_choice_t win_compopup_open_enh(POBJECT_HEAD root_obj, UINT32 draw_mode, UINT32 cmd);

void wincom_dish_move_popup_open(UINT16 pre_sat_id,UINT16 cur_sat_id, UINT32 tuner_idx,UINT8 *back_saved);

void win_compopup_set_timer(UINT32 time, UINT32 msgtype, UINT16 timeout);

//void send_key_timer_handler(UINT32 vkey);
ID win_compopup_start_send_key_timer(UINT32 vkey, UINT32 interval, UINT16 timer_type);
//win_popup_choice_t win_compopup_open_cas(void);
UINT8 get_cas_pop_status(void);
BOOL wincom_is_cnx_title(void);
UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type,OSD_RECT *p_rect, COM_POP_LIST_PARAM_T *param);
UINT8 get_universal_lnb_index(void);
BOOL wincom_reset_nim(void);
void set_container_active(CONTAINER *con,UINT8 action);
void remove_menu_item_conwithout_focus(CONTAINER *root,OBJECT_HEAD *item,UINT32 height);
void set_passwd_status(BOOL flag);// 1 for pass, 0 not pass
BOOL get_passwd_status(void);
void set_pvr_rating_pwd_status(BOOL flag);// 1 for pass, 0 not pass
BOOL get_pvr_rating_pwd_status(void);
void set_chan_lock_status(BOOL flag);// 1 for pass, 0 not pass
BOOL get_chan_lock_status(void);
void set_current_stream_rating(UINT8 rating);
UINT8 get_current_stream_rating(void);
UINT8  find_parent_age_setting(UINT8 select_id);
BOOL check_rating_lock(UINT8 stream_rating);
BOOL check_content_lock(UINT8 stream_rating);
BOOL check_epg_rating(UINT8 stream_rating);

UINT8 com_str_cat(UINT8  *target, const UINT8 *source);
void com_int2str(UINT8 *lp_str, UINT32 dw_value, UINT8 b_len);
UINT32 wincom_asc_to_mbuni(UINT8 *ascii_str, UINT16 *uni_str);
void wincom_i_to_mbs(UINT16  *str, UINT32 num, UINT32 len);
UINT32 wincom_mbs_to_i(UINT16  *str);
void wincom_i_to_mbs_with_dot(UINT16  *str, UINT32 num, UINT32 len/*without dot*/, UINT8 dot_pos);
void wincom_mbs_to_i_with_dot(UINT16  *str, UINT32  *num, UINT8  *dot_pos);
void dvbc_freq_to_str( UINT32 freq, char *str, UINT32 str_size);

#ifdef START_RECORD_SHOW_MESSAGE
INT32 record_get_notice_msg(UINT32 prog_id,  record_notice_type notice_type, UINT16 *unimsg);
#endif

void freq_to_str( UINT32 freq, char *str);
void get_freq_str( UINT32 freq, void *str, int str_size, BOOL uni);

BOOL wincom_backup_region(POSD_RECT frame);
void wincom_restore_region(void);
#ifdef SAT2IP_SERVER_SUPPORT
PRESULT comlist_menu_serv_slot_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
UINT16 win_satip_serv_get_sat_num(void);
#endif

#ifdef __cplusplus
}
#endif

#endif//_WIN_COM_H_
