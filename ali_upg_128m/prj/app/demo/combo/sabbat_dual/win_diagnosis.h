/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_diagnosis.h
*
*    Description:   The root menu of Diagnosis Application
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_DIAGNOSIS_H_
#define _WIN_DIAGNOSIS_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <api/libosd/osd_lib.h>
#include <bus/i2c/i2c.h>

#include "win_com.h"
#include "osdobjs_def.h"

#include "osd_config.h"
#include "mobile_input.h"

#define WIN_DIAG_WIDTH      900//608
#define WIN_DIAG_HEIGHT     580//430
#define WIN_DIAG_LEFT       GET_MID_L(WIN_DIAG_WIDTH)
#define WIN_DIAG_TOP        GET_MID_T(WIN_DIAG_HEIGHT)

#define TXT_DIAG_LEFT       (WIN_DIAG_LEFT + 20)
#define TXT_DIAG_TOP        (WIN_DIAG_TOP + 30)
#define TXT_DIAG_TITLE_WIDTH (WIN_DIAG_WIDTH - 40)     //588

#define TXT_DIAG_LBL_WIDTH     160
#define TXT_DIAG_LBL_GAP     200
#define EDF_NUM_WIDTH         40
#define EDF_DATA_WIDTH         300

#define TXT_DIAG_HEIGHT     40  //28
#define TXT_DIAG_VGAP         50

#define BTN_DIAG_LEFT       (WIN_DIAG_LEFT \
                            + WIN_DIAG_WIDTH/2 \
                 - TXT_DIAG_LBL_WIDTH)
//TXT_DIAG_LEFT //180

#define LBL_DATA_LEN_LEFT     TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP+EDF_DATA_WIDTH+10

#define WSTL_DIAG_TEXT             WSTL_DIAG_BUTTON_01_8BIT //WSTL_BUTTON_01
#define WSTL_DIAG_BUTTON_01     WSTL_DIAG_BUTTON_01_8BIT //WSTL_BUTTON_01
#define WSTL_DIAG_BUTTON_02     WSTL_BUTTON_05_8BIT//WSTL_BUTTON_05
#define WSTL_DIAG_BUTTON_03     WSTL_BUTTON_05_8BIT //WSTL_BUTTON_04
#define WSTL_DIAG_BG_02         WSTL_DIAG_WIN_8BIT //WSTL_GAMEWIN_01
#define DIAG_WSTL_NO_SHOW       WSTL_NOSHOW_IDX

#define STR_I2C_TITLE_ID        0
#define STR_LBL_I2CID_ID        1
#define STR_LBL_ADDR_ID        2
#define STR_LBL_WDATA_ID        3
#define STR_LBL_WLEN_ID        4
#define STR_LBL_RDATA_ID        5
#define STR_LBL_RLEN_ID            6
#define STR_BTN_W_R_ID            7
#define STR_BTN_EXIT_ID            8
#define STR_DIAG_TITLE_ID         9
#define STR_BTN_REGISTER_ID     10
#define STR_BTN_I2C_ID             11

#define DIAG_STR_CNT             12
#define DIAG_STR_LENGTH         16

extern CONTAINER win_diag;
extern CONTAINER diag_i2c;
extern CONTAINER diag_register;

extern TEXT_FIELD txt_diag_title;

extern TEXT_FIELD btn_diag_register;
extern TEXT_FIELD btn_diag_i2c;
extern TEXT_FIELD btn_diag_exit;
extern UINT16 diag_unistr[DIAG_STR_CNT][DIAG_STR_LENGTH];

extern TEXT_FIELD txt_title_saddr;
extern TEXT_FIELD txt_title_maddr;
extern TEXT_FIELD txt_title_value;
extern TEXT_FIELD txt_title_read;
extern TEXT_FIELD txt_title_write;
extern TEXT_FIELD txt_title_exit;
extern EDIT_FIELD edit_saddr;
extern EDIT_FIELD edit_maddr;
extern EDIT_FIELD edit_value;

extern TEXT_FIELD txt_i2c_title;
extern TEXT_FIELD txt_i2c_id;
extern TEXT_FIELD txt_i2c_addr;
extern TEXT_FIELD txt_i2c_write_data;
extern TEXT_FIELD txt_i2c_read_data;
extern TEXT_FIELD txt_i2c_wlength;
extern TEXT_FIELD txt_i2c_rlength;
extern TEXT_FIELD txt_i2c_rdata;

extern EDIT_FIELD edf_i2c_id;
extern EDIT_FIELD edf_i2c_addr;
extern EDIT_FIELD edf_i2c_wdata;
extern EDIT_FIELD edf_i2c_wlen;

extern EDIT_FIELD edf_i2c_rlen;

extern TEXT_FIELD btn_i2c_writeread;
extern TEXT_FIELD btn_i2c_exit;
extern TEXT_FIELD text_eng_infoy;
extern TEXT_FIELD text_eng_focus;
extern TEXT_FIELD text_eng_help;

VACTION diag_btn_keymap(POBJECT_HEAD obj, UINT32 key);
VACTION diag_con_keymap(POBJECT_HEAD obj, UINT32 key);
VACTION diag_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
PRESULT diag_hex_edf_callback(POBJECT_HEAD p_obj, \
        VEVENT event, UINT32 param1, UINT32 param2);
PRESULT diag_exit_callback(POBJECT_HEAD obj, \
        VEVENT event, UINT32 param1, UINT32 param2);

UINT32 com_hex2uni_str(UINT16 *p_buffer, UINT32 num, UINT32 len);
UINT32 com_uni_str2hex(const UINT16* string);
void com_uni_str2hex_arr(UINT8 *p_data, const UINT16* string, UINT8 length);
UINT32 com_hex_arr2uni_str(UINT16 *p_string, const UINT8 *p_data, UINT16 len);

#ifdef __cplusplus
 }
#endif

#endif
