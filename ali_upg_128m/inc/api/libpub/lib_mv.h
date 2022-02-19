#ifndef __MULTIVIEW_HEAD_FILE_H__
#define __MULTIVIEW_HEAD_FILE_H__

#include"lib_frontend.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define BOX_FINISHED            0x100
#define BOX_FOCUS        0x200
#define BOX_SCAN_START          0x1000
#define BOX_PLAY_ABORT        0xFFFF
#define BOX_PLAY_NORMAL            0x0001
#define MV_SCAN_BOX        0x40
#define MV_SCAN_SCREEN        0x20
#define MV_ACTIVE        0x10
#define MV_IDLE            0x0

#define MV_BOX_NUM        25 //max number of boxes in one frame.

enum mvmode
{
    MV_2_BOX=0,
    MV_4_BOX,
    MV_9_BOX,
    MV_16_BOX,
    MV_25_BOX,
    MV_AUTO_BOX    ,
    MV_4_1_BOX,
    MV_5_1_BOX,
    MV_12_1_BOX
};

typedef unsigned long (* mvcallback)(UINT16 u_index);
typedef unsigned long (* simcallback)(UINT16 u_index);

struct mvinit_para
{
    struct rect_size    t_mp_size,t_pip_size,t_mpip_size;
    UINT8             u_box_num;//total number of boxes in the whole picture;
    struct position    pu_box_posi[MV_BOX_NUM],t_mpip_box_posi[MV_BOX_NUM];
    mvcallback    callback;
    simcallback avcallback;
    struct rect     play_win;
};

struct mv_param
{
    struct cc_es_info        es;
    struct cc_device_list    device_list;
    struct ft_frontend        ft;
};

struct multi_view_para
{
    UINT8     u_box_active_idx;//active box index
    UINT8     u_box_num;//total number of boxes in the whole picture;
    UINT16    u_prog_info[MV_BOX_NUM];
    struct mv_param mv_param[MV_BOX_NUM];
    UINT8    mvmode;
    UINT32 multi_view_buf_addr;
    UINT32 multi_view_buf_size;
};

void mv_init(void);

void uimultiview_init(struct mvinit_para *t_mvinit_para, enum mvmode mv_mode);
void uimultiview_init_ext(struct mvinit_para *mv_init_para, enum mvmode mv_mode, BOOL play_in_spe_win);
void uimultiview_enter(struct multi_view_para *mv_para);
void uimultiview_draw_one_box(UINT16 active_idx);//, UINT8 bEnableSanDraw);
void uimultiview_exit();
UINT8 uimultiview_get_status(void);
BOOL get_mv_exit(void);
void set_box_play_status(BOOL flag);
BOOL get_box_play_status(void);
void mv_update_play_para(struct multi_view_para *mv_para);

#ifdef __cplusplus
}
#endif

#endif

