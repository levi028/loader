/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_LIB_H_
#define _OSD_LIB_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>
#include <api/libchar/lib_char.h>

#include "osd_common_draw.h"
#include "osd_primitive.h"
#include "osd_common.h"
#include "osd_vkey.h"

#define OSD_MULTI_REGION    0
#define OSD_VSCR_SUPPORT    1

#define V_KEY_NULL    0xFF
#define OSD_FONT_HEIGHT 24 //???

#define OSD_SIGNAL    osd_notify_event

typedef UINT8 PRESULT;
typedef UINT8 VACTION;
typedef UINT32 VEVENT;

//********** Message type Definiton **********//
typedef enum
{
    MSG_TYPE_KEY = 0,
    MSG_TYPE_EVNT,
    MSG_TYPE_MSG,
    MSG_TYPE_RSVD,
}APP_MSG_TYPE,*PAPP_MSG_TYPE;

//********** VACTION Definiton **********//
typedef enum
{
    VACT_NUM_0 = 1,
    VACT_NUM_1,
    VACT_NUM_2,
    VACT_NUM_3,
    VACT_NUM_4,
    VACT_NUM_5,
    VACT_NUM_6,
    VACT_NUM_7,
    VACT_NUM_8,
    VACT_NUM_9,

    VACT_CLOSE,

    VACT_CURSOR_UP,
    VACT_CURSOR_DOWN,
    VACT_CURSOR_LEFT,
    VACT_CURSOR_RIGHT,
    VACT_CURSOR_PGUP,
    VACT_CURSOR_PGDN,

    VACT_SELECT,

    VACT_INCREASE,//progress bar/multisel/editfield whole data +
    VACT_DECREASE,//progress bar/multisel/editfield,whole data -

    VACT_ENTER,

    /* For Edit Field only*/
    VACT_EDIT_LEFT,
    VACT_EDIT_RIGHT,
    VACT_EDIT_ENTER,
    VACT_EDIT_CANCEL_EXIT,
    VACT_EDIT_SAVE_EXIT,
    VACT_EDIT_INCREASE_,
    VACT_EDIT_DECREASE_,
    VACT_NO_RESPOND,
    VACT_EDIT_PASSWORD,
    VACT_PASS    = 0x30,
    VACT_ACT_1,
    VACT_ACT_2,
    VACT_ACT_3,
	VACT_ACT_4,
	VACT_ACT_5,
	VACT_ACT_6,
	VACT_ACT_7,
	VACT_ACT_8,
	VACT_ACT_9,
	VACT_ACT_10,
	VACT_ACT_11,
	VACT_ACT_12,
	VACT_ACT_13,
    VACT_ACT_14,
    VACT_ACT_15,
	VACT_ACT_16,
	VACT_ACT_17,
	VACT_ACT_18,
	VACT_ACT_19,
	VACT_ACT_20,
	VACT_ACT_21,
	VACT_ACT_22,
	VACT_ACT_23,
	VACT_ACT_24,
	VACT_ACT_25,
	VACT_ACT_26,
	VACT_ACT_27,
	VACT_ACT_28,
	VACT_ACT_29,
	VACT_ACT_30,

    
}vact_t;

//********** VEVENT Definiton **********//
typedef enum
{
    EVN_PRE_DRAW = 1,
    EVN_POST_DRAW,

    EVN_ENTER = 11,
    EVN_REQUEST_DATA,
    EVN_REQUEST_STRING,

    EVN_FOCUS_PRE_LOSE = 21,
    EVN_FOCUS_POST_LOSE,
    EVN_FOCUS_PRE_GET,
    EVN_FOCUS_POST_GET,
    EVN_PARENT_FOCUS_PRE_LOSE,
    EVN_PARENT_FOCUS_POST_LOSE,
    EVN_PARENT_FOCUS_PRE_GET,
    EVN_PARENT_FOCUS_POST_GET,
    EVN_ITEM_PRE_CHANGE,
    EVN_ITEM_POST_CHANGE,

    EVN_PRE_OPEN = 31,
    EVN_POST_OPEN,
    EVN_PRE_CLOSE,
    EVN_POST_CLOSE,

    EVN_CURSOR_PRE_CHANGE = 41,
    EVN_CURSOR_POST_CHANGE,

    EVN_PRE_CHANGE = 51,
    EVN_POST_CHANGE,
    EVN_DATA_INVALID,

    EVN_UNKNOWN_ACTION = 61,

    EVN_KEY_GOT = 71,
    EVN_UNKNOWNKEY_GOT,

    EVN_MSG_GOT = 81,

    EVN_VSCR_PREUPDATE = 91,
    EVN_VSCR_POSTUPDATE,
    //TO ADD:
    EVN_FOCUS_SLIDE,
    EVN_MSG_RSVD,

    EVN_MSG_MAX = 0xff,
}GUI_EVN_TYPE,*PGUI_EVN_TYPE;

//********** PROC return Definiton **********//
typedef enum
{
    PROC_SKIP = 0xfc,
    PROC_LEAVE = 0xfd,
    PROC_PASS = 0xfe,
    PROC_LOOP = 0xff,
}GUI_PROC_TYPE,*PGUI_PROC_TYPE;

typedef enum
{
    STRING_NULL = 0,
    STRING_ID,          // string id, data type is UINT16
    STRING_ID_UINT8,    // string id, data type is UINT8
    STRING_ANSI,        // ANSI string
    STRING_UTF8,        // UTF8 string
    STRING_UNICODE,     // Unicode string or String ID
    STRING_MBCS,        // MBCS
    STRING_NUM_TOTAL,   // num/total format
    STRING_NUMBER,      // decimal number
    STRING_NUM_PERCENT, // decimal number + %
    STRING_PROC,        // call proc to get string
    STRING_REVERSE,    //for arabic
}STRING_TYPE;

//********** Object HEAD  Definiton **********//

#define GET_HALIGN(_align)  ((UINT8)(_align) & 0x03)
#define GET_VALIGN(_align)  ((UINT8)(_align) & 0x0C)
typedef enum
{
    C_ALIGN_CENTER = 0x0,
    C_ALIGN_LEFT = 0x01,
    C_ALIGN_RIGHT = 0x02,
    C_ALIGN_TOP = 0x04,
    C_ALIGN_BOTTOM = 0x08,
    C_ALIGN_VCENTER = 0x0c,
    C_ALIGN_SCROLL     = 0x10,
}GUI_ALIGN_TYPE,*PGUI_ALIGN_TYPE;


typedef enum
{
    C_FONT_DEFAULT = 0,
    C_FONT_1,
    C_FONT_2,
    C_FONT_3,
    C_FONT_4,
    C_FONT_5,
    C_FONT_SLVMASK = 0xf0,
}GUI_FONT_SIZE,*PGUI_FONT_SIZE;

typedef enum
{
    OT_TEXTFIELD = 0,
    OT_EDITFIELD,
    OT_BITMAP,
    OT_MULTISEL,
    OT_PROGRESSBAR,
    OT_SCROLLBAR,
    OT_MULTITEXT,
    OT_LIST,
    OT_MATRIXBOX,
    OT_OBJLIST,
    OT_CONTAINER,
    OT_MAX
}GUI_OBJ_TYPE,*PGUI_OBJ_TYPE;

typedef enum
{
    C_ATTR_ACTIVE = 0x01,
    C_ATTR_HIGHLIGHT,
    C_ATTR_SELECT,
    C_ATTR_INACTIVE,
    C_ATTR_HIDDEN,

    C_ATTR_LMASK = 0xf,

    C_ATTR_SLAVE = 0x10,
    C_ATTR_HMASK = 0xf0,
}GUI_OBJ_ATTR,*PGUI_OBJ_ATTR;

typedef struct _COLOR_STYLE
{
    UINT8    b_show_idx;
    UINT8    b_hlidx;       // Style during HighLight
    UINT8    b_sel_idx;      // Style during Selected, add by sunny 07/21, For Window Object, it's Close Attribute
    UINT8    b_gray_idx;     // Gray Style, It will be active while bAction == ACTION_GRAY. This Attribute is not included in Window Object
}COLOR_STYLE, *PCOLOR_STYLE;

typedef struct _OBJECT_HEAD     OBJECT_HEAD;
typedef struct _OBJECT_HEAD     *POBJECT_HEAD;
typedef VACTION (*PFN_KEY_MAP)(POBJECT_HEAD p_obj, UINT32 key);
typedef PRESULT (*PFN_CALLBACK)(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
struct _OBJECT_HEAD
{
    UINT8 b_type;
    UINT8 b_attr;// defined in form C_ATTR_XXX
    UINT8 b_font;// defined in form C_FONT_XXX
    UINT8 b_id;// 0: for component without focus

    UINT8 b_left_id;
    UINT8 b_right_id;
    UINT8 b_up_id;
    UINT8 b_down_id;

    GUI_RECT  frame;
    COLOR_STYLE style;

    PFN_KEY_MAP pfn_key_map;
    PFN_CALLBACK pfn_callback;

    POBJECT_HEAD p_next;
    POBJECT_HEAD p_root;
};

typedef struct SLAVE_OBJHEAD
{
    GUI_RECT  frame;
    COLOR_STYLE style;
}SDHEAD,*PSDHEAD;

typedef enum
{
    C_UPDATE_ALL = 0x01,
    C_UPDATE_FOCUS,
    C_UPDATE_CONTENT,
    C_UPDATE_FRAME,
    C_UPDATE_CURSOR,
    C_UPDATE_MASK = 0xf,
}GUI_OBJ_UPDATE,*PGUI_OBJ_UPDATE;

typedef enum
{
    C_DRAW_TYPE_NORMAL = (0x01<<4),
    C_DRAW_TYPE_HIGHLIGHT = (0x02<<4),
    C_DRAW_TYPE_GRAY = (0x03<<4),
    C_DRAW_TYPE_SELECT = (0x04<<4),
    C_DRAW_TYPE_HIDE = (0x05<<4),
    C_DRAW_TYPE_MASK = (0xf<<4),
}GUI_OBJ_DRAW_TYPE,*PGUI_OBJ_DRAW_TYPE;

typedef enum
{
    C_DRAW_SIGN_EVN_FLG = 0x80000000UL,
    C_CLOSE_CLRBACK_FLG = 0x40000000UL,
}GUI_OBJ_DRAW_PARAM,*PGUI_OBJ_DRAW_PARAM;

#define osd_set_update_type(n_cmd_draw,update_type) \
    n_cmd_draw = ( (n_cmd_draw & 0xFFFFFFF0) | update_type)
#define osd_get_update_type(n_cmd_draw)    \
    ((n_cmd_draw) & 0x0F)
#define osd_set_draw_type(n_cmd_draw,draw_type) \
    n_cmd_draw = ( (n_cmd_draw & 0xFFFFFF0F) | draw_type)
#define osd_get_draw_type(n_cmd_draw) \
    ( (n_cmd_draw) & 0xF0)

#define osd_set_color(p_obj, _b_show_idx, _b_hlidx, _b_sel_idx, _b_gray_idx) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->style.b_show_idx = (UINT8)(_b_show_idx); \
        ((POBJECT_HEAD)(p_obj))->style.b_hlidx = (UINT8)(_b_hlidx); \
        ((POBJECT_HEAD)(p_obj))->style.b_sel_idx = (UINT8)(_b_sel_idx); \
        ((POBJECT_HEAD)(p_obj))->style.b_gray_idx = (UINT8)(_b_gray_idx); \
    }while(0)
#define osd_get_color(p_obj) (&(((POBJECT_HEAD)(p_obj))->style))

#define osd_set_id(p_obj, _b_id, _b_lid, _b_rid, _b_uid, _b_did) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->b_id = (UINT8)(_b_id); \
        ((POBJECT_HEAD)(p_obj))->b_left_id = (UINT8)(_b_lid); \
        ((POBJECT_HEAD)(p_obj))->b_right_id = (UINT8)(_b_rid); \
        ((POBJECT_HEAD)(p_obj))->b_up_id = (UINT8)(_b_uid); \
        ((POBJECT_HEAD)(p_obj))->b_down_id = (UINT8)(_b_did); \
    }while(0)
#define osd_get_obj_id(p_obj)  ( ((POBJECT_HEAD)(p_obj))->b_id)

#define osd_set_key_map(p_obj, _pfn_key_map) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->pfn_key_map = (PFN_KEY_MAP)(_pfn_key_map); \
    }while(0)
#define osd_set_callback(p_obj, _pfn_callback) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->pfn_callback = (PFN_CALLBACK)(_pfn_callback); \
    }while(0)

#define osd_get_attr_slave(p_obj) \
            (C_ATTR_SLAVE == ((((POBJECT_HEAD)(p_obj))->b_attr)&C_ATTR_SLAVE))

#define osd_set_attr_slave(p_obj)\
   do{((POBJECT_HEAD)(p_obj))->b_attr = (UINT8)(((POBJECT_HEAD)(p_obj))|C_ATTR_SLAVE);}while(0)

#define osd_check_attr(p_obj, _b_attr) \
    ((UINT8)(_b_attr) == ((((POBJECT_HEAD)(p_obj))->b_attr)&C_ATTR_LMASK))
#define osd_set_attr(p_obj, _b_attr) \
    do{((POBJECT_HEAD)(p_obj))->b_attr = (UINT8)(_b_attr);}while(0)

#define osd_set_rect(r, _l, _t, _w, _h) \
    do{ \
        ((PGUI_RECT)(r))->u_start_x  = (UINT16)(_l); \
        ((PGUI_RECT)(r))->u_start_y = (UINT16)(_t); \
        ((PGUI_RECT)(r))->u_width = (UINT16)(_w); \
        ((PGUI_RECT)(r))->u_height = (UINT16)(_h); \
    }while(0)

#define osd_copy_rect(dest, src) osd_set_rect(dest,(src)->u_start_x, (src)->u_start_y, (src)->u_width,(src)->u_height)

#define osd_set_rect2    osd_copy_rect
#define osd_zero_rect(rct) osd_set_rect(rct,0,0,0,0)

#define osd_set_obj_rect(p_obj,_l, _t, _w, _h)    \
    osd_set_rect(& (((POBJECT_HEAD)(p_obj))->frame),_l, _t, _w, _h)

#define osd_set_objp_next(p_obj,p_next_obj)    \
    ((POBJECT_HEAD)(p_obj))->p_next = (POBJECT_HEAD)(p_next_obj)
#define osd_get_objp_next(p_obj)    \
    ((POBJECT_HEAD)(p_obj))->p_next
#define osd_set_obj_root(p_obj,p_root_obj)    \
    ((POBJECT_HEAD)(p_obj))->p_root = (POBJECT_HEAD)(p_root_obj)
#define osd_get_obj_root(p_obj)    \
    ((POBJECT_HEAD)(p_obj))->p_root

#define osd_set_osdmessage(msg,msgtype,msgcode)    \
    ((msg) = ((msgtype) << 16) | (msgcode))
#define osd_get_osdmessage(msgtype,msgcode)    \
    ( ((msgtype) << 16) | (msgcode))
#define osd_get_osdmessage_type(msg)    \
    ((msg)>>16)
#define    osd_get_osdmessage_code(msg)    \
    ((msg) & 0xFFFF)

BOOL    osd_rect_in_rect(const PGUI_RECT R,const PGUI_RECT r);
void    osd_get_rects_cross(const PGUI_RECT back_r,const PGUI_RECT forg_r,PGUI_RECT relativ_r);

POBJECT_HEAD    osd_get_top_root_object(POBJECT_HEAD p_obj);
UINT8    osd_get_focus_id(POBJECT_HEAD p_obj);
void osd_set_whole_obj_attr(POBJECT_HEAD    p_obj, UINT8 b_attr);
POBJECT_HEAD    osd_get_object(POBJECT_HEAD p_obj, UINT8 b_id);
POBJECT_HEAD    osd_get_focus_object(POBJECT_HEAD p_obj);
UINT8    osd_get_adjacent_obj_id(POBJECT_HEAD p_obj, VACTION action);
POBJECT_HEAD    osd_get_adjacent_object(POBJECT_HEAD p_obj, VACTION action);
PRESULT     osd_change_focus(POBJECT_HEAD p_obj, UINT16 b_new_focus_id,UINT32 parm);

PGUI_VSCR osd_draw_slave_frame(PGUI_RECT prct,UINT32 b_style_idx);
UINT8 osd_get_slave_wstyle(PCOLOR_STYLE pstyle,POBJECT_HEAD pobj,UINT8 colorstyle);
UINT8    osd_get_show_index(const PCOLOR_STYLE p_color_style, UINT8 b_attr);
PGUI_VSCR    osd_draw_object_frame(PGUI_RECT p_rect,UINT32 b_style_idx);
void    osd_draw_object_cell(POBJECT_HEAD p_obj, UINT8 b_style_idx,    UINT32 n_cmd_draw);
PRESULT    osd_draw_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw );
PRESULT    osd_track_object(POBJECT_HEAD p_obj,UINT32 n_cmd_draw );
PRESULT    osd_sel_object(POBJECT_HEAD p_obj,  UINT32 n_cmd_draw );
void    osd_hide_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw );
void    osd_clear_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw);

PRESULT    osd_notify_event(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT    osd_obj_open(POBJECT_HEAD p_obj, UINT32 param);
PRESULT    osd_obj_close(POBJECT_HEAD p_obj,UINT32 param);
PRESULT    osd_obj_proc(POBJECT_HEAD p_obj, UINT32 msg_type,UINT32 msg,UINT32 param1);
//***********************************************************************************************//

#include "obj_textfield.h"
#include "obj_bitmap.h"
#include "obj_scrollbar.h"
#include "obj_progressbar.h"
#include "obj_objectlist.h"
#include "obj_container.h"
#include "obj_editfield.h"
#include "obj_multisel.h"
#include "obj_multitext.h"
#include "obj_list.h"
#include "obj_matrixbox.h"
#include "osd_common_draw.h"

typedef enum
{
    LIB_FONT = 0x1000,
    LIB_FONT_MASSCHAR = 0x1f00,
    LIB_STRING = 0x2000,
    LIB_PALLETE = 0x4000,
    LIB_ICON = 0x8000,
}GUI_RSC_TYPE,*PGUI_RSC_TYPE;

typedef struct tag_font_icon_prop
{
    UINT16    m_w_height;
    UINT16    m_w_width;
    UINT16    m_w_actual_width;
    UINT8    m_b_color;
#ifndef _UNCOMPRESSED_ARRAY_METHOD
    BOOL    m_f_flag;//flag to show the data being compressed,structure so----
#endif
}FONTICONPROP;

typedef struct tag_object_info
{
    UINT16    m_w_obj_class;            // the class ID of FONT,STRING the object belongs to
    FONTICONPROP     m_obj_attr;        // the attribute of this object
}OBJECTINFO,*lp_objectinfo;

typedef    UINT16    (*OSD_GET_LANG_ENV)(void);
typedef    BOOL    (*OSD_GET_OBJ_INFO)(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo p_object_info);
typedef    UINT8*     (*OSD_GET_RSC_DATA)(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo object_info);
typedef    UINT8*     (*OSD_GET_THAI_FONT_DATA)(UINT16 w_lib_class,struct thai_cell *cell,lp_objectinfo object_info);
typedef    BOOL       (*OSD_REL_RSC_DATA)(UINT8* lp_data_buf,lp_objectinfo p_object_info);
typedef     ID_RSC     (*OSD_GET_FONT_LIB)(UINT16 u_char);
typedef     PWINSTYLE (*OSD_GET_WIN_STYLE)(UINT8 b_style_idx);
typedef     ID_RSC    (*OSD_GET_STR_LIB_ID)(UINT16 env_id);
typedef    UINT32 (*AP_HK_TO_VK)(UINT32 start, UINT32 key_info, UINT32* vkey );

typedef struct osd_rsc_func
{/*Split from old struct,to keep OSD_LIB maintanace*/
    OSD_GET_LANG_ENV    osd_get_lang_env;
    OSD_GET_OBJ_INFO        osd_get_obj_info;
    OSD_GET_RSC_DATA    osd_get_rsc_data;
    OSD_GET_THAI_FONT_DATA osd_get_thai_font_data;
    OSD_REL_RSC_DATA    osd_rel_rsc_data;
    OSD_GET_FONT_LIB     osd_get_font_lib;
    OSD_GET_WIN_STYLE    osd_get_win_style;
    OSD_GET_STR_LIB_ID    osd_get_str_lib_id;
    AP_HK_TO_VK            ap_hk_to_vk;
}GUI_RSC_FNC,*PGUI_RSC_FNC;

extern GUI_RSC_FNC g_gui_rscfunc;
UINT8 app_rsc_init(PGUI_RSC_FNC p_rsc_info);
BOOL    osd_get_char_width_height(UINT16 u_string,UINT8 font, UINT16* width, UINT16* height);
BOOL osd_get_thai_cell_width_height(struct thai_cell *cell, UINT8 font, UINT16* width, UINT16* height);
#ifdef HINDI_LANGUAGE_SUPPORT
BOOL osd_get_devanagari_width_height(struct devanagari_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
BOOL osd_get_telugu_width_height(struct telugu_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
BOOL osd_get_bengali_width_height(struct bengali_cell *pCell, UINT8 fontsize, UINT16 *pWidth, UINT16 *pHeight);
#endif
UINT16  osd_multi_font_lib_str_max_hw(UINT8* p_string,UINT8 font, UINT16 *w_h, UINT16 *w_w,UINT16 str_len);
UINT8*  osd_get_unicode_string(UINT16 u_index);
BOOL osd_get_lib_info_by_word_idx(UINT16 w_idx, ID_RSC rsc_lib_id, UINT16 *w_width, UINT16 *u_height);
UINT8* osd_get_rsc_pallette(UINT16 w_pal_idx);
#endif//_OSD_LIB_H_

