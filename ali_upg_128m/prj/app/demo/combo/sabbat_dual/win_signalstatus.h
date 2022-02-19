/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_signalstatus.h
*
*    Description: signal status menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _SIGNAL_STATUS_H_
#define _SIGNAL_STATUS_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libosd/obj_container.h>
#include <api/libosd/obj_bitmap.h>
#include <api/libosd/obj_textfield.h>
#include <api/libosd/osd_common.h>

extern CONTAINER g_win_sigstatus;
extern BITMAP sigstatus_bmp1;//,sigstatus_bmp2,sigstatus_bmp3,sigstatus_bmp4;
extern TEXT_FIELD sigstatus_txt;

extern BOOL is_no_signal_status; //
extern UINT8 lv_scramble_fake;   //

extern UINT32       g_signal_scram_flag;   //
extern UINT32       g_signal_unlock_flag;  //

typedef enum
{
    SIGNAL_STATUS_UNLOCK = 0,
    SIGNAL_STATUS_LOCK,
}signal_lock_status;

typedef enum
{
    SIGNAL_STATUS_SCRAMBLED = 0,
    SIGNAL_STATUS_UNSCRAMBLED,
}signal_scramble_status;


typedef enum
{
    SIGNAL_STATUS_LNBSHORT = 0,
    SIGNAL_STATUS_LNBNORMAL,
}signal_lnbshort_status;

typedef enum
{

    SIGNAL_STATUS_PARENT_UNLOCK = 0,
    SIGNAL_STATUS_PARENT_LOCK,
}signal_parentlock_status;

#ifdef PARENTAL_SUPPORT
typedef enum
{

    SIGNAL_STATUS_RATING_UNLOCK = 0,
    SIGNAL_STATUS_RATING_LOCK,
}signal_ratinglock_status;

typedef enum
{

    SIGNAL_STATUS_PVR_RATING_UNLOCK = 0,
    SIGNAL_STATUS_PVR_RATING_LOCK
}signal_pvr_ratinglock_status;
#endif

enum VDEC_PIC_STATUS
{
    VDEC_PIC_NO_SHOW,
    VDEC_PIC_NORMAL,
    VDEC_PIC_FREEZE,
};

enum VDEC_PIC_STATUS key_get_video_status(UINT32 *start_time);
BOOL key_get_dmx0_scramble(UINT32 *start_time);

BOOL get_signal_status(  signal_lock_status *lock_flag,
                        signal_scramble_status *scramble_flag,
                        signal_lnbshort_status *lnbshort_flag,
                        signal_parentlock_status *parrentlock_flag
#ifdef PARENTAL_SUPPORT
                        ,signal_ratinglock_status *ratinglock_flag
#endif
                        );

void set_channel_parrent_lock(signal_parentlock_status parrentlock_flag);

BOOL get_channel_parrent_lock(void);

#ifdef PARENTAL_SUPPORT
void set_rating_lock(signal_ratinglock_status ratinglock_flag);
BOOL get_rating_lock();
#endif

void set_lnbshort_detect(UINT32 flag);
void set_lnbshort_detect_start(UINT32 flag);

BOOL get_channel_scramble(void);
BOOL get_signal_stataus_show(void);
void show_signal_status_osdon_off(UINT8 flag);
void show_signal_status_on_off(void);
void show_signal_status_enable(void);
void show_signal_status_disable(void);
void set12vshort_detect(UINT32 flag);
void set_signal_scramble_fake(UINT8 b_fake);
UINT8 get_signal_scramble_fake(void);

void get_signal_status_rect(OSD_RECT *rect);

BOOL api_is_stream_scrambled(void);

void set_channel_nim_busy(UINT32 flag);
void shift_signal_osd(void);
void restore_signal_osd(void);

#ifdef PARENTAL_SUPPORT
void set_pvrrating_lock(signal_pvr_ratinglock_status ratinglock_flag);
#endif

BOOL get_pvrrating_lock(void);

#ifdef __cplusplus
}
#endif

#endif//_SIGNAL_STATUS_H_

