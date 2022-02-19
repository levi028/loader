#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#include <api/libdb/db_node_combo.h>
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>
#include <hld/sdec/sdec_dev.h>
#include "./copper_common/com_api.h"


#if(defined _BUILD_OTA_E_)//add by comer
INT32 db_search_lookup_node(UINT8 n_type, void *node)
{
    return DB_SUCCES;
}

INT32 add_node(UINT8 n_type, UINT32 parent_id,void *node)
{
    return DB_SUCCES;
}

INT32 get_prog_by_id(UINT32 pg_id,  P_NODE *node)
{
    return DB_SUCCES;
}

void get_audio_lang3b(UINT8 *src,UINT8 *des)
{
}

INT32 get_sat_by_id(UINT16 sat_id,  S_NODE *node)
{
    return DB_SUCCES;
}

INT32 modify_prog(UINT32 pg_id,  P_NODE *node)
{
    return DB_SUCCES;
}

UINT16 get_prog_num(UINT16 filter_mode,UINT32 param)
{
    return 0;
}

INT32 db_lcn_init()
{
    return DB_SUCCES;
}

INT32 init_db(UINT32 db_base_addr, UINT32 db_length, UINT32 tmp_info_base, UINT32 tmp_info_len)
{
    return DB_SUCCES;
}

INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr)
{
    return DB_SUCCES;
}

UINT16 get_sat_num(UINT16 select_mode)
{
    return 0;
}

INT32 recreate_prog_view(UINT16 create_flag, UINT32 param)
{
    return DB_SUCCES;
}

UINT16 get_prog_pos(UINT32 pg_id)
{
    return 0;
}

INT32 get_prog_at(UINT16 pos,  P_NODE* node)
{
    return 0;
}

INT32 get_sat_at(UINT16 pos, UINT16 select_mode, S_NODE *node)
{
    return 0;
}

int erom_rw( unsigned long mem_addr, void *buf,unsigned long len,\
         unsigned int block_mode, unsigned int is_read)
{
    return 0;
}

int erom_read_tmo( unsigned long mem_addr, void *buf,unsigned long len,\
         unsigned int block_mode, unsigned long tmo_us)
{
    return 0;
}

INT32 get_tp_by_id(DB_TP_ID tp_id,  T_NODE *node)
{
    return 0;
}

INT32 modify_tp(DB_TP_ID tp_id,  T_NODE *node)
{
    return 0;
}

INT32 update_data( )
{
    return 0;
}

INT32 db_search_create_pg_view(UINT8 parent_type, UINT32 parent_id, UINT8 prog_mode)
{
    return 0;
}

INT32 db_search_create_tp_view(UINT16 parent_id)
{
    return 0;
}

UINT16 get_tp_num_sat(UINT16 sat_id)
{
    return 0;
}

INT32 get_tp_at(UINT16 sat_id,UINT16 pos,  T_NODE *node)
{
    return 0;
}

void get_audio_lang2b(UINT8 *src,UINT8 *des)
{
}

UINT16 *DB_STRCPY(UINT16 *dest, UINT16 *src)
{
    return NULL;
}

INT32 DB_STRCMP(UINT16 *s, UINT16 *t)
{
    return 0;
}

INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *node_pos)
{
    return 0;
}

INT32 undo_prog_modify(UINT32 tp_id,UINT16 prog_number)
{
    return 0;
}

INT32 save_tmp_data(UINT8 *buff,UINT16 buff_len)
{
    return 0;
}

INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *ch_v_cnt,UINT16 *ch_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt)
{
    return 0;
}

INT32 modify_sat(UINT16 sat_id,  S_NODE *node)
{
    return 0;
}

INT32 load_tmp_data(UINT8 *buff,UINT16 buff_len)
{
    return 0;
}

INT32 erase_tmp_sector()
{
    return 0;
}

INT32 recreate_tp_view(UINT16 create_flag, UINT32 param)
{
    return 0;
}

INT32 recreate_sat_view(UINT16 create_mode,UINT32 param)
{
    return 0;
}
#endif

#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif

#include <api/libosd/osd_lib.h>
#include <bus/tsi/tsi.h>
#include <api/libpvr/lib_pvr.h>

#define SDATA long

#ifdef  _BUILD_OTA_E_
UINT8 show_and_playchannel;

UINT8 win_ci_dlg_get_status()
{
    return 0;
}
void timermsg_show(char* msg)
{
}
BOOL menus_find_root(UINT32 msg_type,UINT32 msg_code,BOOL* b_flag,POBJECT_HEAD* winhandle)
{
    return FALSE;
}
BOOL cc_tsg_task_is_running(void)
{
    return FALSE;
}

INT32 pvr_r_request(PVR_HANDLE handle, UINT8 **addr, INT32 length)
{
    return 0;
}
INT32 pvr_p_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator)
{
    return 0;
}
BOOL pvr_r_update(PVR_HANDLE handle, UINT32 size, UINT16 offset)
{
    return TRUE;
}
PVR_HANDLE pvr_get_handle_to_dmx(UINT32 dmx_id)
{
    return 0;
}

void mpg_decode_init()
{
}

void mpg_decode_init_ad(void* mpg_decoder)
{
}

typedef struct
{    BYTE   *input_r_ptr;
    WORD   bs_length;
    SDATA  *output_w_ptr;
//    BYTE   *output_raw;
    BYTE   *extbs_r_ptr;
    WORD   extbs_length;
    BYTE   ismpeg1;
} DECINSIP_MPG;

typedef struct
{    BYTE status;
    BYTE bit_depth;
    WORD sample_rate;
    WORD samp_num;
    WORD left_data;
    WORD left_extdata;
    WORD chan_num;
    WORD samprateid;
    WORD inmod;
} DECOUTSIP_MPG;


DECOUTSIP_MPG  mpg_decode_one_frame(DECINSIP_MPG mpg_para)
{
    DECOUTSIP_MPG out;
    return out;
}

DECOUTSIP_MPG  mpg_decode_one_frame_ad(void* mpg_decoder, DECINSIP_MPG mpg_para)
{
    DECOUTSIP_MPG out;
    return out;
}

//ttx subt move to main 
//ota compiler for link error
INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
    return RET_FAILURE;
}

INT32  sdec_stop(struct sdec_device *dev)
{
    return RET_FAILURE;
}

void osd_subt_enter(void)
{

}

void osd_subt_leave(void)
{

}
#ifndef _M3503B_
INT32 as_service_query_stat(void)
{
    return 0;
}

INT32 as_service_register(void)
{
    return 0;
}

void api_stop_play(UINT32 bpause)
{
}
#endif

enum API_PLAY_TYPE api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password, BOOL b_id)
{
    return API_PLAY_NOCHANNEL;
}


#ifdef _MHEG5_SUPPORT_
typedef enum
{
    UI_MENU_OPEN = 0, 
    UI_MENU_CLOSE, 
    UI_MENU_PROC, 
}mhg_ui_menu_state;

void mheg_notify_video_pref_changed(void)
{
}
void mheg_save_cur_ap_ctrl_tsk_id(ID task_id)
{
}
UINT8 mheg_hook_proc(mhg_ui_menu_state State, POBJECT_HEAD pMenu, control_msg_t *pMsg)
{
}
void mheg_app_enter(void)
{
}
BOOL mheg_app_avaliable(void)
{
    return FALSE;
}
void mheg_running_app_kill(BOOL pmt_monitor)
{
}
#endif

#endif


