/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_closecaption_internal.h

   *    Description: define MACRO,data and function used by ATSC CC
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __LIB_CLOSECAPTION_INTERNAL_H__
#define __LIB_CLOSECAPTION_INTERNAL_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define CLOSE_CAPTION_NORMAL_0 0
#define CLOSE_CAPTION_NORMAL_1 1
#define CLOSE_CAPTION_NORMAL_2 2
#define CLOSE_CAPTION_NORMAL_3 3
#define CLOSE_CAPTION_NORMAL_4 4
#define CLOSE_CAPTION_NORMAL_5 5
#define CLOSE_CAPTION_NORMAL_6 6
#define CLOSE_CAPTION_NORMAL_7 7
#define CLOSE_CAPTION_NORMAL_8 8
#define CLOSE_CAPTION_NORMAL_11 11

#define CLOSE_CAPTION_NORMAL_35 35
#define CLOSE_CAPTION_MAX_WIN_NUM 8
#define CC_CHARACTER_0 0x20
#define CC_CHARACTER_1 0x2a
#define CC_CHARACTER_2 0x5c
#define CC_CHARACTER_3 0x5e
#define CC_CHARACTER_4 0x5f
#define CC_CHARACTER_5 0x60
#define CC_CHARACTER_6 0x7b
#define CC_CHARACTER_7 0x7c
#define CC_CHARACTER_8 0x7d
#define CC_CHARACTER_9 0x7e
#define CC_CHARACTER_A 0x23
#define CC_CHARACTER_B 0x89
#define CC_CHARACTER_C 0x10A0

#define CC_INDICATOR_1 0x47
#define CC_INDICATOR_2 0x41
#define CC_INDICATOR_3 0x39
#define CC_INDICATOR_4 0x34
#define CC_DATA_INDICATOR 0x03
#define CC_DATA_FLAG 0x40

#define VDEC_CC_HEARDER_1 0x00
#define VDEC_CC_HEARDER_2 0x00
#define VDEC_CC_HEARDER_3 0x00
#define VDEC_CC_HEARDER_4 0x01
#define VDEC_CC_HEARDER_5 0x06

#define VDECH265_CC_HEARDER_1 0x00
#define VDECH265_CC_HEARDER_2 0x00
#define VDECH265_CC_HEARDER_3 0x01
#define VDECH265_CC_HEARDER_4 0x4E

#define CC_WIN_CMD_MAX_CNT 0x80
#define CC_NO_VBI_RATING_MAX_CNT 0x1fff
#define CC_CGMSA_INVALID_FLAG 0x0f
#define CC_MIDROW_CODE_1 0x11
#define CC_MIDROW_CODE_2 0x19
#define CC_XDS_INVALID_FLAG 0x0f
#define CC_NO_USEFUL_MAX_DATA 0x0f
#define CC_CTRL_MIN_DATA_SKIP_1 0x11
#define CC_CTRL_MAX_DATA_SKIP_1 0x17
#define CC_CTRL_MIN_DATA_SKIP_2 0x11
#define CC_CTRL_MAX_DATA_SKIP_2 0x17

#define MAX_EAS_LEN 256
#define SUPPORT_PAL
#define CMD_ERASE_DISPLAYED_MEMORY_NONDIS_CH1   0x94
#define CMD_ERASE_DISPLAYED_MEMORY_NONDIS_CH2   0x1C
#define CMD_ERASE_DISPLAYED_MEMORY_DIS  0x2C

#define LINE21_DEPTH_DTVCC 4096//for ATSC cc
#define DEPTH_SERVICE_BUFF  128
#define CC_MAX_REGION_NUM 8

#define CC_FIELD_1 1
#define CC_FIELD_2 2

#define CC_TOTAL_LINE 15
#define CC_MAX_LINE 14

#define DTVCC_TOTAL_FLASH_ID 20
#define DTVCC_MAX_FLASH_ID 19
#define DTVCC_MAX_PARSE_COUNT 2000
#define CC_MAX_PARSE_COUNT 1000
#define CC_MAX_TIME_FRESHOLD 2000


#define CC_PUSH_TOTAL_COUNT 45
#define CC_FUNC_RET_RETURN 0
#define CC_FUNC_RET_NORMAL 1
#define CC_FUNC_RET_LOOP 2
#define USER_DATA_LENGTH_MAX    210//104//xing for DTVCC
#define USER_DATA_ARRAY_MAX 10
#define CC_MAX_COL 32
#define CC_MAX_ROW 15

/* foreground FLASHING */
struct dtvcc_flash_data_cell
{
    UINT8 start_x;
    UINT8 y;
    UINT8 end_x;
    UINT8 fg_color;
    UINT8 bg_color;
    UINT8 underline;
    UINT8 italics;
    UINT8 string[32];
};

extern INT32 g_user_data_array_rd;
extern struct vbi_config_par *ccg_vbi27_pconfig_par;
//extern struct vbi_m3327_private *g_vbi_priv;
extern struct vbi_config_par *g_vbi27_pconfig_par;
extern BOOL b_ccrecv_pac;
extern UINT8 *ccg_vbi27_cc_by_osd;
extern struct atsc_cc_config_par g_atsc_cc_config_par;
extern BOOL ccg_disable_process_cc;
extern BOOL cc_block_start;//a patch for sarnoff c13, c14
extern BOOL cr_b2t;
extern BOOL last_block_byte;
extern BOOL defined_windows;
extern UINT8 dtvcc_pos_x;
extern UINT8 dtvcc_pos_y;
extern UINT8 g_dtv_cc_service;
extern INT16 wr_dtvcc;
extern INT16 rd_dtvcc;
extern INT16 cnt_dtvcc;
extern UINT8 g_cc_windows_cur_win;
extern BOOL  first_defined;
/* foreground FLASHING */
extern BOOL fgflashing;
extern BOOL bgflashing;
extern UINT8 dtvcc_flash_idx;
extern UINT8 dtvcc_flash_data_idx;
/* foreground FLASHING */
extern DWORD g_cc_delay;
extern BOOL CMD_DELAY;
extern UINT16 pop_on_string_dtvcc[8][15][36];
extern UINT8 pop_on_string_dtvcc_fg[8][15][36];//fg color
extern UINT8 pop_on_string_dtvcc_bg[8][15][36];//bg color
extern UINT8 pop_on_string_dtvcc_ud[8][15][36];//under line
extern UINT8 pop_on_string_dtvcc_itl[8][15][36];//italic
extern UINT8 pop_on_valid_dtvcc[8][15];
extern UINT8 pop_on_startx_dtvcc[8][15];
extern UINT8 pop_on_endx_dtvcc[8][15];
extern DWORD dtvcc_timestamp;
extern UINT16 *ccg_vbi27_dtvcc_by_osd;
extern UINT32 ccvbi_rating;
extern UINT32 ccpre_vbi_rating;
extern BOOL ccvbi_cc;
extern BOOL clean_cc_buffer;
extern BOOL g_dtvcc_war;
//extern UINT8 g_dtv_cc_service;
extern UINT8 ccvbi_cgmsa;   //for cgmsa
extern unsigned char bsn_cc[33]; // total + 2 * 16
extern UINT8 bcontrol;
extern void cc_osd_enter();
extern void cc_osd_leave();
extern UINT8 trans_rgbto_index(UINT8 r,UINT8 g,UINT8 b);
extern INT16 push_service_data(UINT8 data);
extern INT16 pop_service_data(UINT8 *data);
extern BOOL vbi_line21_pop_dtvcc(UINT16 *data);
extern void add_sn(unsigned char bno);
extern void gather_sn(void);
extern void remove_sn(void);
extern void scan_sn(void);
extern void display_popup_window(UINT8 windows_id, UINT8 line_start, UINT8 line_end);
extern BOOL cc_cmd_set_pen_color(UINT8 param);
extern BOOL cc_cmd_define_windows(UINT8 param);
extern void cc_cmd_current_windows(UINT8 param);
extern BOOL cc_cmd_clear_windows(UINT8 param);
extern BOOL cc_cmd_del_windows(UINT8 param);
extern BOOL cc_cmd_display_windows(UINT8 param);
extern BOOL cc_cmd_hide_windows(UINT8 param);
extern BOOL cc_cmd_tog_windows(UINT8 param);
extern BOOL cc_cmd_set_win_attr(UINT8 param);
extern BOOL cc_cmd_set_pen_atr(UINT8 param);
extern BOOL cc_cmd_set_pen_loc(UINT8 param);
extern BOOL cc_cmd_delay(UINT8 param);
extern void cc_cmd_reset(void);
extern void cc_set_pen_style_id (UINT8 windows_id, UINT8 param);
extern void cc_set_window_style_id (UINT8 windows_id, UINT8 param);
extern void dtvcc_init(void);
extern void set_eas_onoff(BOOL on_off, UINT8 *text, UINT16 len);
extern BOOL get_eas_onoff(void);
extern void CC_VSCR_CLEAN(void);
extern void cc_fill_char(UINT16 x, UINT16 y,UINT16 w,UINT16 h,UINT8 bg_color);
extern void cc_carriage_return(UINT8 start_row,UINT8 end_row, UINT8 dtvcc_width , UINT8 win_color , BOOL text_flag);
extern void cc_move_intact(UINT8 start_row, UINT8 des_row ,UINT8 num_row);
extern void osd_cc_init(void);
extern void dtvcc_carriage_return(UINT8 start_row,UINT8 end_row,
    UINT8 dtvcc_width , UINT8 win_color , UINT8 start_col, UINT8 end_col, UINT8 win_scroll_dir);
extern void dtvcc_modify_osdopacity(UINT8 color_idx, UINT8 opa_level);
extern void disable_process_cc(BOOL b_on);
extern void disable_vbioutput(BOOL b_on);
extern void set_cc_by_vbi(BOOL b_on);
extern void cc_vbi_show_on(BOOL b_on);
extern INT32 atsc_cc_show_onoff(BOOL b_on);  //vicky20110124
extern UINT8 get_vbi_cgmsa(void);//for cgmsa
extern void set_vbi_cgmsa_none();//for cgmsa
extern UINT32 get_vbi_rating(void);
extern void set_vbi_rating_none();
extern BOOL get_vbi_cc(void);
extern void cc_coding_layer_parsing(UINT8 data);
extern void miscellaneous_command_dtvcc(UINT8 data);
extern BOOL cc_push_field(UINT8 field);
extern void eas_on_off(BOOL on_off, UINT8 *text, UINT16 len);
extern struct dtvcc_flash_data_cell dtvcc_flash_data[20];
extern struct ccwindow m_ccwindow[CC_MAX_REGION_NUM];
#ifdef __cplusplus
 }
#endif



#endif /*__LIB_CLOSECAPTION_INTERNAL_H__ */

