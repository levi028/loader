/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_COMMON_DRAW_H_
#define _OSD_COMMON_DRAW_H_
#include "osd_primitive.h"

enum gui_const
{
    VSCR_DIRECTDRAW = 0,
    VSCR_REQBUF,
    VSCR_GET_CORNER,
    VSCR_GET_BACK,
    VSCR_FILL_BACK,
    VSCR_NULL = 10,

    C_NOSHOW = 0xff68d7ebUL,
    C_NOSHOW_TEXT = 0xfe68d7ebUL,
    C_MIXER = 0x80000000UL,//(0x1<<31)

    C_SINGLE_LINE = 0x02000000UL,
    C_DOUBLE_LINE = 0x04000000UL,
    C_TRIPLE_LINE = 0x06000000UL,
    C_FOURFOLD_LINE = 0x08000000UL,
    C_WS_LINE_MASK = 0x0f000000UL,

    C_WS_LINE_DRAW = 0xf2000000UL,//default line width is 2
    C_WS_LINE_CIRCL_DRAW = 0xe2000000UL,//default line width is 2
    C_WS_PIC_DRAW = 0xd0000000UL,
    C_WS_USER_DEFINE = 0xc0000000UL,//lower bits reserved for 32bit color flag
    C_WS_TYPE_MASK = 0xf0000000UL,

    INVALID_INDEX = 0xffffffffUL,
};

typedef UINT32 ID_RSC;

typedef struct gui_wstyle{
    UINT32 b_win_style_type;
    UINT32 w_top_line_idx;
    UINT32 w_left_line_idx;
    UINT32 w_right_line_idx;
    UINT32 w_bottom_line_idx;
    UINT32 w_bg_idx;
    UINT32 w_fg_idx;
    UINT32 w_left_top_idx;
    UINT32 w_left_buttom_idx;
    UINT32 w_right_top_idx;
    UINT32 w_right_buttom_idx;
    UINT32 icon_lib_idx;
}WINSTYLE, *PWINSTYLE;

enum wstyle_ids
{
    WID_TOP = 0,
    WID_LEFT,//1
    WID_RIGHT,//1
    WID_BTM,
    WID_BG,
    WID_FG,
    WID_LTOP,
    WID_LBTM,
    WID_RTOP,
    WID_RBTM,
    WID_FGMIX,/*10*/
    WID_BGMIX,/*11*/
    WID_CNDRW,/*Corner bitmap directly draw not color on background*/
    WID_RSV1,/*13*/
    WID_RSV2,
    WID_MAX = 15,//16bits used for flag
};

//comments for this
typedef struct wstyle_type
{
    UINT32    wstyle_format:8;//such as:C_WS_LINE_DRAW etc
    UINT32    rsv:8;//reserved now
    UINT32    wid_max:1;//WID_MAX,in fact it's reserved
    UINT32    wid_rsv1:1;
    UINT32    wid_rsv2:1;
    UINT32    wid_rsv3:1;
    UINT32    wid_bgmix:1;
    UINT32    wid_fgmix:1;
    UINT32    wid_rbtm:1;
    UINT32    wid_rtop:1;
    UINT32    wid_lbtm:1;
    UINT32    wid_ltop:1;
    UINT32    wid_fg:1;
    UINT32    wid_bg:1;
    UINT32    wid_btm:1;
    UINT32    wid_right:1;
    UINT32    wid_left:1;
    UINT32    wid_top:1;
}WUINT32;//bWinStyleType will be parsed as above in 32bits UI

#define wstyle_setline(drawstyle,linestyle)    (drawstyle|linestyle)
#define wstyle_getline(bwstyletype)    ((bwstyletype&C_WS_LINE_MASK)>>24)

/*Only valid for 32bits UI*/
#define wstyle_getflag(wstyle,idx)    ((((wstyle->b_win_style_type)&(0x1<<idx))>0)?1:0)
#define wstyletype_getflag(bwstyletype,idx)    (((bwstyletype&(0x1<<idx))>0)?1:0)
#define wstyle_setflag(idx)    (0x1<<idx)

#define wstyle_merge(type,t,l,r,b,bg,fg,lt,lb,rt,rb,fgm,bgm,rsv1,rsv2,rsv3,max) (((type) | (t&0x01) | ((l&0x01)<<1) | ((r&0x01)<<2) \
     | ((b&0x01)<<3) | ((bg&0x01)<<4) | ((fg&0x01)<<5) | ((lt&0x01)<<6) | ((lb&0x01)<<7) | ((rt&0x01)<<8) | ((rb&0x01)<<9) | ((fgm&0x01)<<10) \
      | ((bgm&0x01)<<11) | ((rsv1&0x01)<<12) | ((rsv2&0x01)<<13) | ((rsv3&0x01)<<14) | ((max&0x01)<<15)))

//most common usage
#define wstylecm(type,t,l,r,b,bg,fg,lt,lb,rt,rb,fgm,bgm)  wstyle_merge(type,t,l,r,b,bg,fg,lt,lb,rt,rb,fgm,bgm,0,0,0,0)
#define wstylemix(type,fgmix,bgmix)    wstyle_merge(type,0,0,0,0,0,0,0,0,0,0,fgmix,bgmix,0,0,0,0)
#define wstyle_bgmixer(type)    wstylemix(type,0,1)
#define wstyle_cndrw(type)    ((type)|0x1000)
#define wstyle_getmixer(type,mix_idx) ((((type)&(0x1<<mix_idx))>0)?C_MIXER:0)

#ifndef GUI_COLOR_NOSHOW
#define GUI_COLOR_NOSHOW C_NOSHOW
#endif
#define wstyle_noshow(color)    ((color == GUI_COLOR_NOSHOW)?1:0)
#define wstyle_colorshow(color)    ((color == GUI_COLOR_NOSHOW)?0:1)
//--------------------------- Function Prototype ----------------------------//
UINT16 osd_draw_char2vscr(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch, UINT8 font, PGUI_VSCR p_vscr);
void osd_draw_rect2vscr(PGUI_RECT pfrm, UINT32 bg_color, PGUI_VSCR pvscr);
void osd_draw_bmp2vscr(PGUI_POINT pos,UINT32 styleidx,PGUI_VSCR p_vscr);
UINT16 osd_draw_text(PGUI_RECT p_rect, UINT8* p_text, UINT32 color, UINT8 assign_style, UINT8 font_size, PGUI_VSCR p_vscr);
void osd_draw_style_rect(PGUI_RECT p_rect, UINT32 b_sh_idx, PGUI_VSCR p_vscr);
UINT16 osd_draw_thai_cell(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, struct thai_cell *cell, UINT8 font, PGUI_VSCR p_vscr);

PGUI_RECT osd_clear_vscr(PGUI_VSCR pvscr);
RET_CODE osd_clear_scrn(PGUI_VSCR pvscr);
RET_CODE osd_update_vscr(PGUI_VSCR p_vscr);
PGUI_VSCR osd_get_task_vscr(ID task_id);
PGUI_VSCR osd_get_task_slvscr(ID task_id);
PGUI_VSCR osd_get_vscr(PGUI_RECT p_rect, UINT32 b_flag);
PGUI_VSCR osd_get_slvscr(PGUI_RECT p_rect, UINT32 b_flag);

#define osd_set_vscr_cfg(pvscr,param) \
    do{ \
        (pvscr)->root_layer = (UINT8)((param&0xff000000)>>24); \
        (pvscr)->root_rgn = (UINT8)((param&0xff0000)>>16); \
        (pvscr)->dst_layer = (UINT8)((param&0xff00)>>8); \
        (pvscr)->dst_rng = (UINT8)(param&0xff); \
    }while(0)

#define osd_set_vscr_layer(pvscr,rootlayer,rootrgn,dstlayer,dstrgn) \
    do{ \
        (pvscr)->root_layer = rootlayer; \
        (pvscr)->root_rgn = rootrgn; \
        (pvscr)->dst_layer = dstlayer; \
        (pvscr)->dst_rgn = dstrgn; \
    }while(0)

#define osd_set_vscr_frm(pvscr,prect) osd_copy_rect((&pvscr->frm),(prect))
#define osd_set_vscr_colormode(pvscr,color_mode)    \
    ((pvscr)->color_mode = color_mode)
#define osd_set_vscr_modified(pvscr)    \
    ((pvscr)->dirty_flag = 1)
#define osd_clear_vscr_modified(pvscr)    \
    ((pvscr)->dirty_flag = 0)
#define osd_set_vscr_suspend(pvscr)    \
    do{(pvscr)->suspend = 1;}while(0)
#define osd_clear_vscr_suspend(pvscr)    \
    ((pvscr)->suspend = 0)
#endif
