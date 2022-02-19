/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_lib.h
*
*    Description: OSD object management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OSD_LIB_H_
#define _OSD_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <api/libchar/lib_char.h>
#include "lib_ge_osd.h"
#include "osd_common.h"
#include "osd_vkey.h"

#define OSD_MULTI_REGION    0
#define OSD_VSCR_SUPPORT    1

#define V_KEY_NULL  0xFF
#define OSD_FONT_HEIGHT 24 //???

/////////////////////////////////////////////////////////////
//********** Message type Definiton **********//
/////////////////////////////////////////////////////////////
#define MSG_TYPE_KEY        0
#define MSG_TYPE_EVNT       1
#define MSG_TYPE_MSG        2

/////////////////////////////////////////////////////////////
//********** VACTION Definiton **********//
/////////////////////////////////////////////////////////////
// Common Basic Key
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
    VACT_PASS   = 0x30,
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

/////////////////////////////////////////////////////////////
//********** VEVENT Definiton **********//
/////////////////////////////////////////////////////////////


// Param1 - Drawing mode
// Param2 - only for MatrixBox, specifies the cell rect.
// Return - PROC_LOOP: terminate the default drawing execution
//          PROC_PASS: continue drawing
#define EVN_PRE_DRAW        1
#define EVN_POST_DRAW       2

// Param1 - Determined by component type
//          TextField: control ID
//          EditField: the current value, pointer to a string or a numeric value
//                     depending on editfield style
//          Multisel, List, MatrixBox: the current selection
// Return - Determined by component type
//          TextField: PROC_LEAVE to exit the applicaton.
//                     PROC_LOOP: continue done.
#define EVN_ENTER           11

#define EVN_FOCUS_PRE_LOSE    21
#define EVN_FOCUS_POST_LOSE      22

#define EVN_FOCUS_PRE_GET   23
#define EVN_FOCUS_POST_GET       24

#define EVN_PARENT_FOCUS_PRE_LOSE      25
#define EVN_PARENT_FOCUS_POST_LOSE     26
#define EVN_PARENT_FOCUS_PRE_GET   27
#define EVN_PARENT_FOCUS_POST_GET       28

#define EVN_ITEM_PRE_CHANGE   29
#define EVN_ITEM_POST_CHANGE    30

/*
#define EVN_WIN_CLOSING       31
#define EVN_WIN_CLOSED        32
#define EVN_WIN_OPENING       33
#define EVN_WIN_OPENED        34
*/

#define EVN_PRE_OPEN            31
#define EVN_POST_OPEN           32
#define EVN_PRE_CLOSE           33
#define EVN_POST_CLOSE          34

// Only for EditField, List, MatrixBox
// Param1 - pointer to the new cursor position. (UINT8 *)
#define EVN_CURSOR_PRE_CHANGE 41   // The cursor will be changed
// Param1 - the current cursor position
#define EVN_CURSOR_POST_CHANGE  42   // The cursor has been changed

// Only for EditField, Multisel, List, and MatrixBox
// Param1 - Determined by component type
//          EditField: pointer to the new value. (UINT32 * or PWCHAR)
//          Multisel, List, MatrixBox: pointer to the new selection
#define EVN_PRE_CHANGE        51  // control will change its content
// Param1 - Determined by component type
//          EditFiled: the current value
//          Multisel, List, MatrixBox: the current selection
#define EVN_POST_CHANGE         52  // control has changed its content

#define EVN_DATA_INVALID        53  // control's data is invalid

// Only for List, and MatrixBox
// Param1 - Determined by component type
//          List:
//          MatrixBox:
#define EVN_REQUEST_DATA    12  // Request data

//Only for Multisel
//Param1 - Determined by component type
//          Multisel: the current selection
#define EVN_REQUEST_STRING 13 // Request string

// Param1 - action code
// Param1 - action code
#define EVN_UNKNOWN_ACTION     61   //EVN_USER_ACTION

#define EVN_KEY_GOT            71
#define EVN_UNKNOWNKEY_GOT     72   //EVN_PASS_ACTION

#define EVN_MSG_GOT            81   //A message gotted

//********** VEVENT parameter Definiton **********//

#define EVN_LIST_PAGE_PRE_CHANGE   82       // only for obj_objlist,
                                            // to calculate the new focus when page changed
                                            // (sometime the obj attr will be inactive, need re-load).


//********** PROC return Definiton **********//
#define PROC_LOOP       0xFF
#define PROC_PASS       0xFE
#define PROC_LEAVE      0xFD

/////////////////////////////////////////////////////////
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
    STRING_REVERSE, //for arabic
}STRING_TYPE;


/////////////////////////////////////////////////////////////
//********** Object HEAD  Definiton **********//
/////////////////////////////////////////////////////////////



#define C_ALIGN_CENTER  0x00
#define C_ALIGN_LEFT    0x01
#define C_ALIGN_RIGHT   0x02
#define C_ALIGN_TOP     0x04
#define C_ALIGN_BOTTOM  0x08
#define C_ALIGN_VCENTER 0x0C
#define C_ALIGN_SCROLL  0x10



#define GET_HALIGN(_align)  ((UINT8)(_align) & 0x03)
#define GET_VALIGN(_align)  ((UINT8)(_align) & 0x0C)

#define C_FONT_DEFAULT  0
#define C_FONT_1        1
#define C_FONT_2        2
#define C_FONT_3        3
#define C_FONT_4        4
#define C_FONT_5        5

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
    OT_ANIMATION,
    OT_MAX
}OBJECT_TYPE;

enum
{
    PIXEL0BIT   = 0,
    PIXEL1BIT   = 1,
    PIXEL2BIT   = 2,
    PIXEL4BIT   = 4,
    PIXEL8BIT   = 8,
    PIXEL16BIT  = 16,
    PIXEL24BIT  = 24,
    PIXEL32BIT  = 32,
};

#define     C_ATTR_ACTIVE       0x01
//#define   C_ATTR_HL       0x02
#define C_ATTR_SELECT       0x03
#define     C_ATTR_INACTIVE     0x04
#define     C_ATTR_HIDDEN       0x05

typedef struct _COLOR_STYLE
{
    UINT8 b_show_idx;
    UINT8 b_hlidx;       // Style during HighLight
    UINT8 b_sel_idx;      // Style during Selected, add by sunny 07/21,
                        //For Window Object, it's Close Attribute
    UINT8 b_gray_idx;     // Gray Style, It will be active while
                        //bAction == ACTION_GRAY. This Attribute is not
                        //included in Window Object
}COLOR_STYLE, *PCOLOR_STYLE;

#define PRESULT UINT8
#define VACTION vact_t
#define VEVENT  UINT32

typedef struct _OBJECT_HEAD     OBJECT_HEAD;
typedef struct _OBJECT_HEAD     *POBJECT_HEAD;
typedef VACTION (*PFN_KEY_MAP)(POBJECT_HEAD p_obj, UINT32 key);
typedef PRESULT (*PFN_CALLBACK)(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1,\
        UINT32 param2);

struct _OBJECT_HEAD
{
    UINT8 b_type;
    UINT8 b_attr;    // defined in form C_ATTR_XXX
    UINT8 b_font;    // defined in form C_FONT_XXX
    UINT8 b_id;      // 0: for component without focus

    UINT8 b_left_id;
    UINT8 b_right_id;
    UINT8 b_up_id;
    UINT8 b_down_id;

    OSD_RECT frame;
    COLOR_STYLE style;

    PFN_KEY_MAP pfn_key_map;
    PFN_CALLBACK pfn_callback;
 
    POBJECT_HEAD p_next;
    POBJECT_HEAD p_root;
};

/////////////////////////////////////////////////////////////
//********** some function return and parameter  Definiton **********//
/////////////////////////////////////////////////////////////

typedef struct tag_vscr_list
{
    VSCR    vscr;
    struct tag_vscr_list  *p_next;
    struct tag_vscr_list   *p_cur;
}VSCR_LIST,*PVSCR_LIST;

/* update(draw) content */
// 0x0000 00xx
#define C_UPDATE_ALL        0x01
#define C_UPDATE_FOCUS      0x02
#define C_UPDATE_CONTENT    0x03
#define C_UPDATE_CURSOR     0x04

/*draw color style */
//0x0000 xx00
#define C_DRAW_TYPE_NORMAL      (0x01<<4)
#define C_DRAW_TYPE_HIGHLIGHT   (0x02<<4)
#define C_DRAW_TYPE_GRAY        (0x03<<4)
#define C_DRAW_TYPE_SELECT      (0x04<<4)
#define C_DRAW_TYPE_HIDE        (0x05<<4)

#define C_DRAW_SIGN_EVN_FLG             0x80000000
#define C_CLOSE_CLRBACK_FLG         0x40000000  //only for OSD_Closeobject(...)


#define osd_set_update_type(n_cmd_draw,update_type) \
    n_cmd_draw = ( (n_cmd_draw & 0xFFFFFFF0) | update_type)
#define osd_get_update_type(n_cmd_draw) \
    ((n_cmd_draw) & 0x0F)

#define osd_set_draw_type(n_cmd_draw,draw_type)  \
    n_cmd_draw = ( (n_cmd_draw & 0xFFFFFF0F) | draw_type)
#define osd_get_draw_type(n_cmd_draw)   \
    ( (n_cmd_draw) & 0xF0)

/////////////////////////////////////////////////////////

#define OSD_DIRDRAW         0
#define OSD_REQBUF          1
#define OSD_GET_CORNER      2
#define OSD_GET_BACK        4
#define OSD_INIT_BUFF       8

/////////////////////////////////////////
//macro to access object header

#define osd_set_color(p_obj, _b_show_idx, _b_hlidx, _b_sel_idx, _b_gray_idx) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->style.b_show_idx = (UINT8)(_b_show_idx); \
        ((POBJECT_HEAD)(p_obj))->style.b_hlidx = (UINT8)(_b_hlidx); \
        ((POBJECT_HEAD)(p_obj))->style.b_sel_idx = (UINT8)(_b_sel_idx); \
        ((POBJECT_HEAD)(p_obj))->style.b_gray_idx = (UINT8)(_b_gray_idx); \
    }while(0)
//PCOLOR_STYLE OSD_GetColor(POBJECT_HEAD pObj);
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

#define osd_check_attr(p_obj, _b_attr) \
    ((UINT8)(_b_attr) == ((POBJECT_HEAD)(p_obj))->b_attr)
//void OSD_SetAttr(POBJECT_HEAD pObj, UINT8 bAttr);
#define osd_set_attr(p_obj, _b_attr) \
    do{((POBJECT_HEAD)(p_obj))->b_attr = (UINT8)(_b_attr);}while(0)
UINT8 osd_get_show_index(const PCOLOR_STYLE p_color_style, UINT8 b_attr);


#define osd_set_rect(r, _l, _t, _w, _h) \
    do{ \
        ((POSD_RECT)(r))->u_left = (UINT16)(_l); \
        ((POSD_RECT)(r))->u_top = (UINT16)(_t); \
        ((POSD_RECT)(r))->u_width = (UINT16)(_w); \
        ((POSD_RECT)(r))->u_height = (UINT16)(_h); \
    }while(0)

//void OSD_CopyRect(POSD_RECT pDest, POSD_RECT pSrc);
#define osd_copy_rect(dest, src) \
    do{ \
        ((UINT32 *)(dest))[0] = ((UINT32 *)(src))[0]; \
        ((UINT32 *)(dest))[1] = ((UINT32 *)(src))[1]; \
    }while(0)
#define osd_set_rect2    osd_copy_rect

//void OSD_ZeroRect(POSD_RECT pRect)
#define osd_zero_rect(r) \
    do{ \
        ((UINT32 *)(r))[0] = 0; \
        ((UINT32 *)(r))[1] = 0; \
    }while(0);

#define osd_set_obj_rect(p_obj,_l, _t, _w, _h) \
    osd_set_rect(& (((POBJECT_HEAD)(p_obj))->frame),_l, _t, _w, _h)

#define osd_set_objp_next(p_obj,p_next_obj)  \
    ((POBJECT_HEAD)(p_obj))->p_next = (POBJECT_HEAD)(p_next_obj)

#define osd_get_objp_next(p_obj)   \
    ((POBJECT_HEAD)(p_obj))->p_next

#define osd_set_obj_root(p_obj,p_root_obj)   \
    ((POBJECT_HEAD)(p_obj))->p_root = (POBJECT_HEAD)(p_root_obj)

#define osd_get_obj_root(p_obj)    \
    ((POBJECT_HEAD)(p_obj))->p_root

//common get object operation w functions
#define osd_set_osdmessage(msg,msgtype,msgcode)  \
    ((msg) = ((msgtype) << 16) | (msgcode))

#define osd_get_osdmessage(msgtype,msgcode)  \
    ( ((msgtype) << 16) | (msgcode))

#define osd_get_osdmessage_type(msg)  \
    ((msg)>>16)

#define osd_get_osdmessage_code(msg)  \
    ((msg) & 0xFFFF)

#define osd_set_obj_font(p_obj,font) \
    do{ \
        ((POBJECT_HEAD)(p_obj))->b_font = font; \
    }while(0)

#define osd_get_obj_font(p_obj)    (((POBJECT_HEAD)(p_obj))->b_font)


//rectangle operation functions
BOOL osd_rect_in_rect(const struct osdrect* R,const struct osdrect* r);
void osd_get_rects_cross(const struct osdrect *back_r, \
    const struct osdrect *forg_r, struct osdrect* relativ_r);

//Virtual screen operation functions
void osd_update_vscr(VSCR* p_vscr);
void osd_update_vscr_ext(VSCR *p_vscr, UINT8 region_id);
LPVSCR osd_get_vscr(struct osdrect* p_rect, UINT32 b_flag);
LPVSCR osd_draw_object_frame(struct osdrect *p_rect,UINT8 b_style_idx);
#define osd_set_vscr_modified(p_vscr)  \
    (p_vscr)->update_pending = 1


//Get top root object, focus object ID, object by ID, focus object
POBJECT_HEAD osd_get_top_root_object(POBJECT_HEAD p_obj);
UINT8        osd_get_focus_id(POBJECT_HEAD p_obj);
POBJECT_HEAD osd_get_object(POBJECT_HEAD p_obj, UINT8 b_id);
POBJECT_HEAD osd_get_focus_object(POBJECT_HEAD p_obj);
UINT8 osd_get_adjacent_obj_id(POBJECT_HEAD p_obj, VACTION action);
POBJECT_HEAD osd_get_adjacent_object(POBJECT_HEAD p_obj, VACTION action);
PRESULT      osd_change_focus(POBJECT_HEAD p_obj, UINT16 b_new_focus_id,
    UINT32 parm);

//OSD object draw functions
void osd_draw_object_cell(POBJECT_HEAD p_obj, UINT8 b_style_idx,
    UINT32 n_cmd_draw);

//nCmdDraw : C_DRAW_SIGN_EVN_FLG | C_UPDATE_XXX
void osd_draw_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw );
void osd_track_object(POBJECT_HEAD p_obj,UINT32 n_cmd_draw );
void osd_sel_object(POBJECT_HEAD p_obj,  UINT32 n_cmd_draw );
void    osd_hide_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw );
void    osd_clear_object(POBJECT_HEAD p_obj, UINT32 n_cmd_draw);

//call callback functions
PRESULT osd_notify_event(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1,
    UINT32 param2);
#define OSD_SIGNAL  osd_notify_event


//Open / close a the menu with specifed object
PRESULT osd_obj_open(POBJECT_HEAD p_obj, UINT32 param);
PRESULT osd_obj_close(POBJECT_HEAD p_obj,UINT32 param);

//object proc function
PRESULT osd_obj_proc(POBJECT_HEAD p_obj, UINT32 msg_type,UINT32 msg,
    UINT32 param1);



#define LIB_FONT                    0x1000
#define LIB_STRING              0x2000
#define LIB_PALLETE             0x4000
#define LIB_ICON                    0x8000
#define LIB_FONT_MASSCHAR       0x1F00

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
#include "obj_animation.h"

#include "osd_common_draw.h"

/*****************************************
***-the property of a certain object-****
NOTE:members of this struct is meaningful
only to font and icon object,except m_fFlag,
which is usefule to all kinds of object*/
typedef struct tag_font_icon_prop
{
    UINT16  m_w_height;
    UINT16  m_w_width;
    UINT16  m_w_actual_width;         //the actual width the data was stored,because filling happened
                        //to match with the unit of 8bytes
                        //actualwidth data with the unit of UINT8,not BITE
                        //considering you can calculate from width and color,why give it???
    UINT8   m_b_color;           // the color BPP of this object
#ifndef _UNCOMPRESSED_ARRAY_METHOD
    BOOL    m_f_flag;            //flag to show the data being compressed,structure so----
#endif
#ifdef BIDIRECTIONAL_OSD_STYLE
    BOOL    m_mirror_flag;
#endif
}FONTICONPROP;

typedef struct tag_object_info
{
    UINT16  m_w_obj_class;            // the class ID of FONT,STRING the object belongs to
                                    // the object ID of ICON,PALLETE
    FONTICONPROP    m_obj_attr;      // the attribute of this object
}OBJECTINFO,*lp_objectinfo;

typedef enum
{
    GE_DRAW_MODE = 0,
    OSD_DRAW_MODE,
}OSD_DRAW_DEV_MODE;

#define ID_RSC  UINT16
typedef UINT16  (*OSD_GET_LANG_ENV)(void);
typedef BOOL    (*OSD_GET_OBJ_INFO)(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo p_object_info);
typedef UINT8*  (*OSD_GET_RSC_DATA)(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo object_info);
typedef UINT8*  (*OSD_GET_THAI_FONT_DATA)(UINT16 w_lib_class, struct thai_cell *cell,lp_objectinfo object_info);
#ifdef HINDI_LANGUAGE_SUPPORT
typedef BOOL	(*OSD_GET_DEVANAGARI_INFO)(UINT16 w_lib_class, UINT16 group_index, UINT16 *width, UINT16 *height);
typedef UINT8*	(*OSD_GET_DEVANAGARI_DATA)(UINT16 w_lib_class, struct devanagari_cell *pCell, lp_objectinfo object_info);
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
typedef BOOL	(*OSD_GET_TELUGU_INFO)(UINT16 w_lib_class, UINT16 group_index, UINT16 *width, UINT16 *height);
typedef UINT8*	(*OSD_GET_TELUGU_DATA)(UINT16 w_lib_class, struct telugu_cell *pCell, lp_objectinfo object_info);
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
typedef BOOL	(*OSD_GET_BENGALI_INFO)(UINT16 w_lib_class, UINT16 group_index, UINT16 *width, UINT16 *height);
typedef UINT8*	(*OSD_GET_BENGALI_DATA)(UINT16 w_lib_class, struct bengali_cell *pCell, lp_objectinfo object_info);
#endif
typedef BOOL    (*OSD_REL_RSC_DATA)(UINT8* lp_data_buf,lp_objectinfo p_object_info);
typedef ID_RSC  (*OSD_GET_FONT_LIB)(UINT16 u_char);
typedef PWINSTYLE (*OSD_GET_WIN_STYLE)(UINT8 b_style_idx);
typedef ID_RSC    (*OSD_GET_STR_LIB_ID)(UINT16 env_id);
typedef UINT32 (*AP_HK_TO_VK)(UINT32 start, UINT32 key_info, UINT32* vkey );
#ifdef BIDIRECTIONAL_OSD_STYLE
typedef BOOL    (*OSD_GET_MIRROR_FLAG)(void);
#endif

struct osd_rsc_info
{/*Split from old struct,to keep OSD_LIB maintanace,UI resource is unique*/
    OSD_GET_LANG_ENV    osd_get_lang_env;
    OSD_GET_OBJ_INFO        osd_get_obj_info;
    OSD_GET_RSC_DATA    osd_get_rsc_data;
    OSD_GET_THAI_FONT_DATA osd_get_thai_font_data;
#ifdef HINDI_LANGUAGE_SUPPORT
	OSD_GET_DEVANAGARI_INFO osd_get_devanagari_info;
	OSD_GET_DEVANAGARI_DATA osd_get_devanagari_data;
#endif	
#ifdef TELUGU_LANGUAGE_SUPPORT
	OSD_GET_TELUGU_INFO osd_get_telugu_info;
	OSD_GET_TELUGU_DATA osd_get_telugu_data;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	OSD_GET_BENGALI_INFO osd_get_bengali_info;
	OSD_GET_BENGALI_DATA osd_get_bengali_data;
#endif	
    OSD_REL_RSC_DATA    osd_rel_rsc_data;
    OSD_GET_FONT_LIB    osd_get_font_lib;
    OSD_GET_WIN_STYLE   osd_get_win_style;
    OSD_GET_STR_LIB_ID  osd_get_str_lib_id;
    AP_HK_TO_VK         ap_hk_to_vk;
#ifdef BIDIRECTIONAL_OSD_STYLE
    OSD_GET_MIRROR_FLAG osd_get_mirror_flag;
#endif
};
struct osd_region_info
{/*Inherit from OSD hld driver,eg. 29,29C serial*/
    HANDLE      osddev_handle;
    struct osdpara  t_open_para;
    struct osdrect  osdrect;
    UINT8       region_id;
    UINT8       u_is_use_vscr;
};

struct osd_surface_info
{/*New build for Ge hld driver*/
    struct ge_device    *ge_dev;
    ge_surface_desc_t   *ge_dissf,*ge_virsf;
    ge_region_t     ge_region_para;
    UINT8           dissf_region_id;
    UINT8           virsf_region_id;
};

extern struct osd_rsc_info   g_osd_rsc_info;//Global OSD resource track record
#ifdef SUPPORT_DRAW_EFFECT
extern PVSCR_LIST    p_vscr_head;
#endif
void osd_region_init(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info);
void osd_surface_init(struct osd_surface_info *p_surface_info,struct osd_rsc_info *p_rsc_info);
OSD_DRAW_DEV_MODE osd_get_draw_mode();
void osd_set_draw_mode(OSD_DRAW_DEV_MODE mode_chg);
void osd_set_whole_obj_attr(POBJECT_HEAD   p_obj, UINT8 b_attr);

void osd_move_object(POBJECT_HEAD p_obj, short x, short y, BOOL is_relative);
BOOL osd_clear_screen2(void);
void osd_small_region_init(struct osd_region_info *p_region_info, struct osd_rsc_info *p_rsc_info);
BOOL osd_switch_region(VSCR *p_vscr,UINT32 new_color_mode);
BOOL osd_multiview_switch_region(struct osd_region_info *p_region_info,struct osd_rsc_info *p_rsc_info);
enum osdcolor_mode osd_get_cur_color_mode(void);
enum osdcolor_mode osd_set_cur_color_mode(enum osdcolor_mode color_mode);

#ifdef __cplusplus
}
#endif
#endif//_OSD_LIB_H_
