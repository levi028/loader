/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_draw.c

*    Description: The drawing flows of the BMP\TEXT\ICON in pvr ctrl bar
                  will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <api/libpvr/lib_pvr.h>
#include "win_pvr_ctrl_bar_draw.h"
#include "pvr_ctrl_basic.h"
#include "ap_ctrl_ci.h"

#ifndef SD_UI
#define VOFFSET 30 //70//0
#define HOFFSET 174 //210//0

static record_bmp_t record_bmps[] =
{
    {RECORD_BMP_PLAY_RECORD,    46 + HOFFSET,   360 + VOFFSET, 48, 54, 0},

    {RECORD_BMP_STATIC,         580 + HOFFSET,  372 + VOFFSET,  32, 32, 0},
    {RECORD_BMP_STATIC,         694 + HOFFSET,  372 + VOFFSET,  32, 32, 0},

    {RECORD_BMP_PLAY_MODE,      12 + HOFFSET,   510 + VOFFSET, 39, 35, 0},
    {RECORD_BMP_HDD_STATUS,     752 + HOFFSET,  470 + VOFFSET, 26, 46, 0},
    {RECORD_BMP_RECORD,         46 + HOFFSET,   370 + VOFFSET,  90, 35, IM_N_INFORMATION_PVR},
};

#define RECORD_BMP_NUM    (sizeof(record_bmps)/sizeof(record_bmp_t))

#define RECORD_TXT_BLACK_IDX    WSTL_N_TXT1
#define RECORD_TXT_WHITE_IDX    WSTL_N_TXT1

#define LONG_BAR

#ifdef LONG_BAR
#define BAR_L (128 + HOFFSET)
#define BAR_T (478 + VOFFSET)
#define BAR_W 580
#define PLAY_TIME_1_L BAR_L
#define PLAY_TIME_T (408 + VOFFSET)
#else
#define BAR_L (460 + HOFFSET)
#define BAR_T (500 + VOFFSET)
#define BAR_W (372)
#define PLAY_TIME_1_L (340 + HOFFSET)
#define PLAY_TIME_T (496 + VOFFSET)
#endif


record_txt_t record_txts[] =
{
    {RECORD_TXT_PLAY_NAME,      150 + HOFFSET,  366 + VOFFSET,  385,    40, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_DATE,           620 + HOFFSET,  366 + VOFFSET,  74,     40, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_TIME,           734 + HOFFSET,  366 + VOFFSET,  86,     40, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_PLAY_MODE,      42 + HOFFSET,   430 + VOFFSET,  86,     40, RECORD_TXT_BLACK_IDX},

    {RECORD_TXT_PLAY_1ST_TIME,  128 + HOFFSET,  430 + VOFFSET,  148,    40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_PLAY_2ND_TIME,  600 + HOFFSET,  430 + VOFFSET,  148,    40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_HDD_STATUS,     782 + HOFFSET,  470 + VOFFSET,  72, 40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_1ST_TIME,    68 + HOFFSET,   510 + VOFFSET,  128,    40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_2ND_TIME,    430 + HOFFSET,  510 + VOFFSET,  114,    40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_BITRATE, 550 + HOFFSET,  510 + VOFFSET,  86, 40, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_HDD_SPEED,      636 + HOFFSET,  510 + VOFFSET,  230,    40, RECORD_TXT_BLACK_IDX}, //242
};

#define RECORD_TXT_NUM    (sizeof(record_txts)/sizeof(record_txt_t))

static record_bar_t record_bars[] =
{
    {RECORD_BAR_PLAY,           128 + HOFFSET, 470 + VOFFSET, 580, 24, WSTL_PLAYLIST_TICK_FG},
    {RECORD_BAR_RECORD,         190 + HOFFSET, 520 + VOFFSET, 230, 24, WSTL_PLAYLIST_TICK_FG},
};
#else
#define VOFFSET 0 //70//0
#define HOFFSET 0 //210//0

static record_bmp_t record_bmps[] =
{
    {RECORD_BMP_PLAY_RECORD,    45 + HOFFSET,   260 + VOFFSET, 38, 40, IM_INFORMATION_PVR},

    {RECORD_BMP_STATIC,         406 + HOFFSET,  270 + VOFFSET,  24, 24, IM_INFORMATION_DAY},
    {RECORD_BMP_STATIC,         486 + HOFFSET,  270 + VOFFSET,  24, 24, IM_INFORMATION_TIME},

    {RECORD_BMP_PLAY_MODE,      20 + HOFFSET,   362 + VOFFSET, 24, 24, 0},
    {RECORD_BMP_HDD_STATUS,     523 + HOFFSET,  314 + VOFFSET, 20, 40, 0},
    {RECORD_BMP_RECORD,         45 + HOFFSET,   264 + VOFFSET,  60, 40, IM_INFORMATION_REC},
};

#define RECORD_BMP_NUM    (sizeof(record_bmps)/sizeof(record_bmp_t))

#define RECORD_TXT_BLACK_IDX    WSTL_MIXBACK_IDX_08_8BIT
#define RECORD_TXT_WHITE_IDX    WSTL_MIXBACK_IDX_04_8BIT

#define LONG_BAR

#define BAR_L (90 + HOFFSET)
#define BAR_T (340 + VOFFSET)
#define BAR_W 425
#define PLAY_TIME_1_L BAR_L
#define PLAY_TIME_T (315 + VOFFSET)

static record_txt_t record_txts[] =
{
    {RECORD_TXT_PLAY_NAME,      110 + HOFFSET,  270 + VOFFSET,  270,    24, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_DATE,           430 + HOFFSET,  270 + VOFFSET,  52,     24, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_TIME,           510 + HOFFSET,  270 + VOFFSET,  52,     24, RECORD_TXT_WHITE_IDX},
    {RECORD_TXT_PLAY_MODE,      42 + HOFFSET,   322 + VOFFSET,  60,     24, RECORD_TXT_BLACK_IDX},

    {RECORD_TXT_PLAY_1ST_TIME,  104 + HOFFSET,  322 + VOFFSET,  90,    24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_PLAY_2ND_TIME,  436 + HOFFSET,  322 + VOFFSET,  90,    24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_HDD_STATUS,     544 + HOFFSET,  330+ VOFFSET,  45, 24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_1ST_TIME,    52 + HOFFSET,   362 + VOFFSET,  80,    24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_2ND_TIME,    284 + HOFFSET,  362 + VOFFSET,  80,    24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_RECORD_BITRATE, 372 + HOFFSET,  362 + VOFFSET,  60, 24, RECORD_TXT_BLACK_IDX},
    {RECORD_TXT_HDD_SPEED,      434 + HOFFSET,  362 + VOFFSET,  130,    24, RECORD_TXT_BLACK_IDX}, //242
};

#define RECORD_TXT_NUM    (sizeof(record_txts)/sizeof(record_txt_t))

static record_bar_t record_bars[] =
{
    {RECORD_BAR_PLAY,           189 + HOFFSET, 326+ VOFFSET, 240, 16, WSTL_PLAYLIST_TICK_FG},
    {RECORD_BAR_RECORD,         142 + HOFFSET, 366 + VOFFSET, 130, 16, WSTL_PLAYLIST_TICK_FG},
};

#endif

static UINT32 rtm_last=0;//add for CAM out when in TMS_PLAY or REC_PLAY state!
static UINT16 hd_status_icons[] =
{
    IM_HDD_INFO_01,
    IM_HDD_INFO_02,
    IM_HDD_INFO_03,
    IM_HDD_INFO_04,
    IM_HDD_INFO_05,
};
static UINT16 play_status_icons[] =
{
    IM_PVR_ICON_STOP,//NV_STOP      = 0,         //stop
    IM_PVR_ICON_PLAY,//NV_PLAY      = 1,         //normal playback
    IM_PVR_ICON_PAUSE,//NV_PAUSE     = 2,         //pause
    IM_PVR_ICON_FORWARD,//NV_FF         = 3,        //fast forward
    IM_PVR_ICON_STEP,//NV_STEP      = 4,        //step
    IM_PVR_ICON_BACKWARD,//NV_FB         = 5,        //fast backward
    IM_PVR_ICON_SLOWFORWARD,//NV_SLOW         = 6,        //slow
    IM_PVR_ICON_SLOWBACKWARD,//NV_REVSLOW    = 7,        //reverse slow
};
static void win_pvr_ctlbar_draw_bmp(struct dvr_hdd_info *hdd_info,BOOL cur_prog_live_play_only,pvr_record_t *rec,
    UINT16 play_rl_idx)
{
    UINT8 i=0;
    UINT16 icon = 0;
    UINT32 step_sectors = 0;
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    UINT32 hd_fulless = 0;
    PVR_STATE play_state = 0;
    BITMAP *bmp = &record_bmp;
    record_bmp_t *bmp_desc = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    cur_pvr_mode = pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    pvr_info  = api_get_pvr_info();
    for(i=0;i<RECORD_BMP_NUM;i++)
    {
        bmp_desc = &record_bmps[i];
        icon = INVALID_ID;

        if ((cur_prog_live_play_only) &&
            ((RECORD_BMP_PLAY_RECORD == bmp_desc->bmp_type) ||
             (RECORD_BMP_PLAY_MODE == bmp_desc->bmp_type)   ||
             (RECORD_BMP_RECORD == bmp_desc->bmp_type )))
        {
            continue;
        }

        switch(bmp_desc->bmp_type)
        {
        case RECORD_BMP_STATIC:
            icon = bmp_desc->icon;
            break;
        case RECORD_BMP_PLAY_RECORD:
            if( pvr_info->play.play_handle != 0)
            {
                if(play_rl_idx != TMS_INDEX && (rec != NULL))
                {
                    icon = bmp_desc->icon;
                }
            }
            break;
        case RECORD_BMP_PLAY_MODE:
            play_state = 0xFF;

            if( pvr_info->play.play_handle != 0)
            {
                play_state    = pvr_p_get_state( pvr_info->play.play_handle);
            }
            if(play_state< sizeof(play_status_icons)/2)
            {
                icon = play_status_icons[play_state];
            }
            break;
        case RECORD_BMP_HDD_STATUS:
            if(PVR_DISK_REC_AND_TMS == cur_pvr_mode)
            {
                step_sectors = (hdd_info->total_size - hdd_info->tms_size) / 5;
            }
            else
            {
                if((PVR_DISK_REC_WITH_TMS ==  cur_pvr_mode  ) || (PVR_DISK_ONLY_REC == cur_pvr_mode  ))
                {
                    step_sectors = hdd_info->total_size / 5;
                }
                else
                {
                    if(PVR_DISK_ONLY_TMS == cur_pvr_mode)
                    {
                        icon = hd_status_icons[4];
                        break;
                    }
                }
            }
            if(step_sectors >0 )
            {
                hd_fulless = (hdd_info->rec_size + step_sectors/2) / step_sectors ;
                if(HD_FULLLESS_NUM == hd_fulless)
                {
                    hd_fulless = 4;
                }
                icon = hd_status_icons[4-hd_fulless];//hd_fulless -> 4-hd_fulles, show used space!
            }
            break;
        case RECORD_BMP_RECORD:
            if((rec != NULL) && (0 == pvr_info->play.play_handle))
            {
                icon = bmp_desc->icon;
            }
            break;
        default:
            break;
        }

        /* If not in recording status, or not in rec_pvr_play state, don't show recording information. */
        if(((!api_pvr_is_rec_only_or_play_rec()) || (pvr_play_goon)) &&
            (RECORD_BMP_RECORD == bmp_desc->bmp_type))
       {
                continue;
       }
        osd_set_bitmap_content(bmp, icon);
        osd_set_rect(&bmp->head.frame, bmp_desc->left, bmp_desc->top, bmp_desc->width,bmp_desc->height);

#ifndef SD_UI
        if(( RECORD_BMP_PLAY_MODE == bmp_desc->bmp_type) &&(!osd_get_vscr_state()))
        {
         //#ifndef SD_UI
         //3281 will flicker because of hide
            osd_hide_object((OBJECT_HEAD*) bmp, C_UPDATE_ALL);
          //#endif
        }
#endif
        osd_draw_object( (OBJECT_HEAD*) bmp, C_UPDATE_ALL);
    }

}

static void win_pvr_ctlbar_draw_txt_name( TEXT_FIELD *txt ,UINT16 play_rl_idx,pvr_record_t *rec)
{
    struct list_info  rl_info;
    UINT16 unistr[30] = {0};
    pvr_play_rec_t  *pvr_info = NULL;
    P_NODE playing_pnode;
    UINT32 cur_prog_id = 0;
    P_NODE p_node;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    if(api_pvr_is_playing())
    {
        //OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)rl_info.txti);
        pvr_get_rl_info(play_rl_idx, &rl_info);
#ifdef NEW_DEMO_FRAME
#ifdef CI_PLUS_PVR_SUPPORT
        if(rl_info.is_reencrypt)
        {
            com_asc_str2uni(((rl_info.ca_mode) ? ((rl_info.rec_type) ? "[CI+PS] $" : "[CI+TS] $") :
                (((rl_info.rec_type) ? "[CI+PS] " : "[CI+TS] "))),unistr);
            com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode + 8], rl_info.txti);        
        }
        else
#endif
        {
            com_asc_str2uni(((rl_info.ca_mode ) ? ((rl_info.rec_type) ? (UINT8 *)"[PS] $" : (UINT8 *)"[TS] $") :
                (((rl_info.rec_type) ?(UINT8 *) "[PS] " : (UINT8 *)"[TS] "))),unistr);
            com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode + 5], rl_info.txti);
        }
#else
        com_asc_str2uni((rl_info.ca_mode) ? "$" : "",unistr);
        com_uni_str_copy_char((UINT8*)&unistr[rl_info.ca_mode], rl_info.txti);
#endif
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
        MEMSET(&playing_pnode,0,sizeof(P_NODE));
        ap_get_playing_pnode(&playing_pnode);
        cur_prog_id = playing_pnode.prog_id; /* to avoid poll-disk while playback cause pvr bar show error */
        MEMSET(&p_node,0,sizeof(P_NODE));
        if (((PVR_STATE_REC == pvr_info->pvr_state) || (PVR_STATE_REC_REC == pvr_info->pvr_state)) && (rec != NULL))
        {
            cur_prog_id = rec->record_chan_id;//pvr_info.rec[pvr_info.rec_last_idx].record_chan_id;
        }
        get_prog_by_id(cur_prog_id, &p_node);
        if(p_node.ca_mode)
        {
            com_asc_str2uni((UINT8 *)"$",unistr);
        }
        com_uni_str_copy_char((UINT8*)&unistr[p_node.ca_mode], p_node.service_name);
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
        //OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)p_node.service_name);
    }
}

static void win_pvr_ctlbar_draw_txt_date(TEXT_FIELD *txt,UINT16 play_rl_idx)
{
    struct list_info  rl_info;
    char string[30] = {0};
    date_time dt;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    MEMSET(&dt,0,sizeof(date_time));
    
    get_local_time(&dt);
    pvr_info  = api_get_pvr_info();
    if( pvr_info->play.play_handle != 0)
    {
        pvr_get_rl_info(play_rl_idx, &rl_info);
        snprintf(string, 30,"%02d/%02d", rl_info.tm.month, rl_info.tm.day);
    }
    else
    {
        snprintf(string,30,"%02d/%02d",dt.month,dt.day);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
}

static void win_pvr_ctlbar_draw_txt_time(TEXT_FIELD *txt,UINT16 play_rl_idx)
{
    struct list_info  rl_info;
    char string[30] = {0};
    date_time dt;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    MEMSET(&dt,0,sizeof(date_time));
	get_local_time(&dt);
    pvr_info  = api_get_pvr_info();
    if( pvr_info->play.play_handle != 0)
    {
        pvr_get_rl_info(play_rl_idx, &rl_info);
        snprintf(string, 30,"%02d:%02d", rl_info.tm.hour, rl_info.tm.min);
    }
    else
    {
        snprintf(string,30,"%02d:%02d",dt.hour,dt.min);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
    if(NULL != api_pvr_set_ttx_string(string))
    {
        return;
    }
}

static void win_pvr_ctlbar_drar_txt_play_mode(TEXT_FIELD *txt)
{
    char string[30] = {0};
    UINT8 play_speed = 0;
    PVR_STATE play_state = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    strncpy(string,"", (30-1));
    pvr_info  = api_get_pvr_info();
    if( pvr_info->play.play_handle != 0)
    {
        play_state    = pvr_p_get_state( pvr_info->play.play_handle);
        play_speed = pvr_p_get_speed( pvr_info->play.play_handle);
        if(NV_STOP == play_state)
        {
            strncpy(string, "Stop", 29);//sprintf(string,"%s","Stop");
        }
        else
        {
            if(NV_PLAY == play_state)
            {
                strncpy(string, "Play", 29);
            }
            else
            {
                if(NV_PAUSE == play_state)
                {
                    strncpy(string, "Pause", 29);
                }
                else
                {
                    if((NV_FF == play_state) || (NV_FB == play_state))
                    {
                        snprintf(string,30,"X%d",play_speed);
                    }
                    else
                    {
                        if((NV_SLOW == play_state) || (NV_REVSLOW == play_state))
                        {
                            snprintf(string,30,"X1/%d",play_speed);
                        }
                        else if(NV_STEP == play_state)
                        {
                            strncpy(string,"Step", 29);
                        }
                    }
                }
            }
        }
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
}

static void win_pvr_ctlbar_draw_txt_hdd_status(TEXT_FIELD *txt,struct dvr_hdd_info *hdd_info)
{
    char string[30] = {0};
    UINT32 step_sectors = 0;
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};

    cur_pvr_mode = pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    if(PVR_DISK_REC_AND_TMS == cur_pvr_mode)
    {
        step_sectors = (hdd_info->total_size - hdd_info->tms_size) / 100;
    }
    else
    {
        if((PVR_DISK_REC_WITH_TMS == cur_pvr_mode) || (PVR_DISK_ONLY_REC == cur_pvr_mode))
        {
            step_sectors = hdd_info->total_size / 100;
        }
        else
        {
            if(PVR_DISK_ONLY_TMS == cur_pvr_mode)
            {
                step_sectors = 1;
            }
        }
    }
    if(step_sectors >0 )
    {
        snprintf(string,30,"%ld%%",(100 - (hdd_info->rec_size+ step_sectors/2 )/step_sectors));
    }
    else
    {
        strncpy(string,"", 29);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
}

static void win_pvr_ctlbar_draw_txt_record_bitrate(TEXT_FIELD *txt,pvr_record_t *rec)
{
    char string[30] = {0};
    struct list_info  rl_info;
    UINT32 bitrate = 0;
    UINT16 rl_idx = 0;
    UINT8  __MAYBE_UNUSED__ rec_type = 0;
    struct dmx_device *dmx=NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    pvr_info  = api_get_pvr_info();
    if (( pvr_info->play.play_handle) && ( pvr_info->pvr_state != PVR_STATE_UNREC_PVR_PLAY))
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if(rl_info.is_reencrypt)
        {
            snprintf(string, 30, "ReEn");
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            return;
        }

        // rl_info.quantum_num is invalid (default to 0) when using PVR3
        if ((rl_info.is_scrambled) && (rl_info.duration) && (rl_info.quantum_num))
        {
            bitrate = rl_info.quantum_num * 47;
            bitrate /= rl_info.duration;
        }
        else
        {
        bitrate = rl_info.ts_bitrate/8096;
        }
    rec_type = rl_info.rec_type;
    }
    else if((rec != NULL)/*pvr_info.rec[pvr_info.rec_last_idx].record_chan_flag*/ || pvr_info->tms_r_handle)
    {
        dmx_io_control(dmx, GET_PROG_BITRATE, (UINT32)&bitrate);
        bitrate = bitrate/(BYTE2BIT(PVR_UINT_K));//KB/S
        if(rec != NULL)//pvr_info.rec[pvr_info.rec_last_idx].record_chan_flag)
        {
            rl_idx = pvr_get_index(rec->record_handle);//pvr_info.rec[pvr_info.rec_last_idx].record_handle);
        }
        else
        {
            rl_idx = pvr_get_index( pvr_info->tms_r_handle);
        }
        pvr_get_rl_info(rl_idx, &rl_info);
        rec_type = rl_info.rec_type;
    }
    else
    {
        bitrate = 0;
        rec_type = 0xFF;
    }
    if ( pvr_info->play.play_handle &&
         ((PVR_STATE_REC_PVR_PLAY == pvr_info->pvr_state)
         || (PVR_STATE_UNREC_PVR_PLAY == pvr_info->pvr_state)) )
    {
        snprintf(string, 30, "%5d",0);
    }
    else
    {
        snprintf(string, 30, "%5lu", bitrate);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
}

static void win_pvr_ctlbar_draw_txt_hdd_speed(TEXT_FIELD *txt,UINT16 play_rl_idx)
{
    UINT32 read_speed = 0;
    UINT32 write0_speed = 0;
    UINT32 write1_speed = 0;
    UINT32 write2_speed = 0;
    char string[30] = {0};
    struct list_info  rl_info;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 rec_type = 0;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
#ifdef CI_PLUS_PVR_SUPPORT
    if( pvr_info->play.play_handle != 0)
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if((rl_info.is_reencrypt) && (PVR_COPY_NEVER == rl_info.copy_control ))
        {
            UINT32 rl_a_time = 0;
            INT32 day=0;
            INT32 hour=0;
            INT32 min=0;
            INT32 sec=0;
            struct PVR_DTM a_time={0};
            struct PVR_DTM cur_time={0};
            UINT32 retention_time = 0;

            retention_time = _pvr_data_trans_retention_time(rl_info.retention_limit);

            pvr_info->play.rl_a_time = 0;

            _pvr_get_local_time(&cur_time);
            _pvr_convert_time_by_offset(&a_time, &cur_time,
                                        -(retention_time/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN)),
                                        -((retention_time%3600)/60));
            // check Start <= A_time
            if(__pvr_compare_time(&rl_info.tm, &a_time) <= 0)
            {
                _pvr_get_time_offset(&rl_info.tm, &a_time, &day, &hour, &min, &sec);
                rl_a_time += day*PVR_TIME_HOUR_PER_DAY*PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN;
                rl_a_time += hour*PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN;
                rl_a_time += min*PVR_TIME_S_PER_MIN+ sec + 1;

                // check the rl_A_time < duration
                if(rl_info.duration > rl_a_time)
                {
                    pvr_info->play.rl_a_time = rl_a_time + 1;
                }
                else
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR,
                        PVR_END_DATAEND + (pvr_msg_hnd2idx( pvr_info->play.play_handle)<<HND_INDEX_OFFSET), TRUE);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                }
            }
       snprintf(string, 30, "RL: %02d:%02d:%02d",pvr_info->play.rl_a_time/((PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN)),
                    ( pvr_info->play.rl_a_time%(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR,
                    pvr_info->play.rl_a_time%PVR_TIME_S_PER_MIN);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;
        }
    }
#endif
    MEMSET(string,0,sizeof(string));
    pvr_get_real_time_rw_speed(&read_speed, &write0_speed, &write1_speed,&write2_speed);
    if (( pvr_info->play.play_handle) && ((PVR_STATE_REC_PVR_PLAY ==  pvr_info->pvr_state)
                || (PVR_STATE_UNREC_PVR_PLAY == pvr_info->pvr_state )) )
    {
         snprintf(string, 30, "%5lu %5d ",read_speed,0);
    }
    else
    {
        snprintf(string, 30, "%5lu %5lu ",read_speed,write0_speed+write1_speed);
    }
    pvr_get_rl_info(play_rl_idx, &rl_info);
    rec_type= rl_info.rec_type;
    if(0==rec_type)
    {
        snprintf(string+12, 30 - 12, "TS  ");
    }
    else
    {
        if(1==rec_type)
        {
            snprintf(string+12, 30 - 12, "PS  ");
        }
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
}

void win_pvr_get_play_2ndtime(char *string, int str_size)
{
    UINT32 rtm = 0;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    UINT8 rec_pos = 0;
    pvr_record_t *rec = NULL;
    date_time *pstartdt = NULL;
    date_time *  __MAYBE_UNUSED__ penddt = NULL;
    date_time dt;
    struct list_info rl_info;
    UINT16 i = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&dt,0,sizeof(date_time));
    MEMSET(&rl_info,0,sizeof(struct list_info));
    pvr_info  = api_get_pvr_info();
    if(0 == pvr_info->play.play_handle )
    {
        rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    }
    else
    {
        for(i=0; i< pvr_info->rec_num; i++)
        {
            rec = api_pvr_get_rec(i + 1);
            if (NULL == rec)
            {
                continue;
            }
            if(pvr_get_index( pvr_info->play.play_handle) == pvr_get_index(rec->record_handle))
            {
                break;
            }
            rec = NULL;
        }
    }
    if(rec != NULL)
    {
        pstartdt    = &rec->record_start_time;
        penddt = &rec->record_end_time;
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &rl_info);
    }
    else
    {
        pstartdt = NULL;
        penddt = NULL;
    }
    strncpy(string,"", str_size-1);
    if((pvr_play_goon) && (rtm_last))
    {
        rtm = rtm_last;
        hh = rtm/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
        mm = (rtm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
        ss = (rtm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
        snprintf(string, str_size, "%02lu:%02lu:%02lu",hh,mm,ss);
    }
    else
    {
        if(pstartdt)
        {
            snprintf(string,str_size,"%02d:%02d:%02d",pstartdt->hour,pstartdt->min,pstartdt->sec);
        }
        else
        {
            rtm = api_pvr_gettotaltime_byhandle( pvr_info->play.play_handle);
            rtm_last = rtm;
            hh = rtm/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
            mm = (rtm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
            ss = (rtm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
            snprintf(string, str_size, "%02lu:%02lu:%02lu",hh,mm,ss);
        }
    }

    //return string;
}
static void win_pvr_ctlbar_draw_txt(BOOL cur_prog_live_play_only,UINT16 play_rl_idx,pvr_record_t *rec,
    struct dvr_hdd_info *hdd_info)
{
    TEXT_FIELD *txt = &record_text;
    record_txt_t *txt_desc = NULL;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    char string[30] = {0};
    UINT32 rtm = 0;
    INT32 time_used = 0;
    INT32 time_total = 0;
    UINT8 i=0;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL draw_all_item = FALSE;

    pvr_info  = api_get_pvr_info();
    for(i=0;i<RECORD_TXT_NUM;i++)
    {
        txt_desc = &record_txts[i];
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");

        if (cur_prog_live_play_only &&
            ((RECORD_TXT_PLAY_1ST_TIME == txt_desc->txt_type)
            || (RECORD_TXT_PLAY_2ND_TIME == txt_desc->txt_type  )
            || (RECORD_TXT_RECORD_1ST_TIME ==txt_desc->txt_type)
            || (RECORD_TXT_RECORD_2ND_TIME == txt_desc->txt_type)
            || (RECORD_TXT_RECORD_BITRATE ==txt_desc->txt_type)
            || (RECORD_TXT_HDD_SPEED == txt_desc->txt_type)))
        {
            continue;
        }

        if(pvr_info->pvr_state != PVR_STATE_IDEL &&
            ((RECORD_TXT_RECORD_BITRATE == txt_desc->txt_type)
             || (RECORD_TXT_HDD_SPEED == txt_desc->txt_type)))
        {
            draw_all_item = TRUE;
        }
        else
        {
            draw_all_item = FALSE;
        }

        if (!draw_all_item)
        {
        /* If not in recording status, or not in rec_pvr_play state, don't show recording information. */
            if((!api_pvr_is_rec_only_or_play_rec() || pvr_play_goon) &&
                ( (RECORD_TXT_RECORD_1ST_TIME == txt_desc->txt_type)
                 || (RECORD_TXT_RECORD_2ND_TIME == txt_desc->txt_type)
                 || (RECORD_TXT_RECORD_BITRATE == txt_desc->txt_type)
                 || (RECORD_TXT_HDD_SPEED == txt_desc->txt_type)))
            {
                continue;
            }
            else
            {
                draw_all_item = TRUE;
            }
        }

        if (draw_all_item)
        {
            switch(txt_desc->txt_type)
            {
            case RECORD_TXT_PLAY_NAME:
                win_pvr_ctlbar_draw_txt_name(txt,play_rl_idx,rec);
                break;
            case RECORD_TXT_DATE:
                win_pvr_ctlbar_draw_txt_date(txt,play_rl_idx);
                break;
            case RECORD_TXT_TIME:
                win_pvr_ctlbar_draw_txt_time(txt,play_rl_idx);
                break;
            case RECORD_TXT_PLAY_MODE:
                win_pvr_ctlbar_drar_txt_play_mode(txt);
                break;
            case RECORD_TXT_PLAY_1ST_TIME:
                win_pvr_get_play_1sttime(string, 30);
                osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
                break;
            case RECORD_TXT_PLAY_2ND_TIME:
                win_pvr_get_play_2ndtime(string, 30);
                osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
                break;
            case RECORD_TXT_HDD_STATUS:
                win_pvr_ctlbar_draw_txt_hdd_status(txt,hdd_info);
                break;
            case RECORD_TXT_RECORD_1ST_TIME:
                if(rec != NULL)//pvr_info.rec[pvr_info.rec_last_idx].record_chan_flag)
                {
                    rtm = api_pvr_gettotaltime_byhandle( pvr_info->play.play_handle);
                    hh = rtm/3600;
                    mm = (rtm - hh*3600)/60;
                    ss = (rtm - hh*3600 - mm*60);
                    //sprintf(string,"%02d:%02d:%02d",pstartdt->hour,pstartdt->min,pstartdt->sec);
                    snprintf(string, 30, "%02lu:%02lu:%02lu",hh,mm,ss);
                    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
                }
                break;
            case RECORD_TXT_RECORD_2ND_TIME:
                if(rec != NULL)//pvr_info.rec[pvr_info.rec_last_idx].record_chan_flag)
                {
                    get_rec_time(rec->record_handle, &time_used, &time_total);//in sec
                    hh = time_total/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
                    mm = (time_total - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
                    ss = (time_total - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
                    snprintf(string, 30, "%02lu:%02lu:%02lu",hh,mm,ss);
                    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
                }
                break;
            case RECORD_TXT_RECORD_BITRATE:
                win_pvr_ctlbar_draw_txt_record_bitrate(txt,rec);
                break;
            case RECORD_TXT_HDD_SPEED:
                win_pvr_ctlbar_draw_txt_hdd_speed(txt,play_rl_idx);
                break;
            default:
                break;
            }
        }

        osd_set_rect(&txt->head.frame, txt_desc->left, txt_desc->top, txt_desc->width,txt_desc->height);
        txt->head.style.b_show_idx = txt_desc->shidx;
        osd_draw_object( (OBJECT_HEAD *)txt, C_UPDATE_ALL);

    }
}
static void draw_bookmark(UINT32 rtm)
{
    UINT32 i = 0;
    UINT32 play_pos = 0;
    BITMAP *bmp = &record_bmp;
    PROGRESS_BAR *bar = &record_bar;
    INT16 tmp_val = 0;
    UINT16 left = 0;
    UINT16 top = 0;
    UINT16 width = 0;
    UINT16 height = 0;

    api_pvr_update_bookmark();

    if(api_pvr_get_bookmark_num() && rtm)
    {
        for(i=0; i<api_pvr_get_bookmark_num(); i++)
        {
            //TODO: draw bookmarks
            play_pos = api_pvr_get_bookmark_ptm_by_index(i)*100/rtm;
            tmp_val = osd_get_progress_bar_pos(bar);
            left = bar->head.frame.u_left + bar->head.frame.u_width*play_pos/100*tmp_val/100- POS_ICON_WIDTH/2;
            top = BAR_T+10;
            width = POS_ICON_WIDTH;
            height = bar->head.frame.u_height;
            osd_set_rect(&bmp->head.frame, left, top, width, height);
            osd_set_bitmap_content(bmp, IM_PVR_ICON_BOOKMARK);
            osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
        }
    }
}

static void win_pvr_ctlbar_draw_bar(BOOL cur_prog_live_play_only,pvr_record_t *rec )
{
    UINT8 i=0;
    BITMAP *bmp = &record_bmp;
    UINT32 play_pos = 0;
    PROGRESS_BAR *bar = &record_bar;
    record_bar_t *bar_desc = NULL;
    UINT32 rec_size = 0;
    UINT32 ptm = 0;
    UINT32 rtm = 0;
    INT32 time_used = 0;
    INT32 time_total = 0;
    UINT16 rl_idx = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 tmp_val1 = 0;
    UINT16 tmp_val2 = 0;
    UINT32 tmp = 0;
    UINT16 left = 0;
    UINT16 top = 0;
    UINT16 width = 0;
    UINT16 height = 0;

    pvr_info  = api_get_pvr_info();
    if((PVR_STATE_REC == pvr_info->pvr_state) || (PVR_STATE_REC_REC == pvr_info->pvr_state))
    {
        i=1;
    }
    else
    {
        i=0;
    }
    if (cur_prog_live_play_only)
    {
        i = RECORD_BAR_NUM; // not draw any bar
    }

    for(;i<RECORD_BAR_NUM;i++)
    {
        bar_desc = &record_bars[i];

        /* recorded size */
        switch(bar_desc->bar_type)
        {
        case RECORD_BAR_PLAY:
            rec_size = 0;
            if( pvr_info->play.play_handle != 0)
            {
                    rec_size = POS_HIGH_LIMIT;    /* Recorded play */
            }
            else
            {
                if( pvr_info->tms_r_handle != 0)
                {
                    rtm = pvr_r_get_time( pvr_info->tms_r_handle );
                    tmp = pvr_get_tms_capability();
                    if (0 == tmp )
                    {
                        rec_size = POS_HIGH_LIMIT;
                    }
                    else
                    {
                        rec_size = rtm*POS_HIGH_LIMIT/tmp;
                    }
                }
            }
            if(rec_size > POS_HIGH_LIMIT)
            {
                rec_size = POS_HIGH_LIMIT;
            }
            osd_set_progress_bar_pos(bar, rec_size);
            break;
        case RECORD_BAR_RECORD:
            if(rec != NULL)//pvr_info.rec[pvr_info.rec_last_idx].record_chan_flag)
            {
                get_rec_time(rec->record_handle, &time_used, &time_total);//in sec

                if(time_total<=0)
                {
                    rec_size = POS_HIGH_LIMIT;
                }
                else
                {
                    rec_size = POS_HIGH_LIMIT*time_used/time_total;
                }
                if(rec_size > POS_HIGH_LIMIT)
                {
                    rec_size = POS_HIGH_LIMIT;
                }
                osd_set_progress_bar_pos(bar, rec_size);
            }
            break;
        default:
            break;
        }

        /* If not in recording status, and not in rec_pvr_play state, don't show recording information. */
        if(((!api_pvr_is_rec_only_or_play_rec()) || (pvr_play_goon))
            && (RECORD_BAR_RECORD == bar_desc->bar_type))
        {
            continue;
        }
        osd_set_rect(&bar->head.frame, bar_desc->left, bar_desc->top, bar_desc->width,bar_desc->height);
    #ifndef SD_UI
        bar->rc_bar.u_left     = 0;
        bar->rc_bar.u_top      =0;
        bar->rc_bar.u_width    = bar_desc->width;
        bar->rc_bar.u_height   = bar_desc->height;
        bar->w_tick_fg         = bar_desc->shidx;
    #else
        bar->rc_bar.u_left     = 2;
        bar->rc_bar.u_top      = 2;
        bar->rc_bar.u_width    = bar_desc->width - 4;
        bar->rc_bar.u_height   = bar_desc->height - 4;
        bar->w_tick_fg         = bar_desc->shidx;
    #endif

        osd_draw_object((POBJECT_HEAD)bar, C_UPDATE_ALL);

        if(RECORD_BAR_PLAY == bar_desc->bar_type)
        {
            /* Draw play pos */
            play_pos = win_pvr_get_play_pos();

            if(api_pvr_get_timesrch(&rtm,&ptm,&rl_idx) && api_pvr_get_tmsrch_display())
            {
                tmp_val1 = api_pvr_get_tmsrch_pos();
                tmp_val2 = osd_get_progress_bar_pos(bar);
                left = bar->head.frame.u_left+6+(bar->head.frame.u_width-12)*tmp_val1/100*tmp_val2/100-POS_ICON_WIDTH/2;
                top = bar->head.frame.u_top;
                width = POS_ICON_WIDTH;
                height = bar->head.frame.u_height;
                osd_set_rect(&bmp->head.frame, left, top, width, height);
                osd_set_bitmap_content(bmp, IM_PVR_DOT_ORANGE);
                osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
            }
            else
            {
                api_pvr_set_tmsrch_display(0);
            }
            tmp_val2 = osd_get_progress_bar_pos(bar);
            left = bar->head.frame.u_left+6+(bar->head.frame.u_width - 12)*play_pos/100*tmp_val2/100- POS_ICON_WIDTH/2;
            top = bar->head.frame.u_top;
            width = POS_ICON_WIDTH;
            height = bar->head.frame.u_height;
            osd_set_rect(&bmp->head.frame, left, top, width, height);
            osd_set_bitmap_content(bmp, IM_PVR_DOT);
            osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
            draw_bookmark(rtm);
        }
    }
}
void win_pvr_ctlbar_draw_infor(void)
{
    UINT32 i = 0;
    date_time *  __MAYBE_UNUSED__ pstartdt = NULL;
    date_time *  __MAYBE_UNUSED__ penddt = NULL;
    UINT16 play_rl_idx = 0;
    pvr_record_t *rec = NULL;
    UINT8 rec_pos = 0;
    BOOL cur_prog_live_play_only = FALSE;
    struct list_info  rl_info;
    struct dvr_hdd_info hdd_info;
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(&rl_info,0,sizeof(struct list_info));
    MEMSET(&hdd_info,0,sizeof(struct dvr_hdd_info));
    osd_draw_recode_ge_cmd_start();
    pvr_info  = api_get_pvr_info();
    if(0 == pvr_info->play.play_handle)
    {
        rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    }
    else
    {
        for(i=0; i< pvr_info->rec_num; i++)
        {
            rec = api_pvr_get_rec(i + 1);
            if (NULL == rec)
            {
                continue;
            }
            if(pvr_get_index( pvr_info->play.play_handle) == pvr_get_index(rec->record_handle))
            {
                break;
            }
            rec = NULL;
        }
    }

    if(rec != NULL)
    {
        pstartdt    = &rec->record_start_time;
        penddt = &rec->record_end_time;
        pvr_get_rl_info(pvr_get_index(rec->record_handle), &rl_info);
    }

    if( pvr_info->tms_r_handle != 0)//get timeshift partiation info to display
    {
        pvr_get_tms_hdd(&hdd_info);
    }
    else
    {
        pvr_get_hdd_info(&hdd_info);
    }
    if( pvr_info->play.play_handle != 0)
    {
        play_rl_idx = pvr_get_index( pvr_info->play.play_handle );
    }
    else
    {
        play_rl_idx = 0;
    }

    if( pvr_info->tms_r_handle != 0)
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->tms_r_handle), &rl_info);
    }

    if ((NULL == rec) && (0 == pvr_info->play.play_handle) && (0 ==  pvr_info->tms_r_handle))
    {
        cur_prog_live_play_only = TRUE; // current prog is live playing only
    }

    win_pvr_ctlbar_draw_bmp(&hdd_info,cur_prog_live_play_only,rec,play_rl_idx);
    win_pvr_ctlbar_draw_txt(cur_prog_live_play_only,play_rl_idx,rec,&hdd_info);
    win_pvr_ctlbar_draw_bar(cur_prog_live_play_only,rec);
    osd_draw_recode_ge_cmd_stop();
}


void win_pvr_ctlbar_draw(void)
{
    POBJECT_HEAD pobj = NULL;

    pobj = (POBJECT_HEAD)&g_win_pvr_ctrlbar;

    if((api_pvr_get_updata_infor())&& (!osd_get_vscr_state()))
    {
        win_pvr_ctlbar_draw_infor();
    }
    else
    {
        osd_draw_object(pobj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

void win_pvr_get_play_1sttime(char *string, int str_size)
{
    UINT32 rtm = 0;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    UINT8 rec_pos = 0;
    UINT16 i = 0;
    UINT32 ptm = 0;
    pvr_record_t *rec = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    if(0 == pvr_info->play.play_handle)
    {
        rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
    }
    else
    {
        for(i=0; i< pvr_info->rec_num; i++)
        {
            rec = api_pvr_get_rec(i + 1);
            if (NULL == rec)
            {
                continue;
            }
            if(pvr_get_index( pvr_info->play.play_handle) == pvr_get_index(rec->record_handle))
            {
                break;
            }
            rec = NULL;
        }
    }

    strncpy(string,"", str_size-1);
    switch(ap_pvr_set_state())
    {
        case PVR_STATE_TMS_PLAY:
        case PVR_STATE_REC_TMS_PLAY:
        case PVR_STATE_REC_TMS:
            rtm = pvr_r_get_time( pvr_info->tms_r_handle);
            ptm = pvr_info->play.play_handle ? pvr_p_get_time( pvr_info->play.play_handle) : 0;
            if(!ptm && api_pvr_get_first_time_ptm())
            {
                api_pvr_set_first_time_ptm(FALSE);
                osal_task_sleep(100);
                ptm = pvr_p_get_time( pvr_info->play.play_handle);
            }
            if(rtm >= ptm)
            {
                hh = ptm/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
                mm = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
                ss = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
                snprintf(string, str_size, "%02lu:%02lu:%02lu",hh,mm,ss);
            }
            break;
        case PVR_STATE_REC_PLAY:
        case PVR_STATE_REC_REC_PLAY:
            if (rec)
            {
                rtm = pvr_r_get_time(rec->record_handle);
            }
            ptm = pvr_p_get_time( pvr_info->play.play_handle);
            if(!ptm && api_pvr_get_first_time_ptm())
            {
                api_pvr_set_first_time_ptm(FALSE);
                osal_task_sleep(100);
                ptm = pvr_p_get_time( pvr_info->play.play_handle);
            }
            if(rtm >= ptm)
            {
                hh = ptm/(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN);
                mm = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
                ss = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
                snprintf(string, str_size, "%02lu:%02lu:%02lu",hh,mm,ss);
            }
            break;
        case PVR_STATE_UNREC_PVR_PLAY:
        case PVR_STATE_REC_PVR_PLAY:
        case PVR_STATE_REC_REC_PVR_PLAY:
        case PVR_STATE_REC_TMS_PLAY_HDD:
            ptm = pvr_p_get_time( pvr_info->play.play_handle);
            hh = ptm/3600;
            mm = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN))/PVR_TIME_MIN_PER_HOUR;
            ss = (ptm - hh*(PVR_TIME_MIN_PER_HOUR*PVR_TIME_S_PER_MIN) - mm*PVR_TIME_S_PER_MIN);
            snprintf(string, str_size, "%02lu:%02lu:%02lu",hh,mm,ss);
            break;
        default:
            break;
    }

    return ;
}

UINT32 win_pvr_get_play_pos(void)
{
    UINT32 ptm = 0;
    UINT32 rtm = 0;
    UINT16 play_rl_idx = 0;
    UINT16 rl_idx = 0;
    INT32 time_used = 0;
    UINT32 play_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(0 == api_pvr_get_timesrch(&rtm,&ptm,&rl_idx))
    {
        return play_pos;
    }
    if( pvr_info->play.play_handle != 0)
    {
        if((pvr_play_goon) && (rtm_last))
        {
            rtm = rtm_last;
        }
        play_rl_idx = pvr_get_index( pvr_info->play.play_handle);

        if((TMS_INDEX == play_rl_idx)&& rtm>pvr_get_tms_capability())
        {
            time_used = rtm - pvr_get_tms_capability();
            rtm = pvr_get_tms_capability();
            if(ptm >= (UINT32)time_used)
            {
                ptm -= time_used;
            }
            else
            {
                ptm = 0;
            }
        }

        if(rtm != 0)
        {
            play_pos = ptm*POS_HIGH_LIMIT/rtm;
        }
        else
        {
            play_pos = POS_HIGH_LIMIT;
        }
    }
    else
    {
        play_pos = POS_HIGH_LIMIT;
    }
    if(play_pos > POS_HIGH_LIMIT)
    {
        play_pos = POS_HIGH_LIMIT;
    }

    return play_pos;
}

void api_set_pvr_status_icons(void)
{
    UINT16 tmp_icons = 0;

    tmp_icons = play_status_icons[3];
    play_status_icons[3] = play_status_icons[5];
    play_status_icons[5] = tmp_icons;

    tmp_icons = play_status_icons[6];
    play_status_icons[6] = play_status_icons[7];
    play_status_icons[7] = tmp_icons;
    return;
}

