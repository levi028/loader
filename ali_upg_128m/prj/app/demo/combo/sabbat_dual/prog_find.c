/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: prog_find.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include "win_com.h"
#include "win_com_list.h"
#include "win_com_popup.h"
#include "string.id"
#include "osd_config.h"
#include <api/libpub/lib_as.h>
#include "key.h"
#include "copper_common/dev_handle.h"
#include <hal/hal_gpio.h>
#include "t2malloc.h"
#include <api/libosd/osd_common.h>
#ifdef POLAND_SPEC_SUPPORT


#define SUPPORT_LCN_UNIQUE //If enable the feature, please take care the move LCN feature in chan list UI, please also enable the feature in win_chan_list
#define LIST_ALL_CONFLICT

#define com_mem_set MEMSET

#define GET_VADDR(_addr)  ((UINT8 *)(((UINT32)(_addr) & 0x1fffffff) | 0x80000000))

#define DBG_ASSERT  OSD_ASSERT

INT32 set_pnode_lookup(INT32 (*pnode_lookup)(P_NODE *));

#define SUPPORT_DYNAMIC_SCAN    1 //0
#define APPEND_WEAK_SIGNAL_PROGRAM  0

#define SYS_MAX_NUM_PROGRAM MAX_PROG_NUM
#define __MM_NODE_VIEW_START_ADDR (__MM_AUTOSCAN_DB_BUFFER_ADDR+__MM_AUTOSCAN_DB_BUFFER_LEN)
#define __MM_NODE_VIEW_LEN  0x400000


#define C_OP_LOOKUP     0
#define C_OP_CHECK      1

#define C_CONFLICT_STRING_LENGTH 128

typedef const UINT16 *PCWSTR;
typedef WCHAR*          PWSTR;
typedef WCHAR*          PWCHAR;


typedef enum
{
    POPUP_MSG,
    POPUP_YESNO,
    POPUP_LIST,
}POPUP_STYLE;

typedef struct
{
    UINT32 string;      // [option], indicating text, if 0, the default text defined in m_YesnoSetup will be used.
    UINT16 str_id;
    UINT8 b_focus_id;     // [option], highlight button, if 0, the default button is "No" button
    UINT8 b_default_id;   // [option], if bWaitTime is not zero, when wait timeout, this is the default result.
    UINT8 b_wait_time;    // [option], if 0 or 0xFF, wait until user confirms, otherwise, specifies a duration seconds.
}POPUP_YESNO_PARAM;


typedef struct
{
    UINT16 id_title;     // [option], title string id for the popup keyboard window
    UINT32 sel_stat;     // [option, in/out], depends on bMultiSel, single selection index or multi-selection bitmap.
    INT16 n_count;       // [option], specifies the option number in pTable, it must be less than 32 for multi-selection list
    UINT8 b_wait_time;     // 0/0xFF: wait until user chooses, otherwise, wait time in seconds
    UINT32 *p_table;     // [option], depends on pCtrl, if pCtrl is NULL, this field must be specified.
    UINT32 def_sel;      // [option], default selection if pCtrl is invalid and not multisel list.
}POPUP_LIST_PARAM;


typedef enum
{
    PF_NOT_MATCHED = 0,
    PF_KEY_MATCHED,
    PF_ALL_MATCHED
}PF_RESULT;


typedef enum
{
    PROG_SAME = 0,
    PROG_SAME_BUT_DIFF_TP,
    PROG_DIFF_AT_ALL,
    PROG_DIFF_ORIGINAL_NETWORK_ID,
//    PROG_DIFF_NETWORK_ID,
} PROGRAM_COMPARING_RESULT;

typedef enum
{
    UNKNOWN_NETWORK_ID,
    AUSTRIAN_NETWORK_ID,
    FRENCH_NETWORK_ID,
    ITALIAN_NETWORK_ID,
    SLOVENIA_NETWORK_ID,
    SPANISH_NETWORK_ID,
    SWISS_NETWORK_ID,
}REGION_OF_NETWORK_ID;


typedef struct
{
    UINT16 delete_from_database :1; //delete the old program from database.
    UINT16 delete               :1; //set the deleted flag for the old program.
    UINT16 successor_service    :1; //old program: it will be replaced with new program. New program: PROGRAM_UPDATE.associated_prog_id = old program id.
    UINT16 update_hidden_flag   :1; //for old program: update hidden_flag only.
    UINT16 update_service_name  :1; //for old program: update service_name only.
    UINT16 update_lcn           :1; //for old program: update LCN only.
    UINT16 rearrange_lcn        :1; //for old and new program: the LCN need to be rearranged.
    UINT16 append               :1; //new program need to append into database.
} PROGRAM_UPDATE_CONTROLLER, *PTR_PROGRAM_UPDATE_CONTROLLER;

typedef struct
{
    UINT16 associated_prog_id;  //program id that successor_service will replace to.
    PROGRAM_UPDATE_CONTROLLER   ctrl;
    P_NODE p_node;
} PROGRAM_UPDATE, *PTR_PROGRAM_UPDATE;

typedef struct
{
    UINT16 n_count;
    PROGRAM_UPDATE **program_list;
}PROGRAM_LIST, *PTR_PROGRAM_LIST;

typedef struct
{
    UINT16 n_count;
    UINT16 *tp_list;
}TP_LIST, *PTR_TP_LIST;


typedef struct _PARAM_NODE
{
    UINT16 n_id;
    UINT16 n_channel:15;
    UINT16 b_new_node:1;

    UINT32 prog_number            :16;
    UINT32 ts_id                :16;

    UINT32 LCN                  :16;
    UINT32 orig_lcn             :16;

    UINT32 lcn_true             : 1;
    UINT32 av_flag              : 1;
    UINT32 ca_mode              : 1;
    UINT32 post_update          : 1;
    UINT32 post_lcn_update      : 1;
    UINT32 hidden_flag          : 1;
    UINT32 user_changed_service_name    : 1;
    UINT32 user_changed_lcn     : 1;
    UINT32 deleted              : 1;
    UINT32 reserved             :23;

    UINT32 prog_id;

    PROGRAM_UPDATE_CONTROLLER   ctrl;

    UINT16 current_index;
    UINT16 tp_id;
    WCHAR sz_name[MAX_SERVICE_NAME_LENGTH+1];

    struct _PARAM_NODE *p_next;
}PARAM_NODE, *PPARAM_NODE;

typedef struct _PROGRAM_NODE
{
    UINT16 orig_network_id;
    PPARAM_NODE p_list;
}PROGRAM_NODE, *PPROGRAM_NODE;

typedef struct _PROGRAM_FIND
{
    UINT16 n_count;
    UINT16 n_pos;
    UINT16 n_next_id;
    PPROGRAM_NODE *p_node_index;
}PROGRAM_FIND, *PPROGRAM_FIND;

typedef struct
{
    UINT16 preference_zone_1_min;
    UINT16 preference_zone_1_max;
    UINT16 preference_zone_2_min;
    UINT16 preference_zone_2_max;
    UINT16 preference_overflow_zone_max;
    UINT16 available_preference_overflow_lcn;
    UINT16 available_main_overflow_lcn;
    UINT16 new_overflow_lcn; //for main overflow full then from 849 backwards
} LCN_ZONE, *PTR_LCN_ZONE;



#define SYS_MAX_NUM_TP  300

static BOOL post_processing_init();
static void preprocess_found_program(P_NODE *p_node);
PROGRAM_COMPARING_RESULT compare_program(const P_NODE *p_prog1, const P_NODE *p_prog2);
static BOOL post_processing_insert_node(P_NODE *p_node);
BOOL post_processing_add_normal_tp(UINT16 tp_id);
UINT16 post_processing_get_prog_idx(P_NODE *p_node);
INT32 post_processing_get_prog_node(UINT16 n_id, P_NODE *p_node);
INT32 post_processing_lookup_node(P_NODE *p_node);
BOOL post_processing_save();

BOOL prompt_program_deleted(PPARAM_NODE p_param);
UINT8 prompt_resolve_lcn_conflict(const P_NODE *p_prog1, const P_NODE *p_prog2);
static PWSTR *create_lcn_conflict_list_buffer(UINT8 count);
static void destroy_lcn_conflict_list_buffer(PWSTR *p_list);
static void create_lcn_conflict_string(const P_NODE *p_prog, PWCHAR p_buffer, UINT32 n_size);
void set_auto_resolve_lcn_conflict(BOOL enable);
static void mark_deleted_service();
static void mark_update_append_service();
static void param_node_to_p_node(P_NODE *p_node, const PPARAM_NODE p_param);

#ifdef LIST_ALL_CONFLICT
static  BOOL conflict_list_iinitalize();
static void resolve_lcn_conflict_ext();
#endif

static BOOL m_prompt_mode = TRUE;
static BOOL m_auto_resolve_lcn_conflict = FALSE;
static PROGRAM_LIST m_scanning_found;
static PROGRAM_LIST m_save;
static TP_LIST m_normal_tplist;  //TP list that SDT and NIT had been receved complete.
static REGION_OF_NETWORK_ID m_cur_region_network_id;


extern UINT8 as_stop_scan_signal;

//---------------------------- PRIVATE VARIABLES ----------------------------//
static PPROGRAM_FIND m_p_find = NULL;
static OSAL_ID m_id_lock = OSAL_INVALID_ID;
static UINT16 m_old_radio_count, m_old_tvcount;
static UINT16 m_cur_radio_count, m_cur_tvcount;
//---------------------------- PRIVATE FUNCTIONS ----------------------------//


#ifndef LOG_PROG_FIND
#define DBG_PRINTF(...)  do{} while(0)//libc_printf
#define print_prog_node(...)
#define print_param_node(...)
#define print_node_table(...)
#define print_scanning_found(...)
#define print_setting(...)
#define print_p_node(...)
#define print_db(...)
#endif

#define SUB_LIST_T_OFFSET    60    // sub list title offset
#define SUB_LIST_B_OFFSET    10    // sub list bottom offset
#define SUB_LIST_H_OFFSET     (SUB_LIST_T_OFFSET+SUB_LIST_B_OFFSET)

static UINT32 *conflict_p_list;

PRESULT comlist_lcn_conflict_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i,w_top;
    char str_len;
        OBJLIST* ol;

    PRESULT cb_ret = PROC_PASS;

        ol = (OBJLIST*)p_obj;

    if(event==EVN_PRE_DRAW)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            win_comlist_set_str(i + w_top,NULL,(char*)conflict_p_list[i+w_top],0 );
        }
    }
    else if(event == EVN_POST_CHANGE)
        cb_ret = PROC_LEAVE;

    return cb_ret;
}

UINT16 win_conflict_list_open(OSD_RECT* p_rect,  UINT32 popup_param)
{
    struct osdrect rect;
    UINT16 i,count,dep,cur_idx;
    UINT16 style;
    UINT8 mark_align,offset;
    PFN_KEY_MAP winkeymap;
    PFN_CALLBACK callback;
    UINT8 back_saved;
    POPUP_LIST_PARAM *param = (POPUP_LIST_PARAM*)popup_param;
    char title[128];
    extern TEXT_FIELD list_title;
    extern OBJLIST      g_ol_com_list;
    TEXT_FIELD* txt ;
    OBJLIST *ol;

    dep = (p_rect->u_height - SUB_LIST_H_OFFSET) / 40;

       conflict_p_list = param->p_table;

    count = param->n_count;
    winkeymap = NULL;
    callback = comlist_lcn_conflict_callback;

    cur_idx = param->def_sel;

    txt = &list_title;
    ol = &g_ol_com_list;
    txt->head.p_next = NULL;
    ol->head.p_next = &list_title;
    osd_set_rect2(&rect,p_rect);
    offset = 30;

    style = LIST_VER | LIST_SCROLL | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE;
    style |= LIST_SINGLE_SLECT;
    mark_align = C_ALIGN_RIGHT;
    win_comlist_reset();
    snprintf(title,128,"Which program hold the LCN?");
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)title);
    win_comlist_set_sizestyle(count, dep,style);
    win_comlist_set_align(10,0,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    win_comlist_ext_enable_scrollbar(TRUE);
    win_comlist_set_frame(rect.u_left,rect.u_top, rect.u_width, rect.u_height);

    win_comlist_ext_set_title_rect(rect.u_left+10,rect.u_top+10, rect.u_width-20, 40);
    win_comlist_ext_set_title_style(WSTL_TEXT_30_8BIT);

    win_comlist_set_mapcallback(NULL,winkeymap,callback);

    win_popup_list_set_timer(1000, CTRL_MSG_TYPE_POPUP_TIMER, param->b_wait_time);

    win_comlist_ext_set_ol_frame(rect.u_left + 10, \
        rect.u_top + SUB_LIST_T_OFFSET, \
        rect.u_width - 40, \
        rect.u_height - SUB_LIST_H_OFFSET);
    win_comlist_ext_set_selmark_xaligen(mark_align,offset);
    win_comlist_ext_set_selmark(0);
    win_comlist_ext_set_item_sel(cur_idx);

    win_comlist_ext_set_item_cur(cur_idx);

    win_comlist_popup_ext(&back_saved);

    ol->head.p_next = NULL;
    return win_comlist_get_selitem();
}


INT32 ap_open_popup(POPUP_STYLE type, UINT32 popup_param)
{
    win_popup_choice_t choice, default_choice;
    UINT8 back_saved;

    if(type == POPUP_MSG)
    {
        POPUP_YESNO_PARAM  *param = (POPUP_YESNO_PARAM*)popup_param;
        win_compopup_set_frame(GET_MID_L(600),GET_MID_T(100),600,220);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext(NULL, NULL, param->str_id);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        return SUCCESS;
    }
       else if(type == POPUP_YESNO)
       {
           POPUP_YESNO_PARAM  *param = (POPUP_YESNO_PARAM*)popup_param;
        default_choice=WIN_POP_CHOICE_NO;
        if(param->b_wait_time != 0)
        {
            if(param->b_default_id == 1)
                default_choice = WIN_POP_CHOICE_YES;  //default Yes
            else
                default_choice = WIN_POP_CHOICE_NO;  //default No
        }
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_frame(GET_MID_L(850),GET_MID_T(100),850,220);

        win_compopup_set_msg_ext(NULL, (char*)param->string, (UINT16)NULL);
        win_compopup_set_default_choice(default_choice);

        win_compopup_set_timer(1000, CTRL_MSG_TYPE_POPUP_TIMER, param->b_wait_time);

        choice = win_compopup_open_ext(&back_saved);

        return choice;

       }
    else //type == POPUP_LIST
    {
        OSD_RECT rect;
        rect.u_left = 150;
        rect.u_top = 200;
        rect.u_width = 700;
    #ifdef LIST_ALL_CONFLICT
           POPUP_LIST_PARAM *param =(POPUP_LIST_PARAM*)popup_param;
           if(param->n_count <=5)
            rect.u_height = 40*param->n_count+80;
        else
            rect.u_height = 40*5+80;
    #else
        rect.u_height = 200;
    #endif
        choice = win_conflict_list_open(&rect, popup_param);

        return choice;
    }


}

void ap_close_popup()
{
    win_compopup_close();
}

static int compare_key(UINT32 key1, UINT32 key2)
{
    if(key1 > key2)
        return 1;
    else if(key1 < key2)
        return -1;
    return 0;
}

static int compare_signal(UINT32 q1, UINT32 q2)
{
    if(q1 < q2)
        return -1;
    else if(q1 > q2)
        return 1;

    return 0;
}

static PF_RESULT find_param(PPARAM_NODE p_list, P_NODE *p_node, PPARAM_NODE *pp_param, UINT8 b_op_mode)
{
    PPARAM_NODE p_node = p_list;
    T_NODE t_node;
    INT32 er, match_count;
    PF_RESULT result = PF_KEY_MATCHED;

    er = get_tp_by_id(p_node->tp_id, &t_node);
    DBG_ASSERT(SUCCESS == er);
    while(p_node)
    {
        if(p_node->prog_number == p_node->prog_number && p_node->ts_id == t_node.t_s_id)
        {
            if(pp_param)
                *pp_param = p_node;
            return PF_ALL_MATCHED;
        }
        p_node = p_node->p_next;
    }
    if(C_OP_LOOKUP == b_op_mode)
        result = PF_NOT_MATCHED;
    return result;
}

static void insert_param(PPROGRAM_NODE p_program, P_NODE *p_node, UINT16 n_id, UINT16 n_channel, BOOL b_new_node)
{
    PPARAM_NODE p_param, p_temp, p_prev;
    T_NODE t_node;

    p_param = (PPARAM_NODE)t2_malloc(sizeof(*p_param));
    DBG_ASSERT(p_param);
    com_mem_set(p_param, 0, sizeof(*p_param));

    get_tp_by_id(p_node->tp_id, &t_node);

    p_param->n_id = n_id;
    p_param->n_channel = n_channel;
    if(b_new_node)
        p_param->b_new_node = 1;

    p_param->orig_lcn = p_node->orig_lcn;
    p_param->user_changed_service_name = p_node->user_changed_service_name;
    p_param->user_changed_lcn = p_node->user_changed_lcn;
    p_param->deleted = p_node->deleted;
    p_param->hidden_flag = p_node->hidden_flag;

    p_param->prog_number = p_node->prog_number;
    p_param->ts_id = t_node.t_s_id;
    p_param->LCN = p_node->LCN;
    p_param->lcn_true = p_node->lcn_true;
    p_param->av_flag = p_node->av_flag;
    p_param->ca_mode = p_node->ca_mode;
    p_param->current_index = p_param->current_index;
    p_param->tp_id = p_node->tp_id;

    p_param->prog_id = p_node->prog_id;

    com_uni_str_copy_char_n((UINT8*)p_param->sz_name, p_node->service_name, MAX_SERVICE_NAME_LENGTH);

    p_prev = NULL;
    p_temp = p_program->p_list;
    while(p_temp != NULL)
    {
        p_prev = p_temp;
        p_temp = p_temp->p_next;
    }
    if(NULL == p_prev)
        p_program->p_list = p_param;
    else
        p_prev->p_next = p_param;
}

static PF_RESULT find_node(P_NODE *p_node, INT16 *p_pos, PPARAM_NODE *pp_param, UINT8 b_op_mode)
{
    PPROGRAM_NODE p_node;
    INT16 i = 0, min = 0, max = m_p_find->n_count, pos = 0;
    int cmp = -1;
    T_NODE t_node;
    PF_RESULT result = PF_NOT_MATCHED;

    get_tp_by_id(p_node->tp_id, &t_node);
    while(min < max)
    {
        i = (min+max)/2;
        p_node = m_p_find->p_node_index[i];
        cmp = compare_key(t_node.network_id, p_node->orig_network_id);
        if(!cmp)
        {
            pos = i;
            result = find_param(p_node->p_list, p_node, pp_param, b_op_mode);
            break;
        }
        else if(cmp > 0)
            pos = min = i + 1;
        else
            pos = max = i;
    }
    if(p_pos)
        *p_pos = pos;
    return result;
}

static PPROGRAM_NODE insert_node(P_NODE *p_node, UINT16 n_channel, BOOL b_new_node)
{
    INT16 pos;
    int result;
    PPROGRAM_NODE p_node;
    T_NODE t_node;

    // find inserting position
    result = find_node(p_node, &pos, NULL, C_OP_CHECK);

    //If 2 programs are PF_ALL_MATCHED, but own different tp,
    //Z-Book consider they are different programs.
    //So they shall be appended into m_pFind->pNodeIndex[pos].


    if(PF_NOT_MATCHED == result)
    {
        // move program node for inserting node
        if(m_p_find->n_count)
        {
            INT16 i;
            for(i=m_p_find->n_count; i>pos; i--)
                m_p_find->p_node_index[i] = m_p_find->p_node_index[i-1];
        }

        // add new node.
        p_node = (PPROGRAM_NODE)t2_malloc(sizeof(*p_node));
        DBG_ASSERT(p_node);
        com_mem_set(p_node, 0, sizeof(*p_node));

        get_tp_by_id(p_node->tp_id, &t_node);
        p_node->orig_network_id = t_node.network_id;

        m_p_find->p_node_index[pos] = p_node;
        m_p_find->n_count++;
        DBG_ASSERT(m_p_find->n_count <= SYS_MAX_NUM_PROGRAM);
    }
    else
        p_node = m_p_find->p_node_index[pos];

    insert_param(p_node, p_node, m_p_find->n_next_id++, n_channel, b_new_node);
    return p_node;
}

static BOOL add_nodes()
{
    P_NODE node;
    UINT16 i, count;
    INT32 er;

    count = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE,0);
    for(i=0; i<count; i++)
    {
        er = get_prog_at(i, &node);
        if(SUCCESS != er)
            break;
        insert_node(&node, i, FALSE);
    }
    return TRUE;
}


UINT16 get_signal_intensity(UINT16 tp_id)
{
    T_NODE tp;

    get_tp_by_id(tp_id, &tp);

    return tp.intensity;
}

BOOL post_update_program()
{
    return post_processing_save();
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void ap_set_program_count(UINT16 last_radio_count, UINT16 last_tvcount)
{
    m_old_radio_count = m_cur_radio_count = last_radio_count;
    m_old_tvcount = m_cur_tvcount = last_tvcount;
}


BOOL  init_node_view_callback()
{
    if(!add_nodes())
    {
        DBG_PRINTF("%s : Initalize Node View Fail\n", __FUNCTION__);
        return FALSE;
    }
        DBG_PRINTF("%s :  Initalize Node View Successful\n", __FUNCTION__);
     return TRUE;
}

static void find_max_default_index(UINT16 *video_index, UINT16 *audio_index)
{
    UINT16  prog_cnt, audio_cnt, tvcnt;
    UINT16 i;
    P_NODE p_node;
    UINT16 tmp_default_index = 0;


    prog_cnt = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE, 1);
    if(prog_cnt == 0)
    {
        *video_index = 0;
        *audio_index = 0;
        return;
    }

       tvcnt = get_prog_num(VIEW_ALL | TV_CHAN , 1);

    if(tvcnt == 0)
        *video_index = 0;
    else
    {
        for(i=0; i<prog_cnt ; i++)
        {
            get_prog_at(i, &p_node);
            if(p_node.av_flag != 1)
                continue;
            if(p_node.default_index > tmp_default_index)
                tmp_default_index  = p_node.default_index;
        }

        *video_index = tmp_default_index+1;
    }

    tmp_default_index = 0;
    audio_cnt = get_prog_num(VIEW_ALL | RADIO_CHAN, 1);

    if(audio_cnt == 0)
        *audio_index = 0;
    else
    {
        for(i=0; i<prog_cnt ; i++)
        {
            get_prog_at(i, &p_node);
            if(p_node.av_flag != 0)
                continue;
            if(p_node.default_index > tmp_default_index)
                tmp_default_index  = p_node.default_index;
        }

        *audio_index = tmp_default_index+1;
    }
}


BOOL ap_init_node_view_ext(UINT8 view_mode)
{
    UINT8 mode, temp, pre_mode;
    UINT32 ts;
    BOOL b_result = FALSE;
    UINT16 audio_cnt=0;
    UINT16 tvcnt=0;
    UINT16 data_cnt=0;

    if(OSAL_INVALID_ID == m_id_lock)
    {
        m_id_lock = osal_semaphore_create(1);
        DBG_PRINTF("%s: idLock=%d\n", __FUNCTION__, m_id_lock);
    }
    DBG_PRINTF("%s: start=0x%x, len=0x%x, max=0x%x\n", __FUNCTION__,
                __MM_NODE_VIEW_START_ADDR, __MM_NODE_VIEW_LEN,
                sizeof(PROGRAM_FIND)+SYS_MAX_NUM_PROGRAM*(sizeof(PPROGRAM_NODE)+sizeof(PROGRAM_NODE)+sizeof(PARAM_NODE)));
    t2_malloc_init(GET_VADDR(__MM_NODE_VIEW_START_ADDR), __MM_NODE_VIEW_LEN);
    m_p_find = (PPROGRAM_FIND)t2_malloc(sizeof(*m_p_find));
    DBG_ASSERT(m_p_find);
    com_mem_set(m_p_find, 0, sizeof(*m_p_find));

    m_p_find->p_node_index = (PPROGRAM_NODE *)t2_malloc(SYS_MAX_NUM_PROGRAM*sizeof(PPROGRAM_NODE));
    DBG_ASSERT(m_p_find->p_node_index);

    mode = sys_data_get_cur_chan_mode();

    recreate_prog_view(VIEW_SINGLE_SAT|PROG_ALL_MODE, 1);

    data_cnt = get_prog_num(VIEW_SINGLE_SAT|PROG_DATA_MODE, 1); //for data service handle

   if(mode == PROG_TV_MODE)
       tvcnt = get_prog_num(VIEW_ALL | TV_CHAN, 1);
   else
    audio_cnt = get_prog_num(VIEW_ALL |RADIO_CHAN, 1);

    if(view_mode)
    {
        temp = (PROG_TV_MODE == mode) ? PROG_RADIO_MODE : PROG_TV_MODE;
     if(mode == PROG_TV_MODE)
        audio_cnt = get_prog_num(VIEW_ALL |temp, 1);
    else
        tvcnt = get_prog_num(VIEW_ALL|temp, 1);
    }
    else
    {
            if(!add_nodes())
               return FALSE;
    }

    if (post_processing_init() == FALSE)
        return FALSE;

    set_pnode_lookup(post_processing_lookup_node);

    //FindMaxDefaultIndex(&TVCnt, &AudioCnt);

    ap_set_program_count(audio_cnt,  tvcnt);

    if(sys_data_get_lcn() == 0)
        set_auto_resolve_lcn_conflict(TRUE);  //Don't prompt user for the conflict if LCN Mode was turn Off.

    node_view_inital_registor(init_node_view_callback);

    DBG_PRINTF("====================start: %s=====================\n",__FUNCTION__);
    print_setting();
    DBG_PRINTF("--------------------step 1: original database---------------------\n");
    print_node_table();

    return b_result;
}

BOOL ap_init_node_view()
{
    ap_init_node_view_ext(1);
}

void ap_free_node_view()
{
    if(m_p_find != NULL)
    {
        set_pnode_lookup(NULL);
        m_p_find = NULL;
    }

    m_scanning_found.n_count = 0;
    m_scanning_found.program_list = NULL;
    m_save.n_count=0;
   m_save.program_list=NULL;
    m_normal_tplist.n_count = 0;
    m_normal_tplist.tp_list = NULL;
    node_view_inital_unregistor();
}

//=============================================================================

//Italian local original_network_id:
//Italia: 0x217C
//RAI:  0x13E, 0x13F,  0x1B
//Mediaset:  0x110
//Telecom Italia: 0x1D

REGION_OF_NETWORK_ID region_of_network_id(UINT16 net_id, UINT16 original_network_id)
{
    if (net_id >= 0x3301 && net_id <= 0x3400)
        return AUSTRIAN_NETWORK_ID;
    else if (net_id == 0x20FA || (net_id >= 0x3301 && net_id <= 0x3400) )
        return FRENCH_NETWORK_ID;
#ifdef FUBA_SUPPORT
    else if ((net_id >= 0x3001 && net_id <= 0x3100))
 #else
     else if ( (net_id >= 0x3001 && net_id <= 0x3100) \
              ||(   original_network_id == 0x217C \
                 || original_network_id == 0x13E \
                 || original_network_id == 0x13F \
                 || original_network_id == 0x1B \
                 || original_network_id == 0x110 \
                 || original_network_id == 0x1D )\
            )
 #endif
        return ITALIAN_NETWORK_ID;
    else if (net_id >= 0x3201 && net_id <= 0x3300)
        return SLOVENIA_NETWORK_ID;
    else if (net_id >= 0x3101 && net_id <= 0x3200 )
        return SPANISH_NETWORK_ID;
    else if (net_id >= 0x3201 && net_id <= 0x3300)
        return SWISS_NETWORK_ID;
    return UNKNOWN_NETWORK_ID;
}

REGION_OF_NETWORK_ID get_region_by_network_id(const P_NODE *p_prog)
{
    T_NODE t_node;
    get_tp_by_id(p_prog->tp_id, &t_node);
    return region_of_network_id(t_node.net_id, t_node.network_id);
}

const UINT16* get_region_table()
{
    static const UINT16 country_strs_poland_spec[]  =  //must the same as the country_strs2[] in the win_time.c
    {
        RS_REGION_ARG,
        RS_REGION_AUS,
        RS_REGION_BRA,
        RS_REGION_CHN,
        RS_REGION_DEN,
        RS_REGION_FIN,
        RS_REGION_FRA,
        RS_REGION_GER,
        RS_REGION_HK,
        RS_REGION_ITA,
        RS_REGION_POL,
        RS_REGION_RUS,
        RS_REGION_SIG,
        RS_REGION_SPA,
        RS_REGION_SWE,
        RS_REGION_TW,
        RS_REGION_UK,
    #ifdef PERSIAN_SUPPORT
        RS_REGION_IRAN,
    #endif
    };

    return country_strs_poland_spec;

}


static BOOL post_processing_init()
{
    const UINT16 *p_region = get_region_table();
    UINT8 region = sys_data_get_country();
    m_cur_region_network_id = UNKNOWN_NETWORK_ID;
    if(RS_REGION_FRA == p_region[region])
        m_cur_region_network_id = FRENCH_NETWORK_ID;
    else if(RS_REGION_ITA== p_region[region])
        m_cur_region_network_id = ITALIAN_NETWORK_ID;

    else if(RS_REGION_SPA== p_region[region])
        m_cur_region_network_id = SPANISH_NETWORK_ID;

    set_auto_resolve_lcn_conflict(FALSE);

    m_scanning_found.n_count = 0;
    m_scanning_found.program_list = (PROGRAM_UPDATE **)t2_malloc(SYS_MAX_NUM_PROGRAM*sizeof(PROGRAM_UPDATE*));
    DBG_ASSERT(m_scanning_found.program_list);
    if (m_scanning_found.program_list == NULL)
        return FALSE;
    com_mem_set((UINT8 *)m_scanning_found.program_list, 0, sizeof(SYS_MAX_NUM_PROGRAM*sizeof(PROGRAM_UPDATE*)));

     m_save.n_count = 0;
    m_save.program_list = (PROGRAM_UPDATE **)t2_malloc(SYS_MAX_NUM_PROGRAM*sizeof(PROGRAM_UPDATE*));
    DBG_ASSERT(m_save.program_list);
    if (m_save.program_list == NULL)
        return FALSE;
    com_mem_set((UINT8 *)m_save.program_list, 0, sizeof(SYS_MAX_NUM_PROGRAM*sizeof(PROGRAM_UPDATE*)));

#ifdef LIST_ALL_CONFLICT
   conflict_list_iinitalize();
#endif

    m_normal_tplist.n_count = 0;
    m_normal_tplist.tp_list = (UINT16 *)t2_malloc(SYS_MAX_NUM_TP*sizeof(UINT16));
    DBG_ASSERT(m_normal_tplist.tp_list);
    if (m_normal_tplist.tp_list == NULL)
        return FALSE;
    com_mem_set((UINT8 *)m_normal_tplist.tp_list, 0, sizeof(SYS_MAX_NUM_TP*sizeof(UINT16)));
    return TRUE;
}

static BOOL post_processing_insert_node(P_NODE *p_node)
{
    PROGRAM_UPDATE * p_prog;
    if(m_scanning_found.n_count >= SYS_MAX_NUM_PROGRAM)
        return FALSE;
    p_prog = (PROGRAM_UPDATE *)t2_malloc(sizeof(PROGRAM_UPDATE));
    com_mem_set((UINT8 *)p_prog, 0, sizeof(PROGRAM_UPDATE) );
    MEMCPY((UINT8 *)&p_prog->p_node, (UINT8 *)p_node, sizeof(P_NODE));
    preprocess_found_program(&p_prog->p_node);
    m_scanning_found.program_list[m_scanning_found.n_count]= p_prog;
    m_scanning_found.n_count++;
    return TRUE;
}

BOOL post_processing_add_normal_tp(UINT16 tp_id)
{
    if(m_normal_tplist.n_count >= SYS_MAX_NUM_TP)
        return FALSE;
    m_normal_tplist.tp_list[m_normal_tplist.n_count] = tp_id;
    m_normal_tplist.n_count++;
    return TRUE;
}


UINT8*  get_def_langcode()
{
    extern char *stream_iso_639lang_abbr[];
    SYSTEM_DATA* p_sys_data=sys_data_get();
    return (UINT8*)&stream_iso_639lang_abbr[p_sys_data->lang.audio_lang_1];
}

static UINT8 find_default_audio_idx(P_NODE *p_node)
{
    INT32 i=0,j=0;
    UINT32 cur_audio=0;
    UINT8 *plangcode=NULL;
    UINT8 usr_lang_sel;

    UINT8 audio_count = p_node->audio_count;
    UINT8 *audio_type = p_node->audio_type;
    UINT16 *audio_pid = p_node->audio_pid;
    UINT8 audio_lang[P_MAX_AUDIO_NUM][3];

    for(i=0; i<audio_count; i++)
        get_audio_lang3b((UINT8 *)&p_node->audio_lang[i], audio_lang[i]);

    plangcode = get_def_langcode();

    if(audio_count > 1)
    {
        INT32 k=0;
        UINT32 record[P_MAX_AUDIO_NUM];
        MEMSET(record, 0, P_MAX_AUDIO_NUM*4);
        UINT32 default_sel = 0xFFFFFFFF, und_sel = 0xFFFFFFFF;

        for(j=0; j<audio_count; j++)
        {
            if(audio_type[j] ==0x0)
            {
#ifdef DEFAULT_AUDIO_AVOID_AC3
                if(audio_pid[j] & AC3_DES_EXIST)
                    continue;
#endif
                record[k++] = (UINT32)j;
            }
        }
        if(k >0)
        {
            for(j=0;j<k;j++)
            {
                if(MEMCMP(audio_lang[record[j]], plangcode, 3)==0)
                {
                    break;
                }
                else if(und_sel == 0xFFFFFFFF)
                {
                    if( (MEMCMP(audio_lang[record[j]], "und", 3)==0)
                        || (MEMCMP(audio_lang[record[j]], "UND", 3)==0))
                    {
                        und_sel = record[j];
                    }
                    else if(default_sel == 0xFFFFFFFF)
                    {
                        default_sel = record[j];
                    }
                }
            }
            if(j==k)
            {
                if(und_sel!=0xFFFFFFFF)
                {
                    cur_audio = und_sel;
                }
                else if(default_sel!=0xFFFFFFFF)
                {
                    cur_audio = default_sel;
                }
                else
                {
                    cur_audio = record[0];
                    for(j=1;j<k;j++)//select lowest PID
                    {
                        if(audio_pid[record[j]] < audio_pid[cur_audio])
                            cur_audio = record[j];
                    }
                }
            }
            else
            {
                cur_audio = record[j];
            }
        }
        else
        {
            cur_audio = 0;
            for(j=1;j<audio_count;j++)//select lowest PID
            {
                if(audio_pid[j] < audio_pid[cur_audio])
                    cur_audio = j;
            }
        }
    }
    else
    {
        cur_audio = 0;
    }
    return cur_audio;
}

static void preprocess_found_program(P_NODE *p_node)
{
    UINT16 i;

    p_node->cur_audio = find_default_audio_idx(p_node);
}

PROGRAM_COMPARING_RESULT compare_program(const P_NODE *p_prog1, const P_NODE *p_prog2)
{
    T_NODE t_node1, t_node2;

    if( p_prog1->av_flag != p_prog2->av_flag)
        return PROG_DIFF_AT_ALL;

    get_tp_by_id(p_prog1->tp_id, &t_node1);
    get_tp_by_id(p_prog2->tp_id, &t_node2);
    if( t_node1.t_s_id != t_node2.t_s_id \
        || p_prog1->prog_number != p_prog2->prog_number )
        return PROG_DIFF_AT_ALL;
    if( 0!=com_uni_str_cmp_ext(p_prog1->service_name, p_prog2->service_name))
        return PROG_DIFF_AT_ALL;

    if( t_node1.network_id != t_node2.network_id )
        return PROG_DIFF_ORIGINAL_NETWORK_ID;

    return PROG_SAME;
}

UINT16 post_processing_get_prog_idx(P_NODE *p_node)
{
    P_NODE *p_prog = NULL;
    UINT16 n_pos = INVALID_ID;

    osal_semaphore_capture(m_id_lock, OSAL_WAIT_FOREVER_TIME);
    for (n_pos = 0; n_pos < m_scanning_found.n_count; ++n_pos)
    {
        p_prog = &m_scanning_found.program_list[n_pos]->p_node;
        if ( compare_program(p_node, p_prog) == PROG_SAME  \
             && p_node->tp_id == p_prog->tp_id )
             {
             p_prog->prog_id=p_node->prog_id;//add by IRIS on 8/5
             break;
             }
    }
    osal_semaphore_release(m_id_lock);
    return (n_pos < m_scanning_found.n_count ? n_pos : INVALID_ID);
}

UINT32 get_prog_id_indb(P_NODE *p_node)
{
    P_NODE *p_prog = NULL;
    UINT16 n_pos = INVALID_ID;
    UINT32 prog_index_indb=0;

    osal_semaphore_capture(m_id_lock, OSAL_WAIT_FOREVER_TIME);
    for (n_pos = 0; n_pos < m_scanning_found.n_count; ++n_pos)
    {
        p_prog = &m_scanning_found.program_list[n_pos]->p_node;
        if ( compare_program(p_node, p_prog) == PROG_SAME  \
             && p_node->tp_id == p_prog->tp_id )
           {
           prog_index_indb=p_prog->prog_id;
           break;
          }
    }

    osal_semaphore_release(m_id_lock);
    return prog_index_indb;
}
INT32 post_processing_get_prog_node(UINT16 n_id, P_NODE *p_node)
{
    UINT16 i;
    PPARAM_NODE p_param;

    osal_semaphore_capture(m_id_lock, OSAL_WAIT_FOREVER_TIME);
    com_mem_set((UINT8 *)p_node, 0, sizeof(*p_node));
    if (n_id >= m_scanning_found.n_count)
    {
        osal_semaphore_release(m_id_lock);
        return !SUCCESS;
    }

    MEMCPY((UINT8 *)p_node, (UINT8 *)&m_scanning_found.program_list[n_id]->p_node, sizeof(P_NODE));

    osal_semaphore_release(m_id_lock);
    return SUCCESS;
}

INT32 post_processing_lookup_node(P_NODE *p_node)
{
    BOOL result;
    //UINT8 op_mode = (add_flag == 0xFF) ? C_OP_CHECK : C_OP_LOOKUP;
    UINT8 op_mode = C_OP_LOOKUP;

    if(C_OP_LOOKUP == op_mode)
    {
        osal_semaphore_capture(m_id_lock, OSAL_WAIT_FOREVER_TIME);
        result = post_processing_insert_node(p_node);
        if (FALSE == result)
        {
            DBG_PRINTF("%s: full.\n", __FUNCTION__);
        }
        osal_semaphore_release(m_id_lock);
    }

    if(C_OP_LOOKUP == op_mode)
        return SUCCESS; //step 1: make p_search.c:add_search_prog() don't append it into database.
    else
        return SUCCESS; //step 2: make p_search.c:tp_search_update() always sent out message: ASM_TYPE_DUP_PROGRAM.

}


BOOL prompt_program_deleted(PPARAM_NODE p_param)
{
    BOOL confirm = TRUE;
    POPUP_YESNO_PARAM param;
    WCHAR wstr[200];
    WCHAR space[10];
    UINT8 len;

    if ( !m_prompt_mode )
        return TRUE;

    ap_close_popup();

    com_uni_str_copy_char_n((UINT8*)wstr, (UINT8*)p_param->sz_name, ARRAY_SIZE(wstr)-1);
    com_asc_str2uni("  ", space);
    len = com_uni_str_cat(wstr, space);
    len += com_uni_str_copy_char_n((UINT8*)(&wstr[len]), osd_get_unicode_string(RS_DISPLAY_SURE_TO_DELETE), ARRAY_SIZE(wstr)-1-len );
    param.string = (UINT32)wstr;
    param.b_focus_id = 1;
    param.b_default_id = 1;
    param.b_wait_time = 10;
    INT32 ret = ap_open_popup(POPUP_YESNO, (UINT32)&param);
    confirm = (WIN_POP_CHOICE_YES == ret);
    return confirm;
}

UINT8 prompt_resolve_lcn_conflict(const P_NODE *p_prog1, const P_NODE *p_prog2)
{
    POPUP_LIST_PARAM list_param =
    {
        RS_WHICH_PROGRAM_HOLD_THE_LCN, 0, 0, 0 , NULL, 0
    };
    INT32 popup_result = SUCCESS;
    UINT8 total_option=2+1, result;
    PWSTR *p_list;
    POPUP_YESNO_PARAM param;

    if ( !m_prompt_mode || m_auto_resolve_lcn_conflict)
        return 0;   //auto choose pProg1.

    ap_close_popup();

    p_list = create_lcn_conflict_list_buffer(total_option);
    ASSERT(p_list != NULL);

    // make popup list string
    create_lcn_conflict_string(p_prog1, p_list[0], C_CONFLICT_STRING_LENGTH);
    create_lcn_conflict_string(p_prog2, p_list[1], C_CONFLICT_STRING_LENGTH);
    com_uni_str_copy_char_n((UINT8*)p_list[2], osd_get_unicode_string(RS_AUTOMATICALLY_WITH_NO_PROMPT), C_CONFLICT_STRING_LENGTH-1);

    // Indicate end-user to handle conflict items
    list_param.p_table = (UINT32 *)p_list;
    list_param.n_count =total_option;
    list_param.def_sel = list_param.sel_stat;
    list_param.b_wait_time = 10;
    popup_result = ap_open_popup(POPUP_LIST, (UINT32)&list_param);

    if( total_option >2 && popup_result == (UINT8)(total_option - 1) )
    {
        set_auto_resolve_lcn_conflict(TRUE);
        result = 0;  //choose pProg1.
    }
    else
        result = popup_result;;

    destroy_lcn_conflict_list_buffer(p_list);

    return result;

}

static void confirm_delete_service()
{
    UINT16 i;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;

    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(param = pn->p_list; NULL != param; param = param->p_next)
        {
            if(param->ctrl.delete_from_database)
            {

                if(prompt_program_deleted(param) == FALSE)
      
                {
                    param->ctrl.delete_from_database = 0;
                }
                #if 0
                else
                {
                    er = del_prog_at(param->n_channel);
                   // DBG_ASSERT(SUCCESS == er);
                }
                #endif
            }
            else if(param->ctrl.delete)
            {

               if(prompt_program_deleted(param) == FALSE)

                {
                    param->ctrl.delete = 0;
                }
               #if 0
                else
                {
                    er = del_prog_at(param->n_channel);
                    //DBG_ASSERT(SUCCESS == er);
                }
                #endif
            }
        }
    }
}

static PPARAM_NODE check_lcn_conflict_with_old_program(const P_NODE *p_prog, const PPARAM_NODE my_self)
{
    UINT16 i;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param;

    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(p_param = pn->p_list; NULL != p_param; p_param = p_param->p_next)
        {
            if(p_param == my_self)
                continue;
    #ifndef SUPPORT_LCN_UNIQUE
            if(p_param->av_flag != p_prog->av_flag)
                continue;
    #endif
            if (p_param->LCN == P_SEARCH_LCN_INVALID   \
                || p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database)
                continue;
            if (p_param->LCN == p_prog->LCN)
                return p_param;
        }
    }
    return NULL;
}

static INT8 auto_resolve_lcn_conflict_by_current_region(const P_NODE *p_prog1, const P_NODE *p_prog2)
{
    //Z-Book: The program belongs to the current region shall hold the LCN,
    //another program belongs to the other region shall be arrange to the Main Overflow zone.
    REGION_OF_NETWORK_ID region1, region2;
    region1 = get_region_by_network_id(p_prog1);
    region2 = get_region_by_network_id(p_prog2);
    if ( region1 != region2 )
    {
        if ( region1 == m_cur_region_network_id )
            return 0;   //auto choose pProg1.
        if ( region2 == m_cur_region_network_id )
            return 1;   //auto choose pProg2.
    }
    return -1;
}

static PTR_PROGRAM_UPDATE check_lcn_conflict_with_new_program(const P_NODE *p_prog, const PTR_PROGRAM_UPDATE my_self)
{
    UINT16 idx;
    PROGRAM_UPDATE *p_prog_upg;

    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);

        if(p_prog_upg == my_self)
            continue;
#ifndef SUPPORT_LCN_UNIQUE
        if(p_prog_upg->p_node.av_flag != p_prog->av_flag)
            continue;
#endif
        if (p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID   \
            || p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == p_prog->LCN)
            return p_prog_upg;
    }
    return NULL;
}

static void resolve_lcn_conflict()
{
    INT8 choose = -1;
    UINT16 i;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param, p_conflict_with_old_program;
    PTR_PROGRAM_UPDATE p_conflict_with_new_program;
    P_NODE prog, prog2;

    UINT16 idx;
    PROGRAM_UPDATE *p_prog_upg;

    //check for all old programs that need to update LCN.
    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(p_param = pn->p_list; NULL != p_param; p_param = p_param->p_next)
        {
            if (p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database \
                || ! p_param->ctrl.update_lcn \
               )
                continue;
            if (p_param->LCN == P_SEARCH_LCN_INVALID)
            {
                p_param->ctrl.rearrange_lcn = 1;
                p_param->ctrl.update_lcn = 1;
                continue;
            }

            param_node_to_p_node(&prog, p_param);
            p_conflict_with_old_program = check_lcn_conflict_with_old_program(&prog, p_param);
            if(NULL != p_conflict_with_old_program)
            {
                param_node_to_p_node(&prog2, p_conflict_with_old_program);
                choose = auto_resolve_lcn_conflict_by_current_region(&prog, &prog2);
                if (choose < 0)
                    choose = prompt_resolve_lcn_conflict(&prog, &prog2);
                else
                {
                    if(choose == 0)
                        p_conflict_with_old_program->LCN = P_SEARCH_LCN_INVALID;
                    else
                        p_param->LCN = P_SEARCH_LCN_INVALID;
                }

                if(choose == 0)
                {
                    p_conflict_with_old_program->ctrl.rearrange_lcn = 1;
                    p_conflict_with_old_program->ctrl.update_lcn = 1;
                }
                else
                {
                    p_param->ctrl.rearrange_lcn = 1;
                }
                continue;
            }

            p_conflict_with_new_program = check_lcn_conflict_with_new_program(&prog, NULL);
            if(NULL != p_conflict_with_new_program)
            {
                choose = auto_resolve_lcn_conflict_by_current_region(&prog, &p_conflict_with_new_program->p_node);
                if (choose < 0)
                    choose = prompt_resolve_lcn_conflict(&prog, &p_conflict_with_new_program->p_node);
                else
                {
                    if(choose == 0)
                        p_conflict_with_new_program->p_node.LCN = P_SEARCH_LCN_INVALID;
                    else
                        p_param->LCN = P_SEARCH_LCN_INVALID;
                }

                if(choose == 0)
                {
                    p_conflict_with_new_program->ctrl.rearrange_lcn = 1;
                }
                else
                {
                    p_param->ctrl.rearrange_lcn = 1;
                }
                continue;
            }
        }
    }


    //check for all new programs that need to be appended or successor program.
    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);

        if ( p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID)
        {
            p_prog_upg->ctrl.rearrange_lcn = 1;
            continue;
        }

        p_conflict_with_old_program = check_lcn_conflict_with_old_program(&p_prog_upg->p_node, NULL);
        if(NULL != p_conflict_with_old_program)
        {
            param_node_to_p_node(&prog2, p_conflict_with_old_program);
            choose = auto_resolve_lcn_conflict_by_current_region(&prog2, &p_prog_upg->p_node);
            if (choose < 0)
                choose = prompt_resolve_lcn_conflict(&prog2, &p_prog_upg->p_node);
            else
            {
                if(choose == 0)
                    p_prog_upg->p_node.LCN = P_SEARCH_LCN_INVALID;
                else
                    p_conflict_with_old_program->LCN = P_SEARCH_LCN_INVALID;
            }

            if(choose == 0)
            {
                p_prog_upg->ctrl.rearrange_lcn = 1;
            }
            else
            {
                p_conflict_with_old_program->ctrl.rearrange_lcn = 1;
                p_conflict_with_old_program->ctrl.update_lcn = 1;
            }
            continue;   //check next program.
        }

        p_conflict_with_new_program = check_lcn_conflict_with_new_program(&p_prog_upg->p_node, p_prog_upg);
        if(NULL != p_conflict_with_new_program)
        {
            choose = auto_resolve_lcn_conflict_by_current_region(&p_prog_upg->p_node, &p_conflict_with_new_program->p_node);
            if (choose < 0)
                choose = prompt_resolve_lcn_conflict(&p_prog_upg->p_node, &p_conflict_with_new_program->p_node);
            else
            {
                if(choose == 0)
                    p_conflict_with_new_program->p_node.LCN = P_SEARCH_LCN_INVALID;
                else
                    p_prog_upg->p_node.LCN = P_SEARCH_LCN_INVALID;
            }

            if(choose == 0)
            {
                p_conflict_with_new_program->ctrl.rearrange_lcn = 1;
            }
            else
            {
                p_prog_upg->ctrl.rearrange_lcn = 1;
                p_prog_upg->ctrl.update_lcn = 1;
            }
            continue;   //check next program.
        }
    }

}


static UINT16 find_free_lcn(UINT8 av_flag, UINT16 base)
{
    UINT16 i, idx;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param;
    PROGRAM_UPDATE *p_prog_upg;
    BOOL restart;

    for(i=0; i<m_p_find->n_count; i=(restart? 0:i+1) )
    {
        restart = FALSE;
        pn = m_p_find->p_node_index[i];
        for(p_param=pn->p_list; NULL!=p_param; p_param = p_param->p_next)
        {
            if (
        #ifndef SUPPORT_LCN_UNIQUE
          p_param->av_flag != av_flag ||
         #endif
                p_param->LCN == P_SEARCH_LCN_INVALID   \
                || p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database)
                continue;
            if (p_param->LCN == base)
            {
                ++base;
                restart = TRUE;
                break;
            }
        }
    }

    for (idx = 0; idx < m_scanning_found.n_count; idx=(restart? 0:idx+1) )
    {
        restart = FALSE;
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (
    #ifndef SUPPORT_LCN_UNIQUE
        p_prog_upg->p_node.av_flag != av_flag ||
    #endif
            p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID   \
            || p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == base)
        {
            ++base;
            restart = TRUE;
        }
    }
    return base;
}

static UINT16 find_new_free_lcn(UINT8 av_flag, UINT16 base)
{
    UINT16 i, idx;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param;
    PROGRAM_UPDATE *p_prog_upg;
    BOOL restart;

    if(base == 0)
        return 0;
    for(i=0; i<m_p_find->n_count; i=(restart? 0:i+1) )
    {
        restart = FALSE;
        pn = m_p_find->p_node_index[i];
        for(p_param=pn->p_list; NULL!=p_param; p_param = p_param->p_next)
        {
            if (
    #ifndef SUPPORT_LCN_UNIQUE
         p_param->av_flag != av_flag ||
    #endif
                p_param->LCN == P_SEARCH_LCN_INVALID   \
                || p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database)
                continue;
            if (p_param->LCN == base)
            {
                --base;
                restart = TRUE;
                break;
            }
        }
    }

    for (idx = 0; idx < m_scanning_found.n_count; idx=(restart? 0:idx+1) )
    {
        restart = FALSE;
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (
    #ifndef SUPPORT_LCN_UNIQUE
         p_prog_upg->p_node.av_flag != av_flag ||
    #endif
            p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID   \
            || p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == base)
        {
            --base;
            restart = TRUE;
        }
    }
    return base;
}

static UINT16 rearrange_lcn_by_lcn_zone(UINT16 lcn, PTR_LCN_ZONE p_lcn_zone, UINT8 av_flag)
{
//   For Italian LCN:
//   a. LCN from 75 to 89 (Preference Overflow zone) are reserved for conflicting channels with LCN is 10~19 or 50~74.
//      If LCN 89 had been occupied too, then assign the LCN from 850 to 999.
//   b. LCN from 850 to 999 (Main Overflow zone) are reserved for conflicting channels with 89<LCN<850 or no LCN, or LCN=0.

    UINT16 free_lcn;

#if 0

    free_lcn = find_free_lcn(av_flag, p_lcn_zone->available_main_overflow_lcn);
    p_lcn_zone->available_main_overflow_lcn = free_lcn + 1;

    if(p_lcn_zone->available_main_overflow_lcn>1000)
    {
            free_lcn = find_new_free_lcn(av_flag, p_lcn_zone->new_overflow_lcn);
            p_lcn_zone->new_overflow_lcn= free_lcn-1;
    }

    return free_lcn;

#else

    if ((lcn == 0 \
        || lcn > p_lcn_zone->preference_overflow_zone_max \
        || p_lcn_zone->available_preference_overflow_lcn > p_lcn_zone->preference_overflow_zone_max)
        && p_lcn_zone->available_main_overflow_lcn<1000)
    {
        free_lcn = find_free_lcn(av_flag, p_lcn_zone->available_main_overflow_lcn);
        p_lcn_zone->available_main_overflow_lcn = free_lcn + 1;
     if(free_lcn >=1000)
         goto OVERFLOW_FULL;
     else
            return free_lcn;
    }

    if (  (lcn >= p_lcn_zone->preference_zone_1_min && lcn <= p_lcn_zone->preference_zone_1_max) \
        ||(lcn >= p_lcn_zone->preference_zone_2_min && lcn <= p_lcn_zone->preference_zone_2_max) )

    {
        free_lcn = find_free_lcn(av_flag, p_lcn_zone->available_preference_overflow_lcn);
        if (free_lcn <= p_lcn_zone->preference_overflow_zone_max)
        {
            p_lcn_zone->available_preference_overflow_lcn = free_lcn + 1;
            return free_lcn;
        }
        else    //make available_preference_overflow_lcn become full.
            p_lcn_zone->available_preference_overflow_lcn = p_lcn_zone->preference_overflow_zone_max + 1;
    }

    //if available_preference_overflow_lcn is full,
    //or the conflict lcn is not belong to the preerence zone,
    //then assign a lcn from available_main_overflow_lcn.
    free_lcn = find_free_lcn(av_flag, p_lcn_zone->available_main_overflow_lcn);
    p_lcn_zone->available_main_overflow_lcn = free_lcn + 1;


    //the main overflow occupies service numbers 850 to 999. In case Main Overflow space would get filled up,
    //free positions from 849 backwards SHALL be used.
    if(p_lcn_zone->available_main_overflow_lcn>1000)
    {
OVERFLOW_FULL:
            free_lcn = find_new_free_lcn(av_flag, p_lcn_zone->new_overflow_lcn);
            p_lcn_zone->new_overflow_lcn= free_lcn-1;
    }

    return free_lcn;

 #endif
}

static void rearrange_lcn_by_region(UINT8 av_flag)
{
    const UINT16 *p_region = get_region_table();
    UINT8 region = sys_data_get_country();
    UINT16 base, idx, i;
    UINT8 chan_mode, lcn_mode;
    INT32 er;
    P_NODE p_node;
    LCN_ZONE lcn_zone;

    PROGRAM_UPDATE *p_prog_upg;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;

    if(RS_REGION_AUS == p_region[region])
        base = 350;
    else if(RS_REGION_UK == p_region[region])
        base = 800;
    else if(RS_REGION_ITA== p_region[region])
    {
        lcn_zone.preference_zone_1_min= 10;
        lcn_zone.preference_zone_1_max= 19;
        lcn_zone.preference_zone_2_min= 50;
        lcn_zone.preference_zone_2_max= 74;
        lcn_zone.available_preference_overflow_lcn = 75;
        lcn_zone.preference_overflow_zone_max = 89;
        lcn_zone.available_main_overflow_lcn = 850;
        lcn_zone.new_overflow_lcn = 849;
        base = lcn_zone.available_main_overflow_lcn;
    }
    else
        base = 1000;

   //rearrange LCN of old programs stored in database that need to update LCN.
    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(param=pn->p_list; NULL!=param; param = param->p_next)
        {
            if (   param->av_flag != av_flag \
                || ! param->ctrl.rearrange_lcn   \
                || param->deleted \
                || param->ctrl.delete  \
                || param->ctrl.successor_service \
                || param->ctrl.delete_from_database)
                continue;

            if (RS_REGION_ITA == p_region[region])
            {
                    param_node_to_p_node(&p_node, param);
//                if ( GetRegionByNetworkID(&p_node) != m_cur_region_network_id )
//                {
//                    param->LCN = P_SEARCH_LCN_INVALID;
//                }
                param->LCN = rearrange_lcn_by_lcn_zone(param->LCN, &lcn_zone, av_flag);
            }
            else
            {
                base = find_free_lcn(av_flag, base);
                param->LCN = base;
                ++base;
            }
            param->lcn_true = 0;
            param->ctrl.update_lcn = 1;
            param->ctrl.rearrange_lcn = 0;
        }
    }

    //rearrange LCN of all new programs.
    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (   p_prog_upg->p_node.av_flag != av_flag \
            || ! p_prog_upg->ctrl.rearrange_lcn   \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;

        if (RS_REGION_ITA == p_region[region])
        {
//            if ( GetRegionByNetworkID(&pProgUpg->p_node) != m_cur_region_network_id )
//            {
//                pProgUpg->p_node.LCN = P_SEARCH_LCN_INVALID;
//            }
            p_prog_upg->p_node.LCN = rearrange_lcn_by_lcn_zone(p_prog_upg->p_node.LCN, &lcn_zone, av_flag);
        }
        else
        {
            base = find_free_lcn(av_flag, base);
            p_prog_upg->p_node.LCN = base;
            ++base;
        }
        p_prog_upg->p_node.lcn_true = 0;
        p_prog_upg->ctrl.rearrange_lcn = 0;
    }
}

static void rearrange_lcn_by_default(UINT8 av_flag)
{
    UINT16    base;
    UINT32  idx, i;

    PROGRAM_UPDATE *p_prog_upg;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;

    base = 1;

    //rearrange LCN of old programs stored in database that need to update LCN.
    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(param=pn->p_list; NULL!=param; param = param->p_next)
        {
            if (   param->av_flag != av_flag \
                || ! param->ctrl.rearrange_lcn   \
                || param->deleted \
                || param->ctrl.delete  \
                || param->ctrl.successor_service \
                || param->ctrl.delete_from_database)
                continue;

            base = find_free_lcn(av_flag, base);
            param->LCN = base;
            param->lcn_true = 0;
            param->ctrl.update_lcn = 1;
            param->ctrl.rearrange_lcn = 0;
            ++base;
        }
    }

    //rearrange LCN of all new programs.
    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (   p_prog_upg->p_node.av_flag != av_flag \
            || ! p_prog_upg->ctrl.rearrange_lcn   \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        base = find_free_lcn(av_flag, base);
        p_prog_upg->p_node.LCN = base;
        p_prog_upg->p_node.lcn_true = 0;
        p_prog_upg->ctrl.rearrange_lcn = 0;
        ++base;
    }
}

void rearrange_lcn()
{
    if(1)
    {
        rearrange_lcn_by_region(PROG_TV_MODE);
        rearrange_lcn_by_region(PROG_RADIO_MODE);
    }
    else
    {
        rearrange_lcn_by_default(PROG_TV_MODE);
        rearrange_lcn_by_default(PROG_RADIO_MODE);
    }
}


static PWSTR *create_lcn_conflict_list_buffer(UINT8 count)
{
    PWSTR *p_list = NULL;
    UINT8 *p_buffer ;
    int size, i;

    size = sizeof(PWSTR)*count + sizeof(WCHAR)*C_CONFLICT_STRING_LENGTH*count;
    p_buffer = (UINT8 *)t2_malloc(size);
    if(NULL == p_buffer)
        return NULL;

    p_list = (PWSTR *)p_buffer;
    for(i=0; i<count; i++)
    {
        size = sizeof(PWSTR)*count + sizeof(WCHAR)*C_CONFLICT_STRING_LENGTH*i;
        p_list[i] = (PWSTR)(&p_buffer[size]);
    }

    return p_list;
}

static void destroy_lcn_conflict_list_buffer(PWSTR *p_list)
{
    if(p_list != NULL)
        t2_free((PVOID)p_list);
}


static void create_lcn_conflict_string(const P_NODE *p_prog, PWCHAR p_buffer, UINT32 n_size)
{
    UINT32 len=0;
    char buf[256];
    T_NODE t_node;

    get_tp_by_id(p_prog->tp_id, &t_node);
    snprintf(buf, 256,"LCN %d ", p_prog->LCN);
    len = com_asc_str2uni(buf, p_buffer);
    len += com_uni_str_copy_char_n((UINT8*)(&p_buffer[len]), osd_get_unicode_string(RS_ASSIGN_TO), n_size-len);
    snprintf(buf, 256," %d.%03dMHz ", t_node.frq/1000, t_node.frq%1000);
    len += com_asc_str2uni(buf, &p_buffer[len]);
    ASSERT(len < n_size);
    len += com_uni_str_copy_char_n((UINT8*)&p_buffer[len], (UINT8*)p_prog->service_name, n_size-len);

}


void set_auto_resolve_lcn_conflict(BOOL enable)
{
    m_auto_resolve_lcn_conflict = enable;
}

void set_prompt_mode(BOOL enable)
{
    m_prompt_mode = enable;
}

static void mark_deleted_service()
{
    UINT16 i, j;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;
    P_NODE p_node;

    //Check old progams stayed at these normal TP whether still exist in the TS.
    for(j=0; j<m_normal_tplist.n_count; j++)
    {
        for(i=0; i<m_p_find->n_count; i++)
        {
            pn = m_p_find->p_node_index[i];
            for(param = pn->p_list; param!=NULL; param = param->p_next)
            {
                if( m_normal_tplist.tp_list[j] == param->tp_id   \
                    && !param->deleted )
                {
                    param_node_to_p_node(&p_node, param);
                    if (post_processing_get_prog_idx(&p_node) == INVALID_ID)
                    {
                        param->ctrl.delete = TRUE;
                    }
                }
            }
        }
    }
}

static PROGRAM_COMPARING_RESULT post_processing_find_node(P_NODE *p_node, INT16 *p_pos, PPARAM_NODE *pp_param)
{
    INT16 i = 0, min = 0, max = m_p_find->n_count, pos = 0;
    int cmp = -1;
    T_NODE t_node;
    PROGRAM_COMPARING_RESULT result = PROG_DIFF_AT_ALL;
    PPROGRAM_NODE p_node;
    PPARAM_NODE p_param = NULL;

    get_tp_by_id(p_node->tp_id, &t_node);
    result = PROG_DIFF_AT_ALL;
    min = 0;
    max = m_p_find->n_count;
    cmp = -1;
    while(min < max)
    {
        i = (min+max)/2;
        p_node = m_p_find->p_node_index[i];
        cmp = compare_key(t_node.network_id, p_node->orig_network_id);
        if(!cmp)
        {
            pos = i;
            for (p_param = p_node->p_list; NULL != p_param; p_param = p_param->p_next)
            {
                //|| pParam->deleted
                if ( p_param->ctrl.delete_from_database \
                    || p_param->av_flag != p_node->av_flag)
                    continue;
                if(p_param->prog_number == p_node->prog_number && p_param->ts_id == t_node.t_s_id)
                {
                    if (p_node->tp_id == p_param->tp_id)
                    {
                        result = PROG_SAME;
                        if(pp_param)
                            *pp_param = p_param;
                        break;
                    }
                    result = PROG_SAME_BUT_DIFF_TP; //it need to continue to find whether exist PROG_SAME later.
                }
            }
            break;
        }
        else if(cmp > 0)
            pos = min = i + 1;
        else
            pos = max = i;
    }
    if(p_pos)
        *p_pos = pos;
    return result;
}

static void mark_updated_and_appended_service()
{
    UINT16 idx;
    PROGRAM_UPDATE *p_prog_upg;
    PROGRAM_COMPARING_RESULT result = PROG_DIFF_AT_ALL;
    PPARAM_NODE p_param = NULL;

    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        result = post_processing_find_node(&p_prog_upg->p_node, NULL, &p_param);

    if ( PROG_SAME_BUT_DIFF_TP == result )
    {
            if ( p_prog_upg->p_node.orig_lcn != 0 )    //do not append program that if its lcn==0.
                p_prog_upg->ctrl.append = 1;  //it will be appended into channel list.
    }
        else if(PROG_SAME == result)
        {
            ASSERT(p_param);
            if ( p_param->deleted )
            {
                if ( p_prog_upg->p_node.orig_lcn == 0 )
                    p_param->ctrl.delete_from_database = 1;
                continue;  //Don't reintroduce the deleted program.
            }
            if ( p_prog_upg->p_node.orig_lcn == 0 )  //(Unclaimed in Z-Book),really delete this program from database.
            {
                p_param->ctrl.delete_from_database = 1;
                continue;
            }

            if ( ! p_param->user_changed_service_name )  //user had never modified the name.
            {
                if (com_uni_str_cmp((PWSTR)(p_prog_upg->p_node.service_name), p_param->sz_name) != 0)
                {
                    com_uni_str_copy_char_n((UINT8*)p_param->sz_name, p_prog_upg->p_node.service_name,  MAX_SERVICE_NAME_LENGTH);
                    p_param->ctrl.update_service_name = 1;
                }
            }

//            if ( ! pParam->user_changed_lcn && pProgUpg->p_node.lcn_true )  //user had never modified the LCN, and the new LCN is real LCN.
            if ( ! p_param->user_changed_lcn )  //user had never modified the LCN.
            {
                //do not check (pParam->LCN != pProgUpg->p_node.LCN || pParam->lcn_true != pProgUpg->p_node.lcn_true )
                if ( p_param->orig_lcn != p_prog_upg->p_node.orig_lcn )
                {
                    p_param->orig_lcn = p_prog_upg->p_node.orig_lcn;
                    p_param->LCN = p_prog_upg->p_node.LCN;
                    p_param->lcn_true = p_prog_upg->p_node.lcn_true;
              p_param->ctrl.update_lcn = 1;
                }
            }

            if ( p_param->hidden_flag != p_prog_upg->p_node.hidden_flag )
            {
                     p_param->hidden_flag = p_prog_upg->p_node.hidden_flag;
                p_param->ctrl.update_hidden_flag = 1;
            }
        }
        else    //PROG_DIFF_AT_ALL
        {
                if ( p_prog_upg->p_node.orig_lcn != 0 )    //do not append program that if its lcn==0.
                {
                    p_prog_upg->ctrl.append = 1;  //it will be appended into channel list.
                }
        }
    }
}

static void mark_successor_service()
{
    UINT16 idx, i;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;
    PROGRAM_UPDATE *p_prog_upg;

    T_NODE t_node1, t_node2;
    PPARAM_NODE deleted_service, deleted_service_of_other_network, lcn_conflict_service;
    PPARAM_NODE target;
    BOOL user_changed_lcn;

    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if ( ! p_prog_upg->ctrl.append   \
            || ! p_prog_upg->p_node.lcn_true \
            || p_prog_upg->p_node.orig_lcn == P_SEARCH_LCN_INVALID \
           )
           continue;

        deleted_service = NULL;
        deleted_service_of_other_network = NULL;
        lcn_conflict_service = NULL;
        user_changed_lcn = FALSE;

        for(i=0; i<m_p_find->n_count; i++)
        {
            pn = m_p_find->p_node_index[i];
            for(param = pn->p_list; NULL != param; param = param->p_next)
            {
                if( p_prog_upg->p_node.av_flag == param->av_flag \
                    && ! param->ctrl.delete_from_database \
                    && ! param->ctrl.successor_service \
                    && param->orig_lcn == p_prog_upg->p_node.orig_lcn )
//                        ||param->LCN == pProgUpg->p_node.LCN)
                {
                    if( param->deleted || param->ctrl.delete || param->ctrl.delete_from_database )    //check successor
                    {
                        get_tp_by_id(p_prog_upg->p_node.tp_id, &t_node1);
                        get_tp_by_id(param->tp_id, &t_node2);
                        if( param->orig_lcn == p_prog_upg->p_node.orig_lcn \
                            && t_node2.network_id == t_node1.network_id \
                            && t_node2.net_id == t_node1.net_id )
                        {
                            if (deleted_service == NULL)
                                deleted_service = param;
                        }
                        else if( param->orig_lcn == p_prog_upg->p_node.orig_lcn \
                            && t_node2.network_id == t_node1.network_id )
                        {
                            if (deleted_service_of_other_network == NULL)
                                deleted_service_of_other_network = param;
                        }
                        continue;
                    }

                    if (lcn_conflict_service == NULL)
                        lcn_conflict_service = param;
                    if ( param->user_changed_lcn )
                        user_changed_lcn = TRUE;
                }
            }
        }

        target = NULL;
        if (deleted_service != NULL)
            target = deleted_service;
        else if (deleted_service_of_other_network != NULL)
            target = deleted_service_of_other_network;

        if (target != NULL)
        {
                    //pProgUpg->associated_prog_id = Target->nChannel;
                    p_prog_upg->associated_prog_id = target->prog_id;
            p_prog_upg->ctrl.successor_service = 1;
            p_prog_upg->ctrl.append = 0;
            p_prog_upg->p_node.default_index = target->current_index;

            target->ctrl.successor_service = 1;
            target->ctrl.delete = 0;
            target->ctrl.delete_from_database = 0;

            if (lcn_conflict_service != NULL)
            {
                //If the LCN is occupied, it shall be allocated to the Main Overflow.(7.3.4.6.3)
                p_prog_upg->p_node.LCN = P_SEARCH_LCN_INVALID;
                p_prog_upg->p_node.lcn_true = 0;
            }
        }
        else if (lcn_conflict_service != NULL && user_changed_lcn)
        {
            //If the LCN was occupied by a user modified service,
            //then new conflict service shall be allocated to the Main Overflow.
            p_prog_upg->p_node.LCN = P_SEARCH_LCN_INVALID;
            p_prog_upg->p_node.lcn_true = 0;
        }
    }
}

static void param_node_to_p_node(P_NODE *p_node, const PPARAM_NODE p_param)
{
    ASSERT(p_node);
    ASSERT(p_param);

    p_node->tp_id = p_param->tp_id;
    p_node->prog_number = p_param->prog_number;

    p_node->lcn_true = p_param->lcn_true;
    p_node->LCN = p_param->LCN;
    p_node->orig_lcn = p_param->orig_lcn;

    p_node->av_flag = p_param->av_flag;
    p_node->ca_mode = p_param->ca_mode;
    p_node->default_index = p_param->current_index;

    p_node->hidden_flag = p_param->hidden_flag;
    p_node->user_changed_service_name = p_param->user_changed_service_name;
    p_node->user_changed_lcn = p_param->user_changed_lcn;
    p_node->deleted = p_param->deleted;

    p_node->prog_id = p_param->prog_id;

    com_uni_str_copy_char_n(p_node->service_name, (UINT8*)p_param->sz_name, MAX_SERVICE_NAME_LENGTH);
}


static BOOL update_for_old_program(UINT8 av_flag)
{
    UINT32  idx, i;
    PROGRAM_UPDATE *p_prog_upg;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;
    P_NODE p_node;
    BOOL result_update = FALSE, update = FALSE;

    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(param=pn->p_list; NULL!=param; param = param->p_next)
        {
            if (param->av_flag != av_flag \
                ||!(param->ctrl.delete \
                    || param->ctrl.update_hidden_flag  \
                    || param->ctrl.update_service_name \
                    || param->ctrl.update_lcn)
               )
               continue;

            if(SUCCESS != get_prog_at(param->n_channel, &p_node))
               continue;
            update = FALSE;
            if( param->ctrl.delete && p_node.deleted != 1)
            {
                p_node.deleted = 1;
                update = TRUE;
                //CHN_CleanUpForDeletedProgram(param->nChannel, param->av_flag, FALSE); Ming
            }
            if( param->ctrl.update_hidden_flag && p_node.hidden_flag != param->hidden_flag)
            {
                p_node.hidden_flag = param->hidden_flag;
                update = TRUE;
            }
            if( param->ctrl.update_service_name \
                && com_uni_str_cmp((PWSTR)(p_node.service_name), param->sz_name) != 0 )
            {
                com_uni_str_copy_char_n(p_node.service_name, (UINT8*)param->sz_name, MAX_SERVICE_NAME_LENGTH);
                update = TRUE;
            }
            if( param->ctrl.update_lcn \
                && ( p_node.LCN != param->LCN \
                   ||p_node.lcn_true != param->lcn_true \
                   ||p_node.orig_lcn != param->orig_lcn ) \
              )
            {
                p_node.LCN = param->LCN;
                p_node.lcn_true = param->lcn_true;
                p_node.orig_lcn = param->orig_lcn;
                update = TRUE;
            }

            if(update)
            {
                modify_prog(param->prog_id, &p_node);
                result_update = TRUE;
            }
        }
    }

    //Replaced the deleted old program with the new successor program.
    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (   p_prog_upg->p_node.av_flag != av_flag \
            || ! p_prog_upg->ctrl.successor_service  )
            continue;
        modify_prog(p_prog_upg->associated_prog_id, &p_prog_upg->p_node);
        result_update = TRUE;
    }
    return result_update;
}

static BOOL delete_program_from_database(UINT8 av_flag)
{
    UINT32  idx, i;
    PROGRAM_UPDATE *p_prog_upg;
    PPROGRAM_NODE pn;
    PPARAM_NODE param;
    P_NODE p_node;
    BOOL result_update = FALSE;
    INT32 er;

    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(param=pn->p_list; NULL!=param; param = param->p_next)
        {
            if (param->av_flag != av_flag \
                || !(param->ctrl.delete_from_database||param->ctrl.delete))
               continue;
            else
           {
                er=del_prog_by_id(param->prog_id);
                DBG_ASSERT(SUCCESS == er);
                result_update = TRUE;
            }
        }
    }
    return result_update;
}

static BOOL append_new_program(UINT8 av_flag)
{
       INT32 result;
    UINT32  idx;
    PROGRAM_UPDATE *p_prog_upg;
    P_NODE p_node;
    BOOL result_update = FALSE;
    POPUP_YESNO_PARAM param;
    UINT8 mode;

    //mode  = (PROG_VIDEO_MODE == av_flag) ? PROG_VIDEO_MODE : PROG_AUDIO_MODE;
    //UINT32 total = get_prog_num(VIEW_ALL|mode,0);
    UINT32 total = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE,0);

    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);
        if (   p_prog_upg->p_node.av_flag != av_flag \
            || ! p_prog_upg->ctrl.append)
            continue;

        ++total;
        if (total >= SYS_MAX_NUM_PROGRAM)
        {
            if ( m_prompt_mode )
            {
                ap_close_popup();
              param.str_id = RS_MSG_SPACE_FULL;
                   ap_open_popup(POPUP_MSG, (UINT32)&param); //IDW_SEARCH
            }
            break;
        }

//        asearchmenu_inc_prog_count(&pProgUpg->p_node);
        if(p_prog_upg->p_node.av_flag)
            p_prog_upg->p_node.default_index = m_cur_tvcount++;
        else
            p_prog_upg->p_node.default_index = m_cur_radio_count++;

        MEMCPY((UINT8*)&p_node, (UINT8*)&p_prog_upg->p_node, sizeof(P_NODE));
        result = add_node(TYPE_PROG_NODE, p_node.tp_id,&p_node);
     if (result == STORE_SPACE_FULL)
        {
            if ( m_prompt_mode )
            {
                ap_close_popup();
             param.str_id = RS_MSG_SPACE_FULL;
                   ap_open_popup(POPUP_MSG, (UINT32)&param); //IDW_SEARCH
            }
            break;
    }
        result_update = TRUE;
    }
    return result_update;
}

static BOOL update_to_database()
{
    BOOL result_update = FALSE, update = FALSE;
    UINT8 av_flag, temp_av_flag;
    UINT16 n_prog_pos;

    DBG_PRINTF("---------------------step 3: before update to database: Old program---------------------\n");
    print_node_table();
    DBG_PRINTF("---------------------step 3: before update to database: New found program---------------------\n");
    print_scanning_found();
    DBG_PRINTF("\n");

    //update for another mode.
    av_flag = sys_data_get_cur_chan_mode();
    update = update_for_old_program(av_flag);
    if (update)
           update_data();
    result_update |= update;

    update = delete_program_from_database(av_flag);
    if (update)
           update_data();
    result_update |= update;

    update = append_new_program(av_flag);
    if (update)
        update_data();
    result_update |= update;

    //update for another mode.
    temp_av_flag = (PROG_TV_MODE == av_flag) ? PROG_RADIO_MODE : PROG_TV_MODE;
    update = update_for_old_program(temp_av_flag);
    if (update)
        update_data();
    result_update |= update;

    update = delete_program_from_database(temp_av_flag);
    if (update)
        update_data();
    result_update |= update;

    update = append_new_program(temp_av_flag);

    if (update)
        update_data();
    result_update |= update;


    if (result_update)
    {
        //CHN_Reset();
        //nProgPos = CHN_ArrangeAll(sys_data_get_cur_channel());
        //sys_data_set_cur_channel(nProgPos);
    }

    DBG_PRINTF("---------------------step 4: update database is finish:---------------------\n");
    print_db();
    DBG_PRINTF("\n");

    return result_update;
}

static save_program_with_better_signal()
{
    UINT16 i=0, j=0,n=0;
    PROGRAM_UPDATE *p_prog_upg, *p_prog_upg2,*p_progsave,*p_progsave2;
    UINT32 q1, q2,i1,i2;

    for (i = 0; i < m_scanning_found.n_count; i++)
        {
            p_prog_upg = m_scanning_found.program_list[i];

            for (j=0; j < n; j++)
            {
                p_prog_upg2 = m_save.program_list[j];

                if (compare_program(&p_prog_upg->p_node, &p_prog_upg2->p_node) == PROG_SAME)
              {

                i1 = get_signal_intensity(p_prog_upg->p_node.tp_id);
                i2 = get_signal_intensity(p_prog_upg2->p_node.tp_id);

                T_NODE tp1,tp2;

                get_tp_by_id(p_prog_upg->p_node.tp_id, &tp1);
                get_tp_by_id(p_prog_upg2->p_node.tp_id, &tp2);
                //libc_printf("i1 is %d,i2 is %d,fre1 is %d,fre2 is %d\n",i1,i2,tp1.frq,tp2.frq);
                if(compare_signal(i1, i2)> 0)
                 {
                 m_save.program_list[j]=m_scanning_found.program_list[i];

                 }
                break;
             }

           }
                if(j==n)
                {
                m_save.program_list[n]=m_scanning_found.program_list[i];
                 n++;
                 m_save.n_count++;
                 }
     }
            m_scanning_found.n_count=m_save.n_count;
            for(i=0;i<m_save.n_count;i++)
            {
            MEMCPY(m_scanning_found.program_list[i],m_save.program_list[i],sizeof(PROGRAM_UPDATE));
            }

}

BOOL post_processing_save()
{

    BOOL result_update = FALSE;
    POPUP_YESNO_PARAM param;

    DBG_PRINTF("---------------------step 2: scanning found:---------------------\n");
    print_scanning_found();

    do
    {

        mark_deleted_service();
        save_program_with_better_signal();

        mark_updated_and_appended_service();
        mark_successor_service();
        confirm_delete_service();
#ifdef LIST_ALL_CONFLICT
        resolve_lcn_conflict_ext();
#else
        resolve_lcn_conflict();
#endif
        rearrange_lcn();
        result_update = update_to_database();

        if ( m_prompt_mode )
        {
            ap_close_popup();
        }
        set_prompt_mode(TRUE);
    }while(0);
    return result_update;
}


UINT16 get_restored_lcn(UINT32 progid)
{
    P_NODE p_node;
    PPARAM_NODE p_param = NULL, p_conflict_with_old_program = NULL;
    PROGRAM_COMPARING_RESULT result = PROG_DIFF_AT_ALL;
    UINT16 lcn = P_SEARCH_LCN_INVALID;

    ap_init_node_view_ext(0);
    do{
        //    SetPromptMode(FALSE);

            if(SUCCESS != get_prog_by_id(progid, &p_node))
               break;    //error.
            result = post_processing_find_node(&p_node, NULL, &p_param);
            ASSERT(result == PROG_SAME && p_param != NULL);

            p_conflict_with_old_program = check_lcn_conflict_with_old_program(&p_node, p_param);

            if(NULL == p_conflict_with_old_program)
            {
                lcn = p_node.LCN;
                break;
            }
            p_param->ctrl.rearrange_lcn = 1;
            p_param->deleted = 0;

            if(1)
            {
                rearrange_lcn_by_region(sys_data_get_cur_chan_mode());
            }
            else
            {
                rearrange_lcn_by_default(sys_data_get_cur_chan_mode());
            }

            lcn = p_param->LCN;
    }while(0);

    ap_free_node_view();
    return lcn;
}



INT32 modify_lcn(UINT32 prog_id, UINT32 LCN)
{
    P_NODE p_node;
    INT32 er;

    er = get_prog_by_id(prog_id, &p_node);
    DBG_ASSERT(SUCCESS == er);

    p_node.LCN = LCN;
    if(p_node.LCN == p_node.orig_lcn)
        p_node.user_changed_lcn = 0;
    else
     p_node.user_changed_lcn = 1;

    er = modify_prog(prog_id, &p_node);
    DBG_ASSERT(SUCCESS == er);

    return er;
}


INT32 modify_default_index(UINT32 prog_id, UINT32 default_index)
{
    P_NODE p_node;
    INT32 er;

    er = get_prog_by_id(prog_id, &p_node);
    DBG_ASSERT(SUCCESS == er);

    p_node.default_index= default_index;

    er = modify_prog(prog_id, &p_node);
    DBG_ASSERT(SUCCESS == er);

    return er;
}


INT32 modify_service_name(P_NODE *p_node)
{
    INT32 er;

    p_node->user_changed_service_name = 1;

    er = modify_prog(p_node->prog_id, p_node);
    DBG_ASSERT(SUCCESS == er);

    return er;
}


//====================================================================
#ifdef LIST_ALL_CONFLICT
#define MAX_CONFILIC 10

typedef struct
{
    UINT16 n_count;
    PARAM_NODE  **plist;
}PROGRAM_NEW_LIST;

static PROGRAM_LIST m_conflic_channel;
static PROGRAM_NEW_LIST  m_conflic_channel_old;

static  BOOL conflict_list_iinitalize()
{
    m_conflic_channel.n_count = 0;
    m_conflic_channel.program_list = (PROGRAM_UPDATE **)t2_malloc(MAX_CONFILIC*sizeof(PROGRAM_UPDATE*));
    DBG_ASSERT(m_conflic_channel.program_list);
    if (m_conflic_channel.program_list == NULL)
        return FALSE;
    com_mem_set((UINT8 *)m_conflic_channel.program_list, 0, sizeof(MAX_CONFILIC*sizeof(PROGRAM_UPDATE*)));


    m_conflic_channel_old.n_count = 0;
    m_conflic_channel_old.plist = (PARAM_NODE **)t2_malloc(MAX_CONFILIC*sizeof(PARAM_NODE*));
    DBG_ASSERT(m_conflic_channel_old.plist);
    if (m_conflic_channel_old.plist == NULL)
        return FALSE;
    com_mem_set((UINT8 *)m_conflic_channel_old.plist, 0, sizeof(MAX_CONFILIC*sizeof(PARAM_NODE*)));
}


static PPARAM_NODE check_lcn_conflict_with_old_program_ext(const P_NODE *p_prog, const PPARAM_NODE my_self)
{
    UINT16 i;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param;

    for(i=0; i<m_p_find->n_count; i++)
    {
        pn = m_p_find->p_node_index[i];
        for(p_param = pn->p_list; NULL != p_param; p_param = p_param->p_next)
        {
            if(p_param == my_self)
                continue;
    #ifndef SUPPORT_LCN_UNIQUE
            if(p_param->av_flag != p_prog->av_flag)
                continue;
    #endif
            if (p_param->LCN == P_SEARCH_LCN_INVALID   \
                || p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database)
                continue;
            if(p_param->LCN == p_prog->LCN)
              {
                m_conflic_channel_old.plist[m_conflic_channel_old.n_count]=  p_param;
            m_conflic_channel_old.n_count ++;
                if(m_conflic_channel_old.n_count  > MAX_CONFILIC)
                {
                        return m_conflic_channel_old.plist[0];
                    }
            }
        }
    }
    if(m_conflic_channel_old.n_count>= 1)
    {
            return m_conflic_channel_old.plist[0];
    }
    return NULL;
}

static INT8 auto_resolve_old_prog_lcn_conflict_by_current_region_ext(const P_NODE *p_prog1)
{
    //Z-Book: The program belongs to the current region shall hold the LCN,
    //another program belongs to the other region shall be arrange to the Main Overflow zone.
    P_NODE prog;
    REGION_OF_NETWORK_ID region;
    UINT i=0, flag=0,count=0,backup_i=0;

    region = get_region_by_network_id(p_prog1);
    if(region == m_cur_region_network_id)
        flag=TRUE;

    for(i=0; i<m_conflic_channel_old.n_count ; i++)
    {
        param_node_to_p_node(&prog, m_conflic_channel_old.plist[i]);
            region = get_region_by_network_id(&prog);
            if(region== m_cur_region_network_id)
            {
                if(flag)
                {
                    return -1;// not auto resolve,because two program have the same lcn in current region network id
            }
                backup_i = i;
                count++;
            }
     }

    if(flag) //auto resolve
    {
        for(i=0; i<m_conflic_channel_old.n_count ; i++)
               {
                   m_conflic_channel_old.plist[i]->ctrl.rearrange_lcn = 1;
                   m_conflic_channel_old.plist[i]->LCN = P_SEARCH_LCN_INVALID;
        }
        return 0; // auto resolve

       }
    else
    {
        if(count==1)
        {
            for(i=0; i<m_conflic_channel_old.n_count ; i++)
                   {
                       if(i !=backup_i)
                       {
                           m_conflic_channel_old.plist[i]->ctrl.rearrange_lcn = 1;
                           m_conflic_channel_old.plist[i]->LCN = P_SEARCH_LCN_INVALID;
                       }
                       else
                       {
                           m_conflic_channel_old.plist[i]->ctrl.update_lcn = 1;
                       }
                   }
            return 1; // auto resolve
        }
        else
        {
            return -1;;// not auto resolve,because  the same lcn ch none  in current region network id
        }
    }
}


static INT8 auto_resolve_lcn_conflict_by_current_region_ext(const P_NODE *p_prog1)
{
    REGION_OF_NETWORK_ID region;
    UINT i=0, flag=0,count=0,backup_i=0;


    region = get_region_by_network_id(p_prog1);
    if(region == m_cur_region_network_id)
        flag=TRUE;

    for(i=0; i<m_conflic_channel.n_count ; i++)
    {
            region = get_region_by_network_id(&(m_conflic_channel.program_list[i]->p_node));
            if(region== m_cur_region_network_id)
            {
                if(flag)
                {
                    return -1;// not auto resolve,because two ch have the same lcn in current region network id
                }
                backup_i = i;
                count++;
            }
     }

    if(flag) //auto resolve       only one  is region network id.
    {
        for(i=0; i<m_conflic_channel.n_count ; i++)
               {
                   m_conflic_channel.program_list[i]->ctrl.rearrange_lcn = 1;
                   m_conflic_channel.program_list[i]->p_node.LCN = P_SEARCH_LCN_INVALID;
        }
        return 0; // auto resolve

       }
    else
    {
        if(count==1)
        {
            for(i=0; i<m_conflic_channel.n_count ; i++)
                   {
                       if(i !=backup_i)
                       {
                           m_conflic_channel.program_list[i]->ctrl.rearrange_lcn = 1;
                           m_conflic_channel.program_list[i]->p_node.LCN = P_SEARCH_LCN_INVALID;
                       }
                       else
                       {
                           m_conflic_channel.program_list[i]->ctrl.update_lcn = 1;
                       }
                   }
            if(backup_i ==0)
            {
                return 0x10;         //old programe is in current region.
            }
            else
            {
                return 0x11; // auto resolve  only one is region network id
            }
        }
        else
        {
            return -1;// not auto resolve,because  the same lcn program none  in current region network id
        }
    }


}


UINT8 prompt_resolve_old_prog_lcn_conflict(const P_NODE *p_prog1)
{
    POPUP_LIST_PARAM list_param =
    {
        RS_WHICH_PROGRAM_HOLD_THE_LCN, 0, 0, 0 , NULL, 0
    };
    INT32 popup_result = SUCCESS;
    UINT8 total_option=2+1, result;
    PWSTR *p_list;
    POPUP_YESNO_PARAM param;
    P_NODE prog;
    INT8 i=0;

    if ( !m_prompt_mode || m_auto_resolve_lcn_conflict)
        return 0;   //auto choose pProg1.


    total_option = m_conflic_channel_old.n_count +2;

    ap_close_popup();

    p_list = create_lcn_conflict_list_buffer(total_option);
    ASSERT(p_list != NULL);

    // make popup list string
    create_lcn_conflict_string(p_prog1, p_list[0], C_CONFLICT_STRING_LENGTH);
    for(i = 0; i<( total_option-2); i++)
    {
         param_node_to_p_node(&prog, m_conflic_channel_old.plist[i]);
        create_lcn_conflict_string(&prog, p_list[i+1], C_CONFLICT_STRING_LENGTH);
    }
    com_uni_str_copy_char_n((UINT8*)(p_list[total_option-1]), osd_get_unicode_string(RS_AUTOMATICALLY_WITH_NO_PROMPT), C_CONFLICT_STRING_LENGTH-1);

    // Indicate end-user to handle conflict items
    list_param.p_table = (UINT32 *)p_list;
    list_param.n_count =total_option;
    list_param.def_sel = list_param.sel_stat;
    list_param.b_wait_time = 10;
    popup_result = ap_open_popup(POPUP_LIST, (UINT32)&list_param);

    if( total_option >2 && popup_result == (UINT8)(total_option - 1) )
    {
        set_auto_resolve_lcn_conflict(TRUE);
        result = 0;  //choose pProg1.
    }
    else
        result = popup_result;

    if(result ==0)
    {
                for(i = 0; i<m_conflic_channel_old.n_count; i++)
         {
               m_conflic_channel_old.plist[i]->LCN = P_SEARCH_LCN_INVALID;
                    m_conflic_channel_old.plist[i]->ctrl.rearrange_lcn =1;
         }
    }
    else
    {
            for(i = 0; i<m_conflic_channel_old.n_count; i++)
         {
                 if((result -1) != i)
                 {
                   m_conflic_channel_old.plist[i]->LCN = P_SEARCH_LCN_INVALID;
                        m_conflic_channel_old.plist[i]->ctrl.rearrange_lcn =1;
              }
              else
                  {
                     m_conflic_channel_old.plist[i]->ctrl.update_lcn=1;
                  }
         }
        result = 1;
    }

    destroy_lcn_conflict_list_buffer(p_list);

    return result;

}


UINT8 prompt_resolve_lcn_conflict_ext(const P_NODE *p_prog1, BOOL conflict_with_old)
{
    POPUP_LIST_PARAM list_param =
    {
        RS_WHICH_PROGRAM_HOLD_THE_LCN, 0, 0, 0 , NULL, 0
    };
    INT32 popup_result = SUCCESS;
    UINT8 total_option=2+1, result;
    PWSTR *p_list = NULL;
    POPUP_YESNO_PARAM param;
    INT8 i=0,option=0;

    if ( !m_prompt_mode || m_auto_resolve_lcn_conflict)
    {
         goto AUTORESOLVE;
        //return 0;   //auto choose pProg1.
    }

    total_option = m_conflic_channel.n_count +2;

    ap_close_popup();

    p_list = create_lcn_conflict_list_buffer(total_option);
    ASSERT(p_list != NULL);

    // make popup list string
    if(conflict_with_old == TRUE)
       create_lcn_conflict_string( &(m_conflic_channel.program_list[0]->p_node), p_list[0], C_CONFLICT_STRING_LENGTH);
    else
        create_lcn_conflict_string(p_prog1, p_list[0], C_CONFLICT_STRING_LENGTH);

    for(i = 0; i<( total_option-2); i++)
    {
         if((conflict_with_old == TRUE) && i==0)
         create_lcn_conflict_string(p_prog1, p_list[i+1], C_CONFLICT_STRING_LENGTH);
     else
            create_lcn_conflict_string( &(m_conflic_channel.program_list[i]->p_node), p_list[i+1], C_CONFLICT_STRING_LENGTH);
    }
    com_uni_str_copy_char_n((UINT8*)(p_list[total_option-1]), osd_get_unicode_string(RS_AUTOMATICALLY_WITH_NO_PROMPT), C_CONFLICT_STRING_LENGTH-1);

    // Indicate end-user to handle conflict items
    list_param.p_table = (UINT32 *)p_list;
    list_param.n_count =total_option;
    list_param.def_sel = list_param.sel_stat;
    list_param.b_wait_time = 10;
    popup_result = ap_open_popup(POPUP_LIST, (UINT32)&list_param);

    if( total_option >2 && popup_result == (UINT8)(total_option - 1) )
    {
        set_auto_resolve_lcn_conflict(TRUE);
AUTORESOLVE:
     if(conflict_with_old == TRUE)
          result = 1;
     else
            result = 0;  //choose pProg1.
    }
    else
    {
         if(conflict_with_old == TRUE)
         {
         if(popup_result == 0)
             result = 1;
         else if(popup_result == 1)
             result = 0;
         else
              result = popup_result;
         }
     else
            result = popup_result;
    }


       if(result ==0)
    {
        for(i = 0; i<m_conflic_channel.n_count; i++)
         {
                m_conflic_channel.program_list[i]->p_node.LCN = P_SEARCH_LCN_INVALID;
                m_conflic_channel.program_list[i]->ctrl.rearrange_lcn =1;
         }
    }
    else
    {
         for(i = 0; i<m_conflic_channel.n_count; i++)
         {
                 if((result -1) != i)
                 {
                   m_conflic_channel.program_list[i]->p_node.LCN = P_SEARCH_LCN_INVALID;
                        m_conflic_channel.program_list[i]->ctrl.rearrange_lcn =1;
                }
                 else
                 {
                        m_conflic_channel.program_list[i]->ctrl.update_lcn=1;
                 }
         }
           if(result != 1)
           {
               result = 2;
           }
    }

    destroy_lcn_conflict_list_buffer(p_list);

    return result;

}


static PTR_PROGRAM_UPDATE check_lcn_conflict_with_new_program_ext(const P_NODE *p_prog, const PTR_PROGRAM_UPDATE my_self)
{
    UINT16 idx;
    PROGRAM_UPDATE *p_prog_upg;

    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {
        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);

        if(p_prog_upg == my_self)
            continue;
#ifndef SUPPORT_LCN_UNIQUE
        if(p_prog_upg->p_node.av_flag != p_prog->av_flag)
            continue;
#endif
        if (p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID   \
            || p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == p_prog->LCN)
        {
            if(p_prog_upg->p_node.hidden_flag)
            {
                p_prog_upg->p_node.LCN  = P_SEARCH_LCN_INVALID;
                p_prog_upg->ctrl.rearrange_lcn = 1;
                continue;
            }
            m_conflic_channel.program_list[m_conflic_channel.n_count] =  m_scanning_found.program_list[idx];
        m_conflic_channel.n_count ++;
            if(m_conflic_channel.n_count  > MAX_CONFILIC)
                    return m_conflic_channel.program_list[0];
        }
    }

    if(m_conflic_channel.n_count>= 1)
            return m_conflic_channel.program_list[0];
    else
        return NULL;
}

static void resolve_lcn_conflict_ext()
{
    INT8 choose = -1;
    UINT16 i;
    PPROGRAM_NODE pn;
    PPARAM_NODE p_param, p_conflict_with_old_program;
    PTR_PROGRAM_UPDATE p_conflict_with_new_program;
    P_NODE prog, prog2;

    UINT16 idx;
    PROGRAM_UPDATE *p_prog_upg=NULL, *p_prog_old=NULL;
    BOOL b_flag= FALSE;
    BOOL conflict_with_old = FALSE;

    //check for all old programs that need to update LCN.
     /********************************set test condition****************************************/
    /********************************set test condition****************************************/

    //check for all old programs that need to update LCN.
    for(i=0; i<m_p_find->n_count; i++)
    {

        com_mem_set((UINT8 *)m_conflic_channel_old.plist, 0, sizeof(MAX_CONFILIC*sizeof(PARAM_NODE*)));
        m_conflic_channel_old.n_count = 0;
        b_flag = FALSE;

        pn = m_p_find->p_node_index[i];
        for(p_param = pn->p_list; NULL != p_param; p_param = p_param->p_next)
        {
            if (p_param->deleted \
                || p_param->ctrl.delete  \
                || p_param->ctrl.successor_service \
                || p_param->ctrl.rearrange_lcn   \
                || p_param->ctrl.delete_from_database \
                || ! p_param->ctrl.update_lcn \
               )
                continue;
            if (p_param->LCN == P_SEARCH_LCN_INVALID)
            {
                p_param->ctrl.rearrange_lcn = 1;
                p_param->ctrl.update_lcn = 1;
                continue;
            }

            param_node_to_p_node(&prog, p_param);
            p_conflict_with_old_program = check_lcn_conflict_with_old_program_ext(&prog, p_param);
            if(NULL != p_conflict_with_old_program)
            {

           if(p_param->hidden_flag)
              {
                 p_param->ctrl.rearrange_lcn = 1;
                p_param->LCN = P_SEARCH_LCN_INVALID;
                continue;
               }

               for(i=0; i<m_conflic_channel_old.n_count ; i++)
              {
                       if(m_conflic_channel_old.plist[i]->hidden_flag)
                       {
                             m_conflic_channel_old.plist[i]->LCN = P_SEARCH_LCN_INVALID;
                          m_conflic_channel_old.plist[i]->ctrl.rearrange_lcn =1;
                           m_conflic_channel_old.n_count --;
                           b_flag = TRUE;
                       }
                       if(b_flag&&(i+1<m_conflic_channel_old.n_count))
                       {
                           m_conflic_channel_old.plist[i] =m_conflic_channel_old.plist[i+1];
                       }
               }
         if(m_conflic_channel_old.n_count ==0)
             continue;

                choose = auto_resolve_old_prog_lcn_conflict_by_current_region_ext(&prog);
            if(choose<0)
                {
                choose =prompt_resolve_old_prog_lcn_conflict(&prog);
                }

          if(choose ==0)
                {
               p_param->ctrl.update_lcn  = 1;
                }
                else // select one of the m_ConflicChannelOld list
                {
                     p_param->ctrl.rearrange_lcn = 1;
                     p_param->LCN = P_SEARCH_LCN_INVALID;
                }
                continue;
            }
        }
    }


    //check for all new programs that need to be appended or successor program.
    for (idx = 0; idx < m_scanning_found.n_count; ++idx)
    {

     com_mem_set((UINT8 *)m_conflic_channel.program_list, 0, sizeof(MAX_CONFILIC*sizeof(PROGRAM_UPDATE*)));
        m_conflic_channel.n_count =0;
        com_mem_set((UINT8 *)m_conflic_channel_old.plist, 0, sizeof(MAX_CONFILIC*sizeof(PARAM_NODE*)));
        m_conflic_channel_old.n_count = 0;

        p_prog_upg = m_scanning_found.program_list[idx];
        ASSERT(p_prog_upg);

        if ( p_prog_upg->ctrl.rearrange_lcn \
            || !(p_prog_upg->ctrl.successor_service || p_prog_upg->ctrl.append) \
           )
            continue;
        if (p_prog_upg->p_node.LCN == P_SEARCH_LCN_INVALID)
        {
            p_prog_upg->ctrl.rearrange_lcn = 1;
            continue;
        }

    conflict_with_old = FALSE;
        p_conflict_with_old_program = check_lcn_conflict_with_old_program_ext(&p_prog_upg->p_node, NULL);
        if(NULL != p_conflict_with_old_program)
        {
            param_node_to_p_node(&prog2, p_conflict_with_old_program);
               p_prog_old = (PROGRAM_UPDATE *)t2_malloc(sizeof(PROGRAM_UPDATE));
            com_mem_set((UINT8 *)p_prog_old, 0, sizeof(PROGRAM_UPDATE) );
            MEMCPY((UINT8 *)&p_prog_old->p_node, (UINT8 *)&prog2, sizeof(P_NODE));
            m_conflic_channel.program_list[0]=p_prog_old;
            m_conflic_channel.program_list[0]->associated_prog_id = 0xff;
            m_conflic_channel.n_count =1;
        conflict_with_old = TRUE;
        }

        p_conflict_with_new_program = check_lcn_conflict_with_new_program_ext(&p_prog_upg->p_node, p_prog_upg);
        if(NULL != p_conflict_with_new_program)
        {
        for(i=0; i<m_conflic_channel.n_count ; i++)
        {
                 if(m_conflic_channel.program_list[i]->p_node.hidden_flag)
                 {
                     if((i==0)&&(p_conflict_with_old_program!=NULL))
                     {
                         p_conflict_with_old_program ->ctrl.rearrange_lcn = 1;
                            p_conflict_with_old_program ->LCN = P_SEARCH_LCN_INVALID;
                    if(p_prog_old != NULL)
                        t2_free((PVOID)p_prog_old);
                    conflict_with_old = FALSE;
                     }
                     else
                     {
                        m_conflic_channel.program_list[i]->ctrl.rearrange_lcn = 1;
                           m_conflic_channel.program_list[i]->p_node.LCN = P_SEARCH_LCN_INVALID;
                       }
                     m_conflic_channel.n_count --;
                     b_flag = TRUE;
                 }
                 if(b_flag&&(i+1<m_conflic_channel.n_count))
                 {
                     m_conflic_channel.program_list[i] =m_conflic_channel.program_list[i+1];
                 }
         }
         if(m_conflic_channel.n_count ==0)
             continue;

            choose = auto_resolve_lcn_conflict_by_current_region_ext(&p_prog_upg->p_node);
            if (choose < 0)
            {
                choose = prompt_resolve_lcn_conflict_ext(&p_prog_upg->p_node, conflict_with_old);
            }

            if(choose == 0)
            {
                p_prog_upg->ctrl.update_lcn = 1;
                if(NULL != p_conflict_with_old_program)
                {
                    p_conflict_with_old_program ->ctrl.rearrange_lcn = 1;
                    p_conflict_with_old_program ->LCN = P_SEARCH_LCN_INVALID;
                }
            }
            else if(choose == 0x10)
            {
          p_prog_upg->ctrl.rearrange_lcn = 1;
                p_prog_upg->p_node.LCN= P_SEARCH_LCN_INVALID;
            }
            else if(choose == 0x11)
            {
               p_prog_upg->ctrl.rearrange_lcn = 1;
                     p_prog_upg->p_node.LCN= P_SEARCH_LCN_INVALID;
                     if(NULL != p_conflict_with_old_program)
                        {
                          p_conflict_with_old_program ->ctrl.rearrange_lcn = 1;
                        p_conflict_with_old_program ->LCN = P_SEARCH_LCN_INVALID;
                if(p_prog_old != NULL)
                    t2_free((PVOID)p_prog_old);
                       }
            }
            else
            {
                  if((choose !=1)&&(NULL != p_conflict_with_old_program))
                  {
                      p_conflict_with_old_program ->ctrl.rearrange_lcn = 1;
                    p_conflict_with_old_program ->LCN = P_SEARCH_LCN_INVALID;
            if(p_prog_old != NULL)
                t2_free((PVOID)p_prog_old);
                  }
                p_prog_upg->ctrl.rearrange_lcn = 1;
                p_prog_upg->p_node.LCN= P_SEARCH_LCN_INVALID;
            }
            continue;   //check next program.
        }
    }

}
#endif

#endif
