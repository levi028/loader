
#include <sys_config.h>
#ifdef PUB_PORTING_PUB29


#include <api/libpub/lib_mv.h>

#else

#ifndef __MID27_MULTIVIEW_HEAD_FILE_H__
#define __MID27_MULTIVIEW_HEAD_FILE_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define BOX_FINISHED    0x100
#define BOX_FOCUS        0x200
#define BOX_SCAN_START 0x1000
#define BOX_PLAY_ABORT        0xFFFF
#define BOX_PLAY_NORMAL    0x0001
#define MV_SCAN_BOX        0x40
#define MV_SCAN_SCREEN        0x20
#define MV_ACTIVE            0x10
#define MV_IDLE                0x0

#define MV_BOX_NUM            25 //max number of boxes in one frame.


enum mvmode
{
    MV_2_BOX=0,
    MV_4_BOX,
    MV_9_BOX,
    MV_16_BOX,
    MV_25_BOX,
    MV_AUTO_BOX
};

typedef unsigned long (* mvcallback)(UINT16 u_index);

struct mvinit_para
{
    struct rect_size    t_mp_size,t_pip_size;
    UINT8             u_box_num;//total number of boxes in the whole picture;
    //pointer of the coordination array for all boxes;(placement sequence index is left to right, up to down)
    struct position    pu_box_posi[MV_BOX_NUM];
    mvcallback        callback;
    struct rect         play_win;
};

struct multi_view_para
{
    UINT8     u_box_active_idx;//active box index
    UINT8     u_box_num;//total number of boxes in the whole picture;
    UINT16    u_prog_info[MV_BOX_NUM];
};


void mv_init(void);

void uimultiview_init(struct mvinit_para *t_mvinit_para, enum mvmode mv_mode);
void uimultiview_enter(struct multi_view_para *mv_para);
BOOL uimultiview_draw_one_box(UINT16 u_active_index, UINT8 b_enable_san_draw);
void uimultiview_exit();
UINT8 uimultiview_get_status(void);

#ifdef __cplusplus
}
#endif


#endif


#endif
