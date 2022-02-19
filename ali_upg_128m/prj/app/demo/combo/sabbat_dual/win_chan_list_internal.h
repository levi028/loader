/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_list_internal.c
*
*    Description: To realize the UI for user view & edit the channel
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __WIN_CHAN_LIST_INTERNAL__
#define __WIN_CHAN_LIST_INTERNAL__

#ifdef __cplusplus
extern "C" {
#endif

#define FRONTEND_TYPE_S    0x00
#define FRONTEND_TYPE_C    0x01
#define FRONTEND_TYPE_T         0x02
#define FRONTEND_TYPE_ISDBT     0x03
#define BORDER_WIDHT    2       //14

#ifndef SD_UI
#define W_L     335//74//24//170 //0
#define W_T     100//30//sharon 98//44 //112//152//84  //84  //70
#ifdef SUPPORT_CAS_A
#define W_W     886
#else
#define W_W    944// 866
#endif
#define W_H     619//556//sharon 488//558 //334

#define CH_3BTN_L   (W_L + 80)
#define CH_4BTN_L   (W_L + 50)
#define CH_5BTN_L   (W_L + 19)
#define CH_3BTN_GAP 100
#define CH_4BTN_GAP 15
#define CH_5BTN_GAP 100 // 4

#define TITLE_BMP_L     (W_L+14)
#define TITLE_BMP_T     W_T//(W_T - TITLE_BMP_H)//30
#define TITLE_BMP_W     330//W_W //80//W_W
#define TITLE_BMP_H     68 //40

#define CH_GRP_L     570//(W_L+14)
#define CH_GRP_T    102//(TITLE_BMP_T + TITLE_BMP_H)
#define CH_GRP_W    200//494//sharon W_W
#define CH_GRP_H    40//36 //30
#define CH_GRPTXT_H CH_GRP_H

#define CH_BTNB_L   340 //CH_GRP_L
#define CH_BTNB_T   650//(CH_GRP_T + CH_GRP_H)
#define CH_BTNB_W   900//CH_GRP_W//sharon W_W
#define CH_BTNB_H   40//50 //50//38

#define CH_BTN_T    (CH_BTNB_T+2)
#define CH_BTN_W    92
#define CH_BTN_H    36//46

#define PREVIEW_L   810//(W_L + CH_GRP_W+28)//364
#define PREVIEW_T   105//(TITLE_BMP_T + TITLE_BMP_H) //34
#define PREVIEW_W   445//445//320//sharon 306//(204 + 40)
#define PREVIEW_H   250//190//(156+40)

#define INFO_L  PREVIEW_L  //364
#define INFO_T  615//(PREVIEW_T + PREVIEW_H )//(190+40)
#define INFO_W  330 //244
#define INFO_H  40//260//sharon 314 //(212 - 40 + 11)

#define INFO_TXT_L  (INFO_L+30)
#define INFO_TXT_T  615//(INFO_T + 70)
#define INFO_TXT_W  330
#define INFO_TXT_H  40

#else

#define W_L     17     //24//170 //0
#define W_T     17     //sharon 98//44 //112//152//84  //84  //70
#define W_W     570    //sharon 560 //866 //364
#define W_H     410    //sharon 488//558 //334

#define CH_3BTN_L   (W_L + 60)
#define CH_4BTN_L   (W_L + 36)
#define CH_5BTN_L   (W_L + 11)
#define CH_3BTN_GAP 20
#define CH_4BTN_GAP 12
#define CH_5BTN_GAP 0

#define TITLE_BMP_L     (W_L+120)
#define TITLE_BMP_T     W_T  //(W_T - TITLE_BMP_H)//30
#define TITLE_BMP_W     200  //W_W //80//W_W
#define TITLE_BMP_H     40   //40

#define CH_GRP_L    (W_L+9)
#define CH_GRP_T    (TITLE_BMP_T + TITLE_BMP_H+8)
#define CH_GRP_W    350  //sharon W_W
#define CH_GRP_H    30   //30
#define CH_GRPTXT_H 24

#define CH_BTNB_L   CH_GRP_L
#define CH_BTNB_T   (CH_GRP_T + CH_GRP_H)
#define CH_BTNB_W   CH_GRP_W    //sharon W_W
#define CH_BTNB_H   36          //50//38

#define CH_BTN_T    (CH_BTNB_T)
#define CH_BTN_W    68
#define CH_BTN_H    34

#define PREVIEW_L   (W_L + CH_GRP_W+14)             //364
#define PREVIEW_T   (TITLE_BMP_T + TITLE_BMP_H + 8) //34
#define PREVIEW_W   188    //sharon 306//(204 + 40)
#define PREVIEW_H   176    //206//(156+40)

#define INFO_L  PREVIEW_L  //364
#define INFO_T  (PREVIEW_T + PREVIEW_H )   //(190+40)
#define INFO_W  244        //244
#define INFO_H  172        //sharon 314 //(212 - 40 + 11)

#define INFO_TXT_L  (INFO_L)
#define INFO_TXT_T  (INFO_T + 14)
#define INFO_TXT_W  190
#define INFO_TXT_H  24

#endif

typedef struct
{
    UINT8 btn_cnt;
    UINT8 *btn_idx;
    UINT8 *btn_vkey;
    UINT8 *btn_act;
    UINT8 *btn_colidx;
    UINT16 *btn_stridx;
}chan_edit_grp_t;

typedef enum
{
    VACT_FAV = (VACT_PASS + 21),
    VACT_LOCK,
    VACT_SKIP,
    VACT_MOVE,
    VACT_ENTEREDIT,
    VACT_SORT,
    VACT_EDIT,
    VACT_DEL,
    VACT_ADD,
}VACT_EX;

typedef enum
{
    CHAN_EDIT_NONE = 0,
    CHAN_EDIT_FAV,
    CHAN_EDIT_LOCK,
    CHAN_EDIT_DEL,
    CHAN_EDIT_SKIP,
    CHAN_EDIT_MOVE,
    CHAN_EDIT_ENTEREDIT,
    CHAN_EDIT_SORT,
    CHAN_EDIT_EDIT,
    CHAN_EDIT_ADD,
}CHAN_EDIT_TYPE_T;

extern OBJLIST      chlst_ol;
extern TEXT_FIELD   chlist_infor_txt;
extern TEXT_FIELD   chlst_preview_txt;
extern BITMAP       chlist_infor_bmp;
extern CONTAINER  chlst_btn_con;

extern UINT8  chan_edit_grop_idx;
extern UINT8  chan_edit_type_btnidx;
extern UINT16 move_channel_idx;
extern BOOL   pwd_valid_status;
extern UINT32 play_chan_id;
extern UINT8  m_sorted_flag;
extern UINT32 del_channel_idx[MAX_PROG_NUM/32 + 1];
extern chan_edit_grp_t *chan_edit_grps[];
extern TEXT_FIELD *btn_txtobj[];

//void gen_fft_modulation_str(T_NODE *t_node, char *str);
//void gen_gi_fec_str(T_NODE *t_node, char *str);
//void set_tnode_tuner_param(struct nim_device *priv_nim_dev, T_NODE *t_node);
void vact_enter_proc(UINT16 sel, UINT16 ch_cnt, OBJLIST *ol, COM_POP_LIST_PARAM_T *param, OSD_RECT *rect);
void vact_del_proc(chan_edit_grp_t *pcheditgrp, VACTION act, COM_POP_LIST_PARAM_T *param, OSD_RECT *rect, OBJLIST *ol);
UINT16 list_index_2_channel_index(OBJLIST *ol,UINT16 list_idx);
void win_chlist_set_btn_display(BOOL update);
void win_chlist_set_btn_display2(BOOL update, UINT8 mode);
BOOL win_chlist_save_sorted_prog(void);
void win_chlst_draw_channel_infor(void);
void win_chlst_get_preview_rect(UINT16 *x,UINT16 *y, UINT16 *w, UINT16 *h);
void win_chlst_play_channel(UINT16 chan_idx);

#ifdef __cplusplus
}
#endif

#endif
