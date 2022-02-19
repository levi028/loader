/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup.h
*
*    Description: The common function of popup
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COM_POPUP_H_
#define _WIN_COM_POPUP_H_
//win_com_popup.h

#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>

#ifndef SD_UI
#if defined(SUPPORT_CAS7) ||defined(SUPPORT_CAS9)
#define MSG_POPUP_LAYER_W       500
#define MSG_POPUP_LAYER_H       200
#define MSG_POPUP_LAYER_L       290
#define MSG_POPUP_LAYER_T       250
#else
#define MSG_POPUP_LAYER_W       550
#define MSG_POPUP_LAYER_H       200
#define MSG_POPUP_LAYER_L       GET_MID_L(MSG_POPUP_LAYER_W)
#define MSG_POPUP_LAYER_T       GET_MID_T(MSG_POPUP_LAYER_H)
#endif
#else
#define MSG_POPUP_LAYER_W       400
#define MSG_POPUP_LAYER_H       100
#define MSG_POPUP_LAYER_L       ((OSD_MAX_WIDTH - MSG_POPUP_LAYER_W)/2)
#define MSG_POPUP_LAYER_T       250
#endif

typedef enum
{
    OSD_MSG_POPUP_LAYER = 0,
    OSD_MODAL_POPUP_LAYER
}win_popup_layer;

typedef enum
{
    WIN_POPUP_TYPE_SMSG = 0,
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    WIN_POPUP_TYPE_INPUTPIN,
#endif
    WIN_POPUP_TYPE_OK,
    WIN_POPUP_TYPE_OKNO,
    WIN_POPUP_TYPE_OKNOCANCLE,
}win_popup_type_t;

typedef enum
{
    WIN_POP_CHOICE_NULL = 0,
    WIN_POP_CHOICE_YES,
    WIN_POP_CHOICE_NO,
    WIN_POP_CHOICE_CANCEL,
}win_popup_choice_t;

void win_msg_popup_open();
void win_msg_popup_close();
void win_compopup_init(win_popup_type_t type);
void win_compopup_set_title(char* str,char* unistr,UINT16 str_id);
void win_compopup_set_msg(char* str,char* unistr,UINT16 str_id);
void win_compopup_set_msg_ext(char* str,char* unistr,UINT16 str_id);
void win_compopup_set_msg_utf(char *str,char *unistr,char *utf8, char *utf16, UINT16 str_id);
void win_compopup_set_btnstr(UINT32 btn_idx, UINT16 str_id);
void win_compopup_set_frame(UINT16 x,UINT16 y,UINT16 w,UINT16 h);
void win_compopup_set_pos(UINT16 x,UINT16 y);
void win_compopup_set_default_choice(win_popup_choice_t choice);
win_popup_choice_t win_compopup_open(void);
void win_compopup_close(void);
win_popup_choice_t win_com_popup_open(win_popup_type_t type,char* str,\
                                        UINT16 str_id);
win_popup_choice_t win_compopup_open_ext(UINT8* back_saved);
void win_compopup_smsg_restoreback(void);
void wincom_dish_move_popup_open(UINT16 pre_sat_id,UINT16 cur_sat_id, \
                                    UINT32 tuner_idx,UINT8* back_saved);

#if defined(SUPPORT_CAS7) ||defined(SUPPORT_CAS9)
void win_mmicompopup_set_msg(char* str,char* unistr,UINT16 str_id);
void win_mmipopup_open();
void win_mmipopup_close(UINT8 mmi_flag);
void win_pop_msg_open(UINT16 msg_id);
void win_pop_msg_close(UINT8 mmi_flag);
BOOL win_mmipopup_is_closed(void);
#endif
#ifdef SUPPORT_C0200A
void win_pop_msg_open_nvcak(UINT16 msg_id);
void win_pop_msg_close_nvcak(UINT8 mmi_flag);
#endif


#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
void win_compopup_set_enter_wrong_pin_mmi_status(BOOL value);
#endif

extern BOOL mmipopup_is_closed;        // mmi popup window close flag
win_popup_choice_t win_compopup_open_ext_cas(UINT8 *back_saved);
#ifdef SUPPORT_CAS9
void win_mmipopup_close_ex(UINT8 mmi_flag,UINT8 b_force);
#else
void win_mmipopup_close_ex(UINT8 mmi_flag);
#endif
void win_compopup_auto_standby_timer_countdown(void);

#ifdef AUTO_OTA
void win_otacompopup_set_msg(char* str,char* unistr,UINT16 str_id);
#endif

extern MULTI_TEXT   win_popup_msg_mbx_timer;    // popup message box timeout timer

#ifdef __cplusplus
}
#endif

#endif//_WIN_COM_SMSG_H_
