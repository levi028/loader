/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_basic.h

*    Description: The control flow of drawing a pvr ctrl bar will be defined
                  in this file.There will be some callback for updating.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_CTRL_BAR_BASIC_H_
#define _WIN_PVR_CTRL_BAR_BASIC_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

#ifdef DVR_PVR_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#endif
#include <hld/snd/snd_dev.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#include "ctrl_util.h"
#include "pvr_ctrl_basic.h"
#include "ap_ctrl_ci.h"
#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <api/libsi/psi_pmt.h>
#include "win_audio_description.h"
#endif

#ifdef SUPPORT_CAS9
#include <api/libcas/conax_mmi.h> //play_apply_uri
#include "conax_ap/win_ca_uri.h"
#endif


#define DBG_PRINTF    PRINTF

/*******************************************************************************
*    ctrl bar outlook discription
*******************************************************************************/

typedef enum
{
    RECORD_BMP_STATIC,
    RECORD_BMP_PLAY_RECORD,
    RECORD_BMP_PLAY_MODE,
    RECORD_BMP_HDD_STATUS,
    RECORD_BMP_RECORD
}record_bmp_type;

typedef enum
{
    RECORD_TXT_PLAY_NAME,    /* Sat or record*/
    RECORD_TXT_DATE,
    RECORD_TXT_TIME,
    RECORD_TXT_PLAY_MODE,
    RECORD_TXT_PLAY_1ST_TIME,
    RECORD_TXT_PLAY_2ND_TIME,
    RECORD_TXT_HDD_STATUS,
    RECORD_TXT_RECORD_1ST_TIME,
    RECORD_TXT_RECORD_2ND_TIME,
    RECORD_TXT_RECORD_BITRATE,
    RECORD_TXT_HDD_SPEED
}record_txt_type;


typedef enum
{
    RECORD_BAR_PLAY,
    RECORD_BAR_RECORD
}record_bar_type;

typedef struct
{
    record_bmp_type bmp_type;
    UINT16          left,top,width,height;
    UINT16          icon;
}record_bmp_t;

typedef struct
{
    record_txt_type txt_type;
    UINT16          left,top,width,height;
    UINT8           shidx;
}record_txt_t;

typedef struct
{
    record_bar_type bar_type;
    UINT16          left,top,width,height;
    UINT8           shidx;
}record_bar_t;



/*******************************************************************************
*    Objects definition
*******************************************************************************/

extern CONTAINER     g_win_pvr_ctrl;


/*****************************Ctrl Bar*****************************************/

extern CONTAINER     g_win_pvr_ctrlbar;        // win_pvr_ctrlbar UI global
extern BITMAP         record_bmp;        // pvr record bmp
extern TEXT_FIELD     record_text;        // pvr record text
extern PROGRESS_BAR record_bar;            // pvr record progress bar



#ifndef SD_UI
#define W_CTRLBAR_L    169//186
#define W_CTRLBAR_T    390 //430
#define W_CTRLBAR_W    902
#define W_CTRLBAR_H    230
#else
#define W_CTRLBAR_L    3 //4   // 4
#define W_CTRLBAR_T    260//276 //430
#define W_CTRLBAR_W    600
#define W_CTRLBAR_H    170//122

#endif

#define WIN_CTRLBAR_SH_IDX         WSTL_N_PRSCRN1
#define BMP_SH_IDX                  WSTL_MIXBACK_WHITE_IDX

#define B_BG_SH_IDX                WSTL_BARBG_01_8BIT
#define B_MID_SH_IDX                WSTL_MIXBACK_BLACK_IDX//WSTL_NOSHOW_IDX




/*****************************Recored List*****************************************/
extern CONTAINER     g_win_pvr_recpoplist;



extern BITMAP        pvr_prl_bmp0;          // pvr item list bmp0
extern BITMAP        pvr_prl_bmp1;          // pvr item list bmp1
extern BITMAP        pvr_prl_bmp2;          // pvr item list bmp2
extern BITMAP        pvr_prl_bmp3;          // pvr item list bmp3
extern BITMAP        pvr_prl_bmp4;          // pvr item list bmp4
extern BITMAP        pvr_prl_bmp5;          // pvr item list bmp5
extern BITMAP        pvr_prl_bmp6;          // pvr item list bmp6
extern BITMAP        pvr_prl_bmp7;          // pvr item list bmp7
extern BITMAP        pvr_prl_bmp8;          // pvr item list bmp8
extern BITMAP        pvr_prl_bmp9;          // pvr item list bmp9

extern TEXT_FIELD    pvr_prl_idx0;          // pvr item list index0
extern TEXT_FIELD    pvr_prl_idx1;          // pvr item list index1
extern TEXT_FIELD    pvr_prl_idx2;          // pvr item list index2
extern TEXT_FIELD    pvr_prl_idx3;          // pvr item list index3
extern TEXT_FIELD    pvr_prl_idx4;          // pvr item list index4
extern TEXT_FIELD    pvr_prl_idx5;          // pvr item list index5
extern TEXT_FIELD    pvr_prl_idx6;          // pvr item list index6
extern TEXT_FIELD    pvr_prl_idx7;          // pvr item list index7
extern TEXT_FIELD    pvr_prl_idx8;          // pvr item list index8
extern TEXT_FIELD    pvr_prl_idx9;          // pvr item list index9

extern TEXT_FIELD    pvr_prl_date0;         // pvr item list date0
extern TEXT_FIELD    pvr_prl_date1;         // pvr item list date1
extern TEXT_FIELD    pvr_prl_date2;         // pvr item list date2
extern TEXT_FIELD    pvr_prl_date3;         // pvr item list date3
extern TEXT_FIELD    pvr_prl_date4;         // pvr item list date4
extern TEXT_FIELD    pvr_prl_date5;         // pvr item list date5
extern TEXT_FIELD    pvr_prl_date6;         // pvr item list date6
extern TEXT_FIELD    pvr_prl_date7;         // pvr item list date7
extern TEXT_FIELD    pvr_prl_date8;         // pvr item list date8
extern TEXT_FIELD    pvr_prl_date9;         // pvr item list date9

extern TEXT_FIELD    pvr_prl_time0;         // pvr item list time0
extern TEXT_FIELD    pvr_prl_time1;         // pvr item list time1
extern TEXT_FIELD    pvr_prl_time2;         // pvr item list time2
extern TEXT_FIELD    pvr_prl_time3;         // pvr item list time3
extern TEXT_FIELD    pvr_prl_time4;         // pvr item list time4
extern TEXT_FIELD    pvr_prl_time5;         // pvr item list time5
extern TEXT_FIELD    pvr_prl_time6;         // pvr item list time6
extern TEXT_FIELD    pvr_prl_time7;         // pvr item list time7
extern TEXT_FIELD    pvr_prl_time8;         // pvr item list time8
extern TEXT_FIELD    pvr_prl_time9;         // pvr item list time9

extern TEXT_FIELD    pvr_prl_name0;         // pvr item list name0
extern TEXT_FIELD    pvr_prl_name1;         // pvr item list name1
extern TEXT_FIELD    pvr_prl_name2;         // pvr item list name2
extern TEXT_FIELD    pvr_prl_name3;         // pvr item list name3
extern TEXT_FIELD    pvr_prl_name4;         // pvr item list name4
extern TEXT_FIELD    pvr_prl_name5;         // pvr item list name5
extern TEXT_FIELD    pvr_prl_name6;         // pvr item list name6
extern TEXT_FIELD    pvr_prl_name7;         // pvr item list name7
extern TEXT_FIELD    pvr_prl_name8;         // pvr item list name8
extern TEXT_FIELD    pvr_prl_name9;         // pvr item list name9

extern BITMAP        pvr_prl_lock_bmp0;     // pvr item list lock bmp0
extern BITMAP        pvr_prl_lock_bmp1;     // pvr item list lock bmp1
extern BITMAP        pvr_prl_lock_bmp2;     // pvr item list lock bmp2
extern BITMAP        pvr_prl_lock_bmp3;     // pvr item list lock bmp3
extern BITMAP        pvr_prl_lock_bmp4;     // pvr item list lock bmp4
extern BITMAP        pvr_prl_lock_bmp5;     // pvr item list lock bmp5
extern BITMAP        pvr_prl_lock_bmp6;     // pvr item list lock bmp6
extern BITMAP        pvr_prl_lock_bmp7;     // pvr item list lock bmp7
extern BITMAP        pvr_prl_lock_bmp8;     // pvr item list lock bmp8
extern BITMAP        pvr_prl_lock_bmp9;     // pvr item list lock bmp9

extern BITMAP        pvr_prl_fta_bmp0;      // pvr item list fta bmp0
extern BITMAP        pvr_prl_fta_bmp1;      // pvr item list fta bmp1
extern BITMAP        pvr_prl_fta_bmp2;      // pvr item list fta bmp2
extern BITMAP        pvr_prl_fta_bmp3;      // pvr item list fta bmp3
extern BITMAP        pvr_prl_fta_bmp4;      // pvr item list fta bmp4
extern BITMAP        pvr_prl_fta_bmp5;      // pvr item list fta bmp5
extern BITMAP        pvr_prl_fta_bmp6;      // pvr item list fta bmp6
extern BITMAP        pvr_prl_fta_bmp7;      // pvr item list fta bmp7
extern BITMAP        pvr_prl_fta_bmp8;      // pvr item list fta bmp8
extern BITMAP        pvr_prl_fta_bmp9;      // pvr item list fta bmp9

extern TEXT_FIELD   pvr_prl_line0;          // pvr item list line0
extern TEXT_FIELD   pvr_prl_line1;          // pvr item list line1
extern TEXT_FIELD   pvr_prl_line2;          // pvr item list line2
extern TEXT_FIELD   pvr_prl_line3;          // pvr item list line3
extern TEXT_FIELD   pvr_prl_line4;          // pvr item list line4
extern TEXT_FIELD   pvr_prl_line5;          // pvr item list line5
extern TEXT_FIELD   pvr_prl_line6;          // pvr item list line6
extern TEXT_FIELD   pvr_prl_line7;          // pvr item list line7
extern TEXT_FIELD   pvr_prl_line8;          // pvr item list line8
extern TEXT_FIELD   pvr_prl_line9;          // pvr item list line9


extern TEXT_FIELD  pvr_prl_title;           // pvr item list title
extern OBJLIST         pvr_prl_ol;          // pvr item object list
extern SCROLL_BAR     pvr_prl_scb;          // pvr list scroll bar



#define WIN_LIST_SH_IDX      WSTL_POP_WIN_01_8BIT

#define TITLE_SH_IDX    WSTL_TEXT_04_8BIT

#define PVR_LST_SH_IDX        WSTL_NOSHOW_IDX
#define PVR_LST_HL_IDX        WSTL_NOSHOW_IDX
#define PVR_LST_SL_IDX        WSTL_NOSHOW_IDX
#define PVR_LST_GRY_IDX        WSTL_NOSHOW_IDX

#define PVR_CON_SH_IDX   WSTL_TEXT_30_8BIT
#define PVR_CON_HL_IDX   WSTL_BUTTON_05_8BIT
#define PVR_CON_SL_IDX   WSTL_TEXT_30_8BIT
#define PVR_CON_GRY_IDX  WSTL_TEXT_14

#define BMPF_SH_IDX     WSTL_MIX_BMP_HD//sharon WSTL_TEXT_29_8BIT
#define BMPF_HL_IDX       BMPF_SH_IDX
#define BMPF_SL_IDX       BMPF_SH_IDX
#define BMPF_GRY_IDX      BMPF_SH_IDX

#define PVR_TXTI_SH_IDX   WSTL_TEXT_30_8BIT
#define PVR_TXTI_HL_IDX   WSTL_TEXT_28_8BIT
#define PVR_TXTI_SL_IDX   WSTL_TEXT_30_8BIT
#define PVR_TXTI_GRY_IDX  WSTL_TEXT_14

#define TXTD_SH_IDX   WSTL_TEXT_30_8BIT
#define TXTD_HL_IDX   WSTL_TEXT_28_8BIT
#define TXTD_SL_IDX   WSTL_TEXT_30_8BIT
#define TXTD_GRY_IDX  WSTL_TEXT_14

#define TXTT_SH_IDX   WSTL_TEXT_30_8BIT
#define TXTT_HL_IDX   WSTL_TEXT_28_8BIT
#define TXTT_SL_IDX   WSTL_TEXT_30_8BIT
#define TXTT_GRY_IDX  WSTL_TEXT_14

#define PVR_TXTN_SH_IDX   WSTL_TEXT_30_8BIT
#define PVR_TXTN_HL_IDX   WSTL_TEXT_28_8BIT
#define PVR_TXTN_SL_IDX   WSTL_TEXT_30_8BIT
#define PVR_TXTN_GRY_IDX  WSTL_TEXT_14

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#ifndef SD_UI
#define    W_LIST_L     50//186
#define    W_LIST_T     20//70
#define    W_LIST_W     902
#define    W_LIST_H     358

#define TITLE_L        (W_LIST_L + 60)
#define TITLE_T        (W_LIST_T + 30)
#define TITLE_W        (W_LIST_W - 120)
#define TITLE_H        40

#define PVR_ITEM_L    (W_LIST_L + 34)
#define PVR_ITEM_T    (TITLE_T + TITLE_H)
#define PVR_ITEM_W    (W_LIST_W - 18 - 6 - 60)
#define PVR_ITEM_H    40
#define PVR_ITEM_GAP    2

#define PAGE_CNT    6

#define PVR_SCB_L (PVR_ITEM_L+PVR_ITEM_W)
#define PVR_SCB_T PVR_ITEM_T
#define PVR_SCB_W 12//18
#define PVR_SCB_H ((PVR_ITEM_H + PVR_ITEM_GAP)*PAGE_CNT)

#define LST_L    (W_LIST_L + 4)
#define LST_T    (PVR_SCB_T)
#define LST_W    (W_LIST_W - PVR_SCB_W - 6)
#define LST_H    PVR_SCB_H

#define ITEM_BMP_L      2
#define ITEM_BMP_W      40
#define PVR_ITEM_IDX_L      (ITEM_BMP_L + ITEM_BMP_W + 4)
#define PVR_ITEM_IDX_W      60//40
#define ITEM_DATE_L      (PVR_ITEM_IDX_L + PVR_ITEM_IDX_W + 4)
#define ITEM_DATE_W      170//110
#define ITEM_TIME_L      (ITEM_DATE_L + ITEM_DATE_W + 4)
#define ITEM_TIME_W      180//120
#define ITEM_NAME_L      (ITEM_TIME_L + ITEM_TIME_W + 4)
#define ITEM_NAME_W      230
#define ITEM_LOCK_MBP_L (ITEM_NAME_L + ITEM_NAME_W + 8)
#define ITEM_LOCK_MBP_W 30
#define ITEM_LOCK_FTA_L (ITEM_LOCK_MBP_L + ITEM_LOCK_MBP_W + 6)
#define ITEM_LOCK_FTA_W 42

#define PVR_LINE_L_OF       0
#define PVR_LINE_W          PVR_ITEM_W
#define PVR_LINE_H          4
#else/////////////////////////////////////////////////////////
#define    W_LIST_L     30//186
#define    W_LIST_T     20//70
#define    W_LIST_W     560
#define    W_LIST_H     250

#define TITLE_L        (W_LIST_L + 10)
#define TITLE_T        (W_LIST_T + 6)
#define TITLE_W        (W_LIST_W - 20)
#define TITLE_H        30

#define PVR_ITEM_L    (W_LIST_L + 20)
#define PVR_ITEM_T    (TITLE_T + TITLE_H)
#define PVR_ITEM_W    (W_LIST_W - 18 - 6 - 30)
#define PVR_ITEM_H    30
#define PVR_ITEM_GAP    2

#define PAGE_CNT    6

#define PVR_SCB_L (PVR_ITEM_L+PVR_ITEM_W)
#define PVR_SCB_T PVR_ITEM_T
#define PVR_SCB_W 12//18
#define PVR_SCB_H ((PVR_ITEM_H + PVR_ITEM_GAP)*PAGE_CNT)

#define LST_L    (W_LIST_L + 4)
#define LST_T    (PVR_SCB_T)
#define LST_W    (W_LIST_W - PVR_SCB_W - 6)
#define LST_H    PVR_SCB_H

#define ITEM_BMP_L      2
#define ITEM_BMP_W      40
#define PVR_ITEM_IDX_L      (ITEM_BMP_L + ITEM_BMP_W + 4)
#define PVR_ITEM_IDX_W      40//40
#define ITEM_DATE_L      (PVR_ITEM_IDX_L + PVR_ITEM_IDX_W + 4)
#define ITEM_DATE_W      110//110
#define ITEM_TIME_L      (ITEM_DATE_L + ITEM_DATE_W + 4)
#define ITEM_TIME_W      120//120
#define ITEM_NAME_L      (ITEM_TIME_L + ITEM_TIME_W + 4)
#define ITEM_NAME_W      200
#define ITEM_LOCK_MBP_L (ITEM_NAME_L + ITEM_NAME_W + 8)
#define ITEM_LOCK_MBP_W 30
#define ITEM_LOCK_FTA_L (ITEM_LOCK_MBP_L + ITEM_LOCK_MBP_W + 6)
#define ITEM_LOCK_FTA_W 30

#define PVR_LINE_L_OF       0
#define PVR_LINE_W          PVR_ITEM_W
#define PVR_LINE_H          4

#endif




/*****************************Record end time setting *****************************************/
extern CONTAINER     g_win_pvr_rec_etm_set;     // pvr record etm container
extern TEXT_FIELD    pvr_rec_etm_txt;           // pvr record etm text field
extern EDIT_FIELD    pvr_rec_etm_edt;           // pvr record etm edit field

#define WIN_REC_ETM_SH_IDX     WSTL_N_PRSCRN1// WSTL_POP_WIN_01_8BIT//WSTL_WIN_PVR_03//WSTL_INFO_02

#define CON_ETM_SH_IDX   WSTL_N_PRSCRN1//WSTL_TEXT_30_8BIT
#define TXT_ETM_SH_IDX   WSTL_N_PRSCRN1//WSTL_TEXT_30_8BIT
#define EDF_ETM_SH_IDX   WSTL_N_PRSCRN1//WSTL_TEXT_30_8BIT

#ifndef SD_UI
#if defined(SUPPORT_CAS9)
#define W_ETM_W        448
#define W_ETM_H        137
#define W_ETM_L        290
#define W_ETM_T        240
#elif defined(SUPPORT_CAS_A)
#define W_ETM_W    400
#define W_ETM_H        100
#define W_ETM_L        ((OSD_MAX_WIDTH - W_ETM_W)/2)
#define W_ETM_T        280//260
#else
#define W_ETM_W        400
#define W_ETM_H        140
#define W_ETM_L        ((OSD_MAX_WIDTH - W_ETM_W)/2)
#define W_ETM_T        220//260
#endif


#define TXT_ETM_L    (W_ETM_L + 30)
#ifdef SUPPORT_CAS_A
#define TXT_ETM_T    (W_ETM_T + 30)
#else
#define TXT_ETM_T    (W_ETM_T + 50)
#endif
#define TXT_ETM_W    180
#define TXT_ETM_H    40

#define EDF_ETM_L    (TXT_ETM_L + TXT_ETM_W)
#define EDF_ETM_T    TXT_ETM_T
#define EDF_ETM_W    150
#define EDF_ETM_H    40
#else

#define W_ETM_W        380
#define W_ETM_H        50
#define W_ETM_L        100
#define W_ETM_T        200

#define TXT_ETM_L    (W_ETM_L + 10)
#define TXT_ETM_T    (W_ETM_T + 10)
#define TXT_ETM_W    180
#define TXT_ETM_H    28

#define EDF_ETM_L    (TXT_ETM_L + TXT_ETM_W)
#define EDF_ETM_T    TXT_ETM_T
#define EDF_ETM_W    150
#define EDF_ETM_H    28

#endif




/****************************CTRL win *****************************************/
// CTRL win

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/




#define REC_ETM_TIME_OUT 50
#define V_KEY_BOOKMARK_EXT    V_KEY_RED //V_KEY_P_UP        // if no bookmark vkey.
#define V_KEY_JUMPMARK_EXT    V_KEY_GREEN    //V_KEY_P_DOWN    // if no bookmark vkey.

#define MIN_TIME_TMS_BACKPAUSE 5// seconds
#define HDD_PROTECT_TIME 3//seconds to skip to live play
#define MINI_BOOKMARK_TIME    30 //seconds to enable set bookmark.
#define STEP_TOTAL_NUM 10//for left/right step time_len!
#define VACT_EXIT (VACT_PASS + 1)
#define VACT_VALID (VACT_PASS + 1)
#define POS_ICON_WIDTH 16
#define POS_ICON_EDGE  4
#define POS_BOOKMARK_WIDTH 3
#define VACT_ETM_DECREASE (VACT_PASS + 1)
#define VACT_ETM_INCREASE (VACT_PASS + 2)

#define POS_HIGH_LIMIT  100
#define HD_FULLLESS_NUM 5

#endif
void api_pvr_set_from_pvrctrlbar(BOOL val);
BOOL api_pvr_get_from_pvrctrlbar();
void api_pvr_set_updata_infor(BOOL val);
BOOL api_pvr_get_updata_infor(void);

void api_pvr_set_poplist_display(UINT8 val);
UINT8 api_pvr_get_poplist_display();
void api_pvr_set_etm_display(UINT8 val);
UINT8 api_pvr_get_etm_display();
void api_pvr_set_etm_nokey_time(UINT8 val);
UINT8 api_pvr_get_etm_nokey_time();
void api_pvr_set_tmsrch_display(UINT8 val);
UINT8 api_pvr_get_tmsrch_display();
void api_pvr_set_tmsrch_pos(UINT8 val);
UINT8 api_pvr_get_tmsrch_pos();

void api_pvr_set_bar_start_ptm(UINT32 val);
UINT32 api_pvr_get_bar_start_ptm();
char * api_pvr_set_ttx_string(char *set_var);
INT api_pvr_compare_ttx_string(char *to_compare);
void convert_rec_time(date_time *des,date_time *src,INT32 hoff, INT32 moff);
BOOL pvr_check_jump(UINT32 vkey, UINT32 *ptr_ptm, UINT32 *prt_rtm, UINT16 *ptr_idx);
void get_rec_time(PVR_HANDLE handle, INT32 *timepassed,  INT32 *timetotal);

#ifdef __cplusplus
}
#endif

#endif//_WIN_PVR_CTRL_BAR_BASIC_H_
