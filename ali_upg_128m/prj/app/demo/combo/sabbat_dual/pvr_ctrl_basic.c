/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_basic.c
*    Description: All the basic function, global parametor used in pvr_ctrl_* will be defined.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "pvr_ctrl_basic.h"
#include <api/libc/string.h>
#include "copper_common/com_api.h"
#include "ap_ctrl_ci.h"

#define VALID_ARRAY_IDX_8 0xff

BOOL            record_end = FALSE;
static UINT32 pvr_stop_play_attr = P_STOP_AND_REOPEN;
static UINT32 play_pvr_idx = 0;
static BOOL back2menu = FALSE;
static BOOL disk_check_while_init = TRUE;
static BOOL freeze_for_pvr = FALSE;                //black screen result in pvr state trans!
static BOOL end_revs = FALSE;
static BOOL first_time_ptm = FALSE;                //add for show 00:00:00 at first when FB
static BOOL pvr_mointor_enable = FALSE;
static BOOL respond_pvr_key = FALSE;                //respond pvr keys within MIN_TIME when tms or rec
static UINT32 bookmark_ptm[MAX_BOOKMARK_NUM] = {0};
static UINT8 bookmark_num = 0;
static UINT8 last_mark_pos = 0;
static UINT8 playback_type = 0;
static pvr_play_rec_t  g_pvr_info;
static struct list_info  g_play_pvr_info;
static struct list_info g_temp_play_pvr_info;
static BOOL g_need_check_tms=FALSE;

BOOL *api_get_need_check_tms(void)
{
     return &g_need_check_tms;
}

BOOL api_set_play_pvr_info(struct list_info *fill_info)
{
    if (NULL != fill_info)
    {
        MEMCPY(&g_play_pvr_info, fill_info, sizeof(struct list_info));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL api_set_temp_play_pvr_info(struct list_info *fill_info)
{
    if (NULL != fill_info)
    {
        MEMCPY(&g_temp_play_pvr_info, fill_info, sizeof(struct list_info));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

pvr_play_rec_t *api_get_pvr_info(void)
{
    return &g_pvr_info;
}

struct list_info *api_get_g_play_pvr_info(void)
{
    return &g_play_pvr_info;
}

void api_get_play_pvr_info(struct list_info *fill_info)
{
    if (NULL != fill_info)
    {
        MEMCPY(fill_info, &g_play_pvr_info, sizeof(struct list_info));
    }
}

void api_get_temp_play_pvr_info(struct list_info *fill_info)
{
    if (NULL != fill_info)
    {
        MEMCPY(fill_info, &g_temp_play_pvr_info, sizeof(struct list_info));
    }
}

void api_pvr_set_playback_type(UINT8 val)
{
    playback_type = val;
}
UINT8 api_pvr_get_playback_type(void)
{
    return playback_type;
}

void api_pvr_set_last_mark_pos(UINT8 val)
{
    last_mark_pos = val;
}
UINT8 api_pvr_get_last_mark_pos(void)
{
    return last_mark_pos;
}

PVR_HANDLE api_pvr_get_cur_rlidx(UINT16 *rl_idx)
{
    PVR_HANDLE     r_handle = 0;
    UINT8 rec_pos = 0;
    pvr_record_t *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, &rec_pos);

    switch (ap_pvr_set_state())
    {
        case PVR_STATE_IDEL:
            r_handle = 0;
            break;
        case PVR_STATE_TMS:
        case PVR_STATE_REC_TMS:
            r_handle = pvr_info->tms_r_handle;
            break;

        case PVR_STATE_REC:
        case PVR_STATE_REC_REC:
            if (rec)
            {
                r_handle = rec->record_handle;
            }
            break;
        case PVR_STATE_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY_HDD:
        case PVR_STATE_REC_PLAY:
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_REC_REC_PLAY:
        case PVR_STATE_REC_REC_PVR_PLAY:
        case PVR_STATE_UNREC_PVR_PLAY:
            r_handle = pvr_info->play.play_handle;
            break;
        default:
            r_handle = 0;
            break;
    }

    if (r_handle != 0)
    {
        *rl_idx = pvr_get_index(r_handle);
    }

    return r_handle;
}

void api_pvr_update_bookmark(void)
{
    UINT16 rl_idx = 0;

    if (api_pvr_get_cur_rlidx(&rl_idx) != 0)
    {
        if (rl_idx != TMS_INDEX)
        {
            bookmark_num = pvr_get_bookmark(rl_idx, bookmark_ptm);
            return;
        }
    }
    bookmark_num = 0;
    return;
}

void api_pvr_set_bookmark_num(UINT8 val)
{
    bookmark_num = val;
}
UINT8 api_pvr_get_bookmark_num(void)
{
    return bookmark_num;
}

UINT32 api_pvr_get_bookmark_ptm_by_index(UINT8 idx)
{
    if (idx < MAX_BOOKMARK_NUM)
    {
        return bookmark_ptm[idx];
    }
    else
    {
        return 0;
    }
}

void api_pvr_set_respond_pvr_key(BOOL val)
{
    respond_pvr_key = val;
}
BOOL api_pvr_get_respond_pvr_key(void)
{
    return respond_pvr_key;
}

void pvr_monitor_start(void)
{
    pvr_mointor_enable = TRUE;
}

void pvr_monitor_stop(void)
{
    pvr_mointor_enable = FALSE;
    api_pvr_set_freeze(FALSE);
}
BOOL api_pvr_get_monitor_enable(void)
{
    return pvr_mointor_enable;
}

void api_pvr_set_first_time_ptm(BOOL val)
{
    first_time_ptm = val;
}

BOOL api_pvr_get_first_time_ptm(void)
{
    return first_time_ptm;
}

void api_pvr_set_end_revs(BOOL val)
{
    end_revs = val;
}

BOOL api_pvr_get_end_revs(void)
{
    return end_revs;
}

void api_pvr_set_freeze(BOOL val)
{
    freeze_for_pvr = val;
}

BOOL api_pvr_get_freeze(void)
{
    return freeze_for_pvr;
}

void api_pvr_set_disk_checked_init(BOOL val)
{
    disk_check_while_init = val;
}

BOOL api_pvr_get_disk_checked_init(void)
{
    return disk_check_while_init;
}

void api_pvr_set_back2menu_flag(BOOL val)
{
    back2menu = val;
}

BOOL api_pvr_get_back2menu_flag(void)
{
    return back2menu;
}

UINT32 api_pvr_get_stop_play_attr(void)
{
    return pvr_stop_play_attr;
}

void api_pvr_set_stop_play_attr(UINT32 set_state)
{
    pvr_stop_play_attr = set_state;
}

UINT16 api_pvr_get_play_idx(void)
{
    return play_pvr_idx;
}

void api_pvr_set_play_idx(UINT16 set_idx)
{
    play_pvr_idx = set_idx;
}

BOOL pvr_time_shift_status(void)
{
    BOOL ret = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(( pvr_info->hdd_valid) && ( pvr_info->play.play_handle))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}

BOOL pvr_need_set_last_play_ptm(void)
{
    struct list_info play_pvr_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    if (RSM_FINGERP_RE_ENCRYPTION == play_pvr_info.rec_special_mode)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

pvr_state_t ap_pvr_set_state(void)
{
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    struct list_info  r_info;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&r_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    if( pvr_info->rec_num) //rec!
    {
        if( pvr_info->play.play_handle)
        {
            /*fix BUG41920:need to update flag:play_pvr_info.is_recording */
            MEMSET(&r_info, 0, sizeof(r_info));
            pvr_get_rl_info(play_pvr_info.index, &r_info);
            play_pvr_info.is_recording = r_info.is_recording;
            api_set_play_pvr_info(&play_pvr_info);
            if (play_pvr_info.is_recording)
            {
                if(( pvr_info->tms_r_handle) && (TMS_INDEX == play_pvr_idx))
                {
                    pvr_info->pvr_state = PVR_STATE_TMS_PLAY;
                }
                else
                {
                    pvr_info->pvr_state = PVR_STATE_REC_PLAY;
                }
            }
            else
            {
                pvr_info->pvr_state = PVR_STATE_REC_PVR_PLAY;
            }
        }
        else if( pvr_info->tms_r_handle)
        {
            pvr_info->pvr_state = PVR_STATE_REC_TMS;
        }
        else
        {
            pvr_info->pvr_state = PVR_STATE_REC;
        }
    }
    else if( pvr_info->tms_r_handle) //tms!
    {
        if( pvr_info->play.play_handle)
        {
            if(( pvr_info->tms_r_handle) && (TMS_INDEX == play_pvr_idx ))
            {
                pvr_info->pvr_state = PVR_STATE_TMS_PLAY;
            }
            else
            {
                pvr_info->pvr_state = PVR_STATE_UNREC_PVR_PLAY;
            }
        }
        else
        {
            pvr_info->pvr_state = PVR_STATE_TMS;
        }
    }
    else if( pvr_info->play.play_handle)
    {
        pvr_info->pvr_state = PVR_STATE_UNREC_PVR_PLAY;
    }
    else
    {
        pvr_info->pvr_state = PVR_STATE_IDEL;
    }


    return pvr_info->pvr_state;
}


BOOL api_pvr_is_record_active(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(( pvr_info->hdd_valid) && (( pvr_info->rec[REC_IDX0].record_chan_flag)
                                    || ( pvr_info->rec[REC_IDX1].record_chan_flag )))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL api_pvr_is_recording_ca_prog(void)
{
     pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(FALSE == pvr_info->hdd_valid)
    {
        return FALSE;
    }
    if (((pvr_info->rec[REC_IDX0].record_chan_flag) && (pvr_info->rec[REC_IDX0].ca_mode) &&
            (pvr_info->rec[REC_IDX0].is_scrambled)) || ((pvr_info->rec[REC_IDX1].record_chan_flag) &&
             (pvr_info->rec[REC_IDX1].ca_mode) && (pvr_info->rec[REC_IDX1].is_scrambled)))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL api_pvr_is_rec_only_or_play_rec(void)
{
     pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( (PVR_STATE_REC == pvr_info->pvr_state ) || (PVR_STATE_REC_PLAY == pvr_info->pvr_state)   ||
        (PVR_STATE_REC_REC == pvr_info->pvr_state) || (PVR_STATE_REC_REC_PLAY == pvr_info->pvr_state) )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL api_pvr_is_recording(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    if ((PVR_STATE_REC == pvr_info->pvr_state) || (PVR_STATE_TMS_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_TMS == pvr_info->pvr_state) || (PVR_STATE_REC_TMS_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_TMS_PLAY_HDD == pvr_info->pvr_state) || (PVR_STATE_REC_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_REC_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_REC_PVR_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_REC == pvr_info->pvr_state) || (PVR_STATE_REC_PVR_PLAY == pvr_info->pvr_state))
    {
        return TRUE;
    }
    return FALSE;
}
BOOL api_pvr_is_playing(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    if ((PVR_STATE_UNREC_PVR_PLAY == pvr_info->pvr_state) || (PVR_STATE_TMS_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_TMS_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_TMS_PLAY_HDD == pvr_info->pvr_state) ||
            (PVR_STATE_REC_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_PVR_PLAY == pvr_info->pvr_state) ||
            (PVR_STATE_REC_REC_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_REC_PVR_PLAY == pvr_info->pvr_state))
    {
        return TRUE;
    }
    return FALSE;
}

// Current playing hdd program or play recording progam(not tms)
BOOL api_pvr_is_playing_hdd_rec(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    if ((PVR_STATE_REC_TMS_PLAY_HDD == pvr_info->pvr_state) || (PVR_STATE_REC_PVR_PLAY == pvr_info->pvr_state)  ||
            (PVR_STATE_REC_REC_PVR_PLAY == pvr_info->pvr_state)||(PVR_STATE_UNREC_PVR_PLAY == pvr_info->pvr_state))
    {
        return TRUE;
    }
    return FALSE;
}

// Current playing is living broadcast, without time-shifting and playing hdd.
BOOL api_pvr_is_live_playing(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    if ((PVR_STATE_IDEL == pvr_info->pvr_state) || (PVR_STATE_TMS == pvr_info->pvr_state) ||
        (PVR_STATE_REC == pvr_info->pvr_state) || (PVR_STATE_REC_TMS == pvr_info->pvr_state) ||
        (PVR_STATE_REC_REC == pvr_info->pvr_state))
    {
        return TRUE;
    }
    return FALSE;
}
BOOL api_pvr_is_rec_or_playrec(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    ap_pvr_set_state();
    pvr_info  = api_get_pvr_info();
    if((PVR_STATE_REC== pvr_info->pvr_state) || (PVR_STATE_UNREC_PVR_PLAY== pvr_info->pvr_state))
    {
        return TRUE;
    }
    return FALSE;
}
BOOL api_pvr_is_recording_cur_prog(void)
{
    UINT8          rec_pos   = 0;
    pvr_play_rec_t *pvr_info = NULL;
    pvr_record_t   *rec      = NULL;

    pvr_info  = api_get_pvr_info();
    rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, &rec_pos);

    if( pvr_info->tms_r_handle)
    {
        return TRUE;
    }
    if (rec != NULL)
    {
        return TRUE;
    }
    return FALSE;
}
UINT8 api_pvr_get_rec_num_by_dmx(UINT8 dmx_id)
{
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( pvr_info->rec_num)
    {
        for (i = 0; i < sys_data_get_rec_num(); i++)
        {
            if((TRUE == pvr_info->rec[i].record_chan_flag ) &&
                (dmx_id == pvr_r_get_dmx_id( pvr_info->rec[i].record_handle)))
            {
                j++;
            }
        }
    }

    return j;
}

BOOL api_pvr_is_usb_unusable(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(USB_STATUS_OVER == pvr_info->hdd_status)
    {
        return FALSE;
    }
    return TRUE;
}

pvr_record_t *api_pvr_get_rec_by_dmx(UINT8 dmx_id, UINT8 pos)
{
    pvr_record_t *rec = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( pvr_info->rec_num)
    {
        for (i = 0; i < sys_data_get_rec_num(); i++)
        {
            if((TRUE == pvr_info->rec[i].record_chan_flag )
                 && (dmx_id == pvr_r_get_dmx_id( pvr_info->rec[i].record_handle)))
            {
                if (++j == pos)
                {
                    rec = &pvr_info->rec[i];
                    break;
                }
            }
        }
    }

    return rec;
}

pvr_record_t *api_pvr_get_rec(UINT8 pos)
{
    pvr_record_t *rec = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < sys_data_get_rec_num(); i++)
    {
        if(TRUE == pvr_info->rec[i].record_chan_flag)
        {
            if (++j == pos)
            {
                rec = &pvr_info->rec[i];
                break;
            }
        }
    }

    return rec;
}

pvr_record_t *api_pvr_get_rec_by_prog_id(UINT32 prog_id, UINT8 *pos)
{
    pvr_record_t *rec = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < sys_data_get_rec_num(); i++)
    {
        if(TRUE == pvr_info->rec[i].record_chan_flag)
        {
            j++;
            if( pvr_info->rec[i].record_chan_id == prog_id)
            {
                rec = &pvr_info->rec[i];
                if (pos != NULL)
                {
                    *pos = j;
                }
                break;
            }
        }
    }

    return rec;
}

pvr_record_t *api_pvr_get_rec_by_idx(UINT16 index, UINT8 *pos)
{
    pvr_record_t *rec = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < sys_data_get_rec_num(); i++)
    {
        if(TRUE == pvr_info->rec[i].record_chan_flag )
        {
            j++;
            if(pvr_get_index( pvr_info->rec[i].record_handle) == index)
            {
                rec = &pvr_info->rec[i];
                if (pos != NULL)
                {
                    *pos = j;
                }
                break;
            }
        }
    }

    return rec;
}

void api_pvr_set_rec(pvr_record_t *rec_info)
{
    UINT8 i = 0;
    UINT8 j = VALID_ARRAY_IDX_8;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < sys_data_get_rec_num(); i++)
    {
        if(TRUE == pvr_info->rec[i].record_chan_flag)
        {
            if( pvr_info->rec[i].record_chan_id == rec_info->record_chan_id)
            {
                j = i;
                break;
            }
        }
        else
        {
            if(VALID_ARRAY_IDX_8 == j)
            {
                j = i;
            }
        }
    }
    if(j != VALID_ARRAY_IDX_8)
    {
        MEMCPY(&pvr_info->rec[j], rec_info, sizeof(pvr_record_t));

    }
    return;
}



UINT32 pvr_rechnd2idx(PVR_HANDLE pvr_handle)
{
    UINT8 i = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < PVR_MAX_RECORDER; i++)
    {
        if( pvr_info->rec[i].record_handle == pvr_handle)
        {
            return i;
        }
    }

#ifdef PVR3_SUPPORT
    return PVR_HANDLE_2_INDEX(pvr_handle);
#else
    return (pvr_handle) ^ REC_MASK;
#endif
}

PVR_HANDLE pvr_recidx2hnd(UINT32 index)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if (index >= PVR_MAX_RECORDER)
    {
        return INVALID_HANDLE;
    }
    if( pvr_info->rec[index].record_chan_flag)
    {
        return pvr_info->rec[index].record_handle;
    }

#ifdef PVR3_SUPPORT
    return PVR_IDX_2_REC_HND(index);
#else
    return (REC_MASK + index);
#endif
}


UINT8 pvr_msg_hnd2idx(PVR_HANDLE pvr_handle)
{
    UINT8 index = PVR_HND_IDX_REC0;

#ifdef PVR3_SUPPORT
    if (PVR_HANDLE_IS_PLY(pvr_handle))
    {
        index = PVR_HND_IDX_PLY0;
    }
    else
    {
        index = pvr_rechnd2idx(pvr_handle);
    }
#else
    if (pvr_handle == PVR_HND_PLY0)
    {
        index = PVR_HND_IDX_PLY0;
    }
    else
    {
        index = pvr_rechnd2idx(pvr_handle);
    }
#endif

    return index;
}

PVR_HANDLE pvr_msg_idx2hnd(UINT8 index)
{
    UINT32 pvr_handle = PVR_HND_REC0;

#ifdef PVR3_SUPPORT
    if (PVR_HND_IDX_PLY0 == index)
    {
        pvr_handle = PVR_IDX_2_PLY_HND(0);
    }
    else
    {
        pvr_handle = pvr_recidx2hnd(index);
    }
#else
    if (PVR_HND_IDX_PLY0 == index)
    {
        pvr_handle = PVR_HND_PLY0;
    }
    else
    {
        pvr_handle = pvr_recidx2hnd(index);
    }
#endif

    return pvr_handle;
}

PVR_HANDLE pvr_rl_info_index2hnd(UINT32 index)
{
    UINT8 i = 0;
    pvr_record_t *rec_ptr = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < PVR_MAX_RECORDER; i++)
    {
        rec_ptr = &pvr_info->rec[i];
        if (rec_ptr->record_handle)
        {
            if (pvr_get_index(rec_ptr->record_handle) == index)
            {
                return rec_ptr->record_handle;
            }
        }
    }
    return 0;
}


pvr_record_t *api_pvr_get_rec_by_handle(PVR_HANDLE hdl, UINT8 *pos)
{
    pvr_record_t *rec = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    for (i = 0; i < sys_data_get_rec_num(); i++)
    {
        if(TRUE == pvr_info->rec[i].record_chan_flag)
        {
            j++;
            if( pvr_info->rec[i].record_handle == hdl)
            {
                rec = &pvr_info->rec[i];
                *pos = j;
                break;
            }
        }
    }

    return rec;
}

BOOL api_pvr_can_do_tms(UINT32 prog_id)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( PVR_STATE_IDEL == pvr_info->pvr_state
#if (!defined(CAS9_PVR_SUPPORT) && !defined(CI_PLUS_PVR_SUPPORT)  &&(!defined(CAS9_PVR_SCRAMBLED)) \
            &&(!defined(CAS7_PVR_SCRAMBLE)) && (!defined(BC_STD_PVR_CRYPTO)) && (!defined(C0200A_PVR_SUPPORT)) \
        && (!defined(BC_PVR_SUPPORT)) && (!defined(CAS7_PVR_SUPPORT)))
        || PVR_STATE_UNREC_PVR_PLAY == pvr_info->pvr_state
#endif
       )
    {
        return TRUE;
    }
    else if(PVR_STATE_REC ==  pvr_info->pvr_state)
    {
#if(defined(BC_PVR_SUPPORT))
        if (( pvr_info->rec[REC_IDX0].record_handle) || ( pvr_info->rec[REC_IDX1].record_handle))
        {
            //no time shift while recording
            return FALSE;
        }
#endif
        if (( pvr_info->rec[REC_IDX0].record_handle) &&( pvr_info->rec[REC_IDX1].record_handle))
        {
            return FALSE;
        }
        if( (( pvr_info->rec[REC_IDX0].record_handle) &&  (prog_id != pvr_info->rec[REC_IDX0].record_chan_id)) ||
            (( pvr_info->rec[REC_IDX1].record_handle) &&  (prog_id != pvr_info->rec[REC_IDX1].record_chan_id)) )
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL api_pvr_check_tsg_state(void)
{
    struct list_info rl_info;
    BOOL ret = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    if( pvr_info->play.play_handle)
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if (rl_info.is_scrambled)
        {
            ret = TRUE;
        }
    }

    return ret;
}
BOOL api_pvr_check_bitrate(UINT8 r_or_w, UINT32 bitrate) //0 write; 1 read
{
    BOOL ret = FALSE;
    struct dvr_hdd_info hdd_info;
    UINT16 i = 0;
    UINT32 total_bitrate = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&hdd_info, 0, sizeof(struct dvr_hdd_info));
    pvr_info  = api_get_pvr_info();
    pvr_get_hdd_info(&hdd_info);

    for (i = 0; i < PVR_MAX_RECORDER; i++)
    {
        if( pvr_info->rec[i].record_handle)
        {
            total_bitrate += pvr_info->rec[i].ts_bitrate;
        }
    }
    if( pvr_info->tms_r_handle)
    {
        total_bitrate += pvr_info->tms_ts_bitrate;
    }

    if (r_or_w)
    {
        if (bitrate < (BYTE2BIT(hdd_info.read_speed * PVR_UINT_K)))
        {
            ret = TRUE;
        }
    }
    else
    {
        //for record, NOTES: no record start during playback!!!
        if ((bitrate + total_bitrate) < (BYTE2BIT(hdd_info.write_speed * PVR_UINT_K)))
        {
            ret = TRUE;
        }
    }

    return ret;
}

BOOL api_pvr_check_dmx_canuseci(UINT8 ca_mode, UINT8 dmx_id)
{
    BOOL ret = TRUE;
    UINT8 dmx_rec_num = 0;
    UINT16 i = 0;
    pvr_record_t *rec = NULL;
    P_NODE p_cur_node;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&p_cur_node, 0, sizeof(P_NODE));
    pvr_info  = api_get_pvr_info();
    sys_data_get_curprog_info(&p_cur_node);

    // if scramble recording on dedicate dmx, any prog in this dmx play should not operate CI!
    if(ca_mode && ( pvr_info->rec_num > 0) && (0 ==  sys_data_get_scramble_record_mode()))
    {
        dmx_rec_num = api_pvr_get_rec_num_by_dmx(dmx_id);
        for (i = 0; i < dmx_rec_num; i++)
        {
            rec = api_pvr_get_rec_by_dmx(dmx_id, i + 1);
            if ((rec != NULL) && (TRUE == rec->ca_mode))
            {
                // treat any prog need to play on this dmx as FTA!
                ret = FALSE;
                break;
            }
        }
    }

    return ret;
}

BOOL api_pvr_check_can_useci(UINT32 ca_mode, UINT32 param)
{
    UINT8 i = 0;
    UINT8 j = 0;
    BOOL ret = TRUE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(FALSE == ca_mode)
    {
        return TRUE;
    }

    for (i = 0; i < DUAL_CI_SLOT; i++)
    {
        if (api_pvr_check_dmx_canuseci(ca_mode, i))
        {
            if( pvr_info->rec_num > 0)
            {
                for(j = 0; j < pvr_info->rec_num; j++)
                {
                    if( pvr_info->rec[j].ca_mode)
                    {
                        ret = FALSE;
                    }
                    else
                    {
                        ret =  TRUE;
                        break;
                    }
                }
            }
            else
            {
                ret =  TRUE;
                break;
            }
        }
        else
        {
            ret =  FALSE;
        }
        if(TRUE == ret)
        {
            break;
        }
    }
    return ret;
}

UINT32 api_pvr_gettotaltime_byhandle(PVR_HANDLE    play_handle)
{
    UINT32 pvr_total_time = 0;
    UINT16 play_rl_idx = 0;
    UINT8 rec_pos = 0;
    UINT8 i = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info rl_info;
    pvr_record_t *rec = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&rl_info, 0, sizeof(struct list_info));
    rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, &rec_pos);

    if (play_handle != 0)
    {
       play_rl_idx = pvr_get_index( pvr_info->play.play_handle);
    }
    if ((ap_pvr_set_state() != PVR_STATE_REC) && (pvr_info->pvr_state != PVR_STATE_REC_PLAY)
    && (pvr_info->pvr_state != PVR_STATE_REC_TMS))
    {
        if(((0 == play_rl_idx) || (TMS_INDEX == play_rl_idx )) && ( pvr_info->tms_r_handle)) //TMS
        {
            pvr_total_time = pvr_r_get_time( pvr_info->tms_r_handle);
        }
        else if (play_rl_idx > 0)
        {
            pvr_get_rl_info(play_rl_idx, &rl_info);
            pvr_total_time = rl_info.duration;
        }
    }
    else//rec's time
    {
        if(PVR_STATE_REC_TMS == pvr_info->pvr_state )
        {
            if( pvr_info->play.play_chan_id == pvr_info->tms_chan_id)
            {
                pvr_total_time = pvr_r_get_time( pvr_info->tms_r_handle);
            }
            else
            {
                if (NULL == rec)
                {
                    pvr_total_time = 0;
                }
                else
                {
                    pvr_total_time = pvr_r_get_time(rec->record_handle);
                }
            }
        }
        else
        {
            pvr_total_time = 0;
            if ((NULL == rec) && (play_rl_idx != 0))
            {
                for (i = 0; i < sys_data_get_rec_num(); i++)
                {
                    if (TRUE == pvr_info->rec[i].record_chan_flag
                        && pvr_get_index(pvr_info->rec[i].record_handle) == play_rl_idx)
                    {
                        rec = &pvr_info->rec[i];
                        break;
                    }
                }
            }
            if (rec != NULL)
            {
                pvr_total_time = pvr_r_get_time(rec->record_handle);
            }
        }
    }

    return pvr_total_time;
}

UINT32 api_pvr_get_jumpstep(void)
{
    UINT32 pvr_jumpstep = 0;

    switch (sys_data_get_jumpstep())
    {
        case 0:
            pvr_jumpstep = PVR_JUMP_STEP_L0;
            break;
        case 1:
            pvr_jumpstep = PVR_JUMP_STEP_L1;
            break;
        case 2:
            pvr_jumpstep = PVR_JUMP_STEP_L2;
            break;
        case 3:
            pvr_jumpstep = PVR_JUMP_STEP_L3;
            break;
        case 4:
            pvr_jumpstep = PVR_JUMP_STEP_L4;
            break;
        default:
            break;
    }
    return pvr_jumpstep;
}

void pvr_2recorder_skip_prog(void)
{
    return ;
}

UINT32 api_pvr_get_timesrch(UINT32 *rt, UINT32 *pt, UINT16 *rlidx)
{
    struct list_info  rl_info;
    UINT16         rl_idx = 0;
    PVR_HANDLE     r_handle = 0;
    UINT32         ptm = 0;
    UINT32         rtm = 0;
    UINT8 rec_pos = 0;
     pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info, 0, sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    pvr_record_t *rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);

    if (0 == api_pvr_get_cur_rlidx(&rl_idx))
    {
        return 0;
    }
    if((ap_pvr_set_state() != PVR_STATE_REC) && ( pvr_info->pvr_state != PVR_STATE_REC_PLAY))//show play_pvr's 2nd time
    {
        if ((0 == rl_idx) || (TMS_INDEX == rl_idx)) //TMS
        {
            r_handle = pvr_info->tms_r_handle;
            rtm = pvr_r_get_time(r_handle);
        }
        else
        {
            pvr_get_rl_info(rl_idx, &rl_info);
            rtm = rl_info.duration;
        }
    }
    else//show rec's 2nd time
    {
        if (NULL == rec)
        {
            rec = api_pvr_get_rec(1);
        }
        if (NULL != rec)
        {
            r_handle = rec->record_handle;
            rtm = pvr_r_get_time(r_handle);
        }
    }


    if( pvr_info->play.play_handle != 0)
    {
        ptm = pvr_p_get_time( pvr_info->play.play_handle);
    }
    else
    {
        ptm = rtm;
    }
    *rt = rtm;
    *pt = ptm;
    *rlidx = rl_idx;

    return 1;
}

BOOL api_pvr_p_lock_switch(UINT32 bEnable)
{

    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();
    if((FALSE == pvr_info->hdd_valid) || ((PVR_HANDLE)NULL == pvr_info->play.play_handle ))
    {
        return FALSE;
    }
    return pvr_p_lock_switch( pvr_info->play.play_handle, bEnable);

}

