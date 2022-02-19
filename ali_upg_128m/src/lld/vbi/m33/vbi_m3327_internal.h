/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327_internal.h

   *    Description:define the MACRO,variable and function using by VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __VBI_M3327_INTERNAL_H__
#define __VBI_M3327_INTERNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <osal/osal.h>
#include <basic_types.h>
#include <hld/vbi/vbi_dev.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>


#define VBI_CHARACTER_SPACE 0x20
#define VBI_CHARACTER_MAX_VALUE 0x7F
#define VBI_CHARSET_CYRILLIC_1 0x10
#define VBI_CHARSET_CYRILLIC_2 0x20
#define VBI_CHARSET_CYRILLIC_3 0x30
#define VBI_CHARSET_GREEK 0x40
#define VBI_CHARSET_ARABIC 0x50
#define VBI_CHARSET_HEBREW 0x60
#define VBI_RET_LOOP_BEFORE 3

#define TTX_MAX_ROW             24
#define TTX_MAX_COLUMN             40
#define TTX_LAST_ROW_NUM         23

#define TTX_TOTAL_PAGE          799
#define TTX_TOTAL_PAGE_NUM         800//0-799
#define TTX_SUB_PAGE_MAX_NUM     79
#define TTX_SUB_PAGE_TOTAL_NUM     80//0-79
#define TTX_SUB_PAGE_MIN_NUM     0
#define TTX_TOTAL_MAGZINE         8
#define TTX_MAX_MAGZINE_NUM     7//0-7
#define MAX_TENS_NUM             9
#define TTX_INVALID_MAG         0xF
#define TTX_SUBT_PAGE_CHANGE_TIME 2000

#define TTX_TRIPLET_TOTAL_NUM     13
#define TTX_TRIPLET_MAX_NUM     12//0-12
#define TTX_PAGE_CHANGE_TIME     500
#define TTX_COLOR_NUMBER_16     16

#define TTX_NORMAL_0 0
#define TTX_NORMAL_1 1
#define TTX_NORMAL_2 2
#define TTX_NORMAL_3 3
#define TTX_NORMAL_4 4
#define TTX_NORMAL_5 5

#define TTX_NORMAL_7 7
#define TTX_NORMAL_8 8
#define TTX_NORMAL_10 10

#define TTX_NORMAL_22 22
#define TTX_NORMAL_32 32
#define TTX_NORMAL_42 42
#define TTX_NORMAL_44 44
#define TTX_NORMAL_127 127

#define TTX_PACKET_0 0
#define TTX_PACKET_29 29

#define TTX_PACKET_26 26

#define G0_SET_0 0
#define G0_SET_1 1
#define G0_SET_2 2
#define G0_SET_3 3
#define G0_SET_4 4
#define G0_SET_5 5
#define G0_SET_6 6
#define G0_SET_7 7
#define G0_SET_8 8

#define TTX_NATION_0 0
#define TTX_NATION_1 1
#define TTX_NATION_2 2
#define TTX_NATION_3 3
#define TTX_NATION_4 4
#define TTX_NATION_5 5
#define TTX_NATION_6 6
#define TTX_NATION_7 7

#define TTX_LANG_LEN 3
#define TTX_QUEUE_TAG_4 4
#define TTX_QUEUE_TAG_3 3
#define TTX_QUEUE_TAG_2 2
#define TTX_QUEUE_TAG_1 1

#define TTX_PAGE_MIN_NUM 100
#define TTX_PAGE_MAX_NUM 899

#define TTX_TRANSPARENT_LEVEL        4// 4 // 6 //16
#define TTX_TRANSPARENT_STEP        15/(TTX_TRANSPARENT_LEVEL-1)
#define TTXENG_OFF        0

#ifdef TTX_BY_OSD
#define SET_P26_NATION_MAP(page, line, i, v) \
        g_ttx_p26_nation[CALC_POS(page, line, i)] =\
        ((g_ttx_p26_nation[CALC_POS(page, line, i)]&\
        (0xf<<(4-(i&1)*4)))|(v<<((i&1)*4)))

#define GET_P26_NATION_MAP(page, line, i) ((g_ttx_p26_nation[CALC_POS(page, line, i)]>>((i&1)*4))&0xf)
#endif

//begin
extern UINT8 *g_vbi27_cc_by_osd;
extern struct vbidata_hdr *vbi_data_hdr;
extern struct vbi_data_array_t *vbi_data_array;
extern struct vbi_m3327_private *g_vbi_priv;
extern struct vbi_config_par *g_vbi27_pconfig_par;
extern UINT8  *g_vbi27_ttx_by_osd;
extern UINT8  *g_ttx_p26_nation;
extern OSAL_ID ttx_subt_alarm_id;
extern t_ttxdec_cbfunc p_cbpage_update;
extern UINT8 g_packet_exist[8][25];
extern char vbi_m3327_name[HLD_MAX_NAME_SIZE];
extern struct PBF_CB *p_cur_rd_cb;
extern UINT16 req_page; //..100 - 899
extern UINT16 page_queue[4] ;
extern UINT8 queue_tag;
extern UINT8 screen_transparent_flag;
extern UINT8 screen_transparent_flag2;
extern UINT16 wait_sub_page;//0...79
extern UINT16 cur_subpage;//0...79
extern BOOL b_upd_page;
extern UINT8 status_line_data[40];//to show subpage status
extern BOOL new_history_mode;//
//extern UINT8 last_draw_data[24][40];//line1 - line24, not include line 0
extern BOOL b_useful_data;
extern UINT32 end_page_line_num;
extern UINT16 cur_page;
extern UINT16 wait_page;
extern struct t_ttx_lang g_init_page[TTX_SUBT_LANG_NUM];
extern struct t_ttx_lang g_subt_page[TTX_SUBT_LANG_NUM];
extern UINT8 g_subt_page_num;

#ifdef TTX_FLASH_FUNC
extern UINT16 same_cur_subpage;
#endif
//end
#ifdef SEE_CPU
extern OSAL_ID TTX_SEM_ID;
#endif

#ifdef CC_BY_OSD
extern UINT16 *g_vbi27_dtvcc_by_osd;
#endif

extern UINT8 transparent_value[TTX_TRANSPARENT_LEVEL];

extern UINT32 get_subpage_nation_map(UINT16 subpage, UINT16 line, UINT16 i);
extern void show_ttx_info();
extern void ttx_eng_set_seperate_ttxsubt_mode(struct vbi_device *ttx_dev, UINT8 mode);
extern void ttx_eng_set_ttx_history_mode(struct vbi_device *ttx_dev, BOOL mode);
extern void ttx_eng_set_ttx_dsg_font(UINT32 idx);
extern void clear_sub_page_cb(void);
extern INT32 set_sub_page_cb(UINT16 sub_page_id, struct PBF_CB *cb,
    UINT8 start_line_num, UINT8 end_line_num, BOOL show_status);
extern INT32 ttx_request_sub_page_up(UINT16 page_id , UINT16 sub_page_id, struct PBF_CB **cb );
extern INT32 ttx_request_sub_page_down(UINT16 page_id , UINT16 sub_page_id, struct PBF_CB **cb );
extern void show_subpage_status(void);
extern void parse_line(struct PBF_CB *cb, UINT8 line_num,UINT8 skip_start8byte,UINT8 skip_end8byte);
extern void ttx_eng_send_key(struct vbi_device *ttx_dev,TTX_KEYTYPE key);
extern void vbi_m3327_init_ttx_decoder(void);
extern void vbi_m3327_init_cc_decoder(struct vbi_device *dev);
extern BOOL vbi_line21_push(UINT16 data);
extern BOOL vbi_line21_push_dtvcc(UINT16 data);//xing for DTVCC
extern UINT32 get_packet26_nation_map(UINT16 page, UINT16 line, UINT16 i);
extern void vbi_disable_ttx_by_osd(void);
//extern void lld_vbi_m33_callee(UINT8 *msg);
extern UINT16 get_first_ttx_page(void);
extern BOOL check_displayable_char(UINT8 *data, UINT8 len);
extern void language_select(UINT8 g0_set,UINT8 nation,UINT8 *alpha);
extern void language_select_g2(UINT8 g0_set,UINT8 nation,UINT8 *alpha);
extern UINT8 key2char(TTX_KEYTYPE key);
extern void set_packet24_new(struct vbi_device *ttx_dev, UINT16 last_page_num);
extern void parse_line_ext(struct PBF_CB *cb);
extern UINT8 get_mosaic_mode(void);

extern INT32 ttx_eng_open(struct vbi_device *ttx_dev, UINT8 subtitle,UINT16 page_num);
extern void  ttx_eng_close(struct vbi_device *ttx_dev);
extern void ttx_eng_update_page(struct vbi_device *ttx_dev, UINT16 page_id, UINT8 line_id);
extern void  ttx_eng_show_onoff(struct vbi_device *ttx_dev, BOOL b_on);
extern UINT8 ttx_eng_get_state(struct vbi_device *ttx_dev);
extern void ttx_eng_update_init_page(struct vbi_device *ttx_dev, UINT8 num, UINT32 page_addr);
extern void ttx_eng_update_subt_page(struct vbi_device *ttx_dev, UINT8 num, UINT32 page_addr);
extern void ttx_eng_set_cur_language(UINT32 language);
extern BOOL get_b_save_subpage(void);

void ttx_delete_whole(void);
#ifdef __cplusplus
 }
#endif


#endif





