/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc_port_29.c
*
* Description:
*     process the program channel change for former IC 29 serial sub function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/cic/cic.h>
#include <api/libci/ci_plus.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_device_manage.h>

#include <bus/tsi/tsi.h>
#include<api/libsi/lib_epg.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/lib_nvod.h>
#include <api/libsi/sie_monitor.h>
#include <api/libpub/lib_cc.h>

/*****************************************************************************
*
*                       Porting API For PUB29
*
******************************************************************************/
#ifdef PUB_PORTING_PUB29

static UINT32 cur_channel_index = -1;

/******************************************************************************
*           Internal Function
******************************************************************************/
static void get_ch_pids(P_NODE* p_node,UINT16* audio_pid,UINT16* ttx_pid, UINT16* subt_pid,UINT32* audioidx);

static void get_ch_pids(P_NODE* p_node,UINT16* audio_pid,UINT16* ttx_pid,UINT16* subt_pid,UINT32* audioidx)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT8 lang_in_db[MAX_AUDIO_CNT][4];
    UINT32 audio_cnt = 0;

    if ((NULL == p_node)||(NULL == audio_pid)||(NULL == ttx_pid)||(NULL == subt_pid)||(NULL == audioidx))
    {
        ASSERT(0);
        return;
    }
    MEMSET(lang_in_db,0,sizeof(lang_in_db));
    audio_cnt = p_node->audio_count;
    if(audio_cnt>MAX_AUDIO_CNT)
    {
        audio_cnt = MAX_AUDIO_CNT;
    }

 #ifdef _DTGSI_ENABLE_   // for audio select pid not match UI
    *audio_pid = p_node->audio_pid[p_node->cur_audio];
    *audioidx = p_node->cur_audio;
#else
    if(audio_cnt == 0)
    {
        *audio_pid = 0x1FFF;
        *audioidx = 0;
    }
    else if(p_node->cur_audio<audio_cnt)
    {
        *audio_pid = p_node->audio_pid[p_node->cur_audio];
        *audioidx = p_node->cur_audio;
    }
    else
    {
        *audio_pid = p_node->audio_pid[0];
        *audioidx = 0;
    }

    /* If this channel's audio PID has not been selected by user,then decide
      the audio PID by audio language setting */
    if( !p_node->audio_select)
    {
        for(i=0;i<audio_cnt;i++)
        {
            get_audio_lang3b((UINT8 *)&p_node->audio_lang[i],lang_in_db[i]);
        }

        for(i=0;i<l_cc_global_lang_num;i++)
        {
            for(j=0;j<audio_cnt;j++)
            {
                if( STRCMP(l_cc_global_lang_group[i],lang_in_db[j]) == 0)
                {
                    *audio_pid = p_node->audio_pid[j];
                    *audioidx = j;
                    goto GET_AUDIO_PID_DONE;
                }
                else
                {
                    for(k=0;k<ISO_639_LANG_MULTICODE_NUM;k++)
                    {
                        if((STRCMP(iso_639_lang_multicode[k][0], l_cc_global_lang_group[i])==0)
                            && (STRCMP(iso_639_lang_multicode[k][1], lang_in_db[j])==0))
                        {
                            *audio_pid = p_node->audio_pid[j];
                            *audioidx = j;
                            goto GET_AUDIO_PID_DONE;
                        }
                    }
                }
            }
        }
    }

GET_AUDIO_PID_DONE:
 #endif

    *ttx_pid     = p_node->teletext_pid;
    *subt_pid    = p_node->subtitle_pid;
}


BOOL uich_chg_aerial_signal_monitor(UINT32 sfreq, UINT16 bandwidth)
{
    struct libpub_cmd cmd;
    //struct cc_device_list dev_list;
    INT32 ret = RET_FAILURE;

    ENTER_PUB_API();

    MEMSET(&cmd, 0, sizeof(cmd));
    cmd.cmd_type = LIBPUB_CMD_CC_AERIAL_CHG_CH;
    cmd.para1 = sfreq;
    cmd.para2 = bandwidth;
    //MEMSET(&dev_list, 0, sizeof(dev_list));
    //dev_list.vdec = vdec;
    //MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

BOOL prt_pre_play_channel(P_NODE *p_node, struct ft_frontend *ft, \
                struct cc_param *param, BOOL *ci_start_service, BOOL bk_play)
{
    T_NODE t_node;
    S_NODE s_node;
    UINT8 nim_id = 0;
    UINT8 dmx_id = 0;
    UINT8 vdec_id = 0;
    UINT8 ts_id = 0;
    UINT8 tmp_nim_id = 0;

    if ((NULL==p_node))
    {
        return FALSE;
    }
    MEMSET(&t_node, 0, sizeof(t_node));
    MEMSET(&s_node, 0, sizeof(s_node));
    *ci_start_service = FALSE;

    if(SUCCESS!=get_tp_by_id(p_node->tp_id, &t_node))
    {
        return FALSE;
    }

    tmp_nim_id = lib_nimng_request_play(p_node->prog_id);
    if(0 == tmp_nim_id)
    {
        return FALSE;
    }

    if(ft != NULL)
    {
        MEMSET(ft, 0, sizeof(struct ft_frontend));

/*step1--- choice nim */
        nim_id = tmp_nim_id-1;

        get_sat_by_id(t_node.sat_id, &s_node);

        if(nim_id == 0)
        {
            p_node->tuner1_valid = 1;
            p_node->tuner2_valid = 0;
            s_node.tuner1_valid = p_node->tuner1_valid;
            s_node.tuner2_valid = p_node->tuner2_valid;
        }
        else
        {
            p_node->tuner1_valid = 0;
            p_node->tuner2_valid = 1;
            s_node.tuner1_valid = p_node->tuner1_valid;
            s_node.tuner2_valid = p_node->tuner2_valid;
        }

        ft->xpond.t_info.type = t_node.ft_type;
        ft->xpond.t_info.tp_id = t_node.tp_id;
        ft->xpond.t_info.frq = t_node.frq;
        ft->xpond.t_info.sym = t_node.sym;
        ft->xpond.t_info.band_width = t_node.bandwidth*1000;
        ft->xpond.t_info.fft_mode = t_node.FFT;
        ft->xpond.t_info.guard_intl = t_node.guard_interval;
        ft->xpond.t_info.modulation = t_node.modulation;
        ft->xpond.t_info.usage_type = 0x2;//channel change
        ft->xpond.t_info.fec = t_node.fec_inner; //ft->xpond.t_info.pol
        ft->xpond.t_info.inverse = t_node.inverse;

/*step2--- choice vdec */
        vdec_id = 0; //vdec_id = is_pip_chgch ? 1 : 0;

/*step3--- choice dmx */
        dmx_id = nim_id;
        ft->ts_route.dmx_id = dmx_id;

/*step4--- choice TS according to CI state! */
        ft->ts_route.ts_route_enable = 1;
        ft->ts_route.tsiid = (nim_id==0)? TSI_SPI_0 : TSI_SPI_1;
        ft->ts_route.tsi_mode = (nim_id == 0) ? 0x83 : 0x9B;
        ts_id = (nim_id==0)? TSI_TS_A : TSI_TS_B;
        ft->ts_route.ts_id = ts_id;
/* check other setting */
        *ci_start_service = (p_node->ca_mode==0) ? FALSE : TRUE;
        ft->ts_route.ci_enable = *ci_start_service;
        ft->ts_route.ci_slot_mask = 3;
        ft->nim = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    }

    if(param != NULL)
    {
        MEMSET(param, 0, sizeof(struct cc_param));

        param->es.sim_id = -1;
        param->es.blk_enable = 1;
        param->es.prog_id = p_node->prog_id;
        param->es.sat_id = p_node->sat_id;
        param->es.tp_id = p_node->tp_id;
        param->es.service_id = p_node->prog_number;
        param->es.service_type = p_node->service_type;
        param->es.pcr_pid = p_node->pcr_pid;
        param->es.v_type = MPEG2_VIDEO_STREAM;
        param->es.v_pid = (p_node->av_flag==PROG_TV_MODE) ? \
            p_node->video_pid : INVALID_PID;
        if(0 == param->es.v_pid)
        {
            param->es.v_pid = INVALID_PID;
        }
        param->es.a_type = AUDIO_MPEG2;
#ifdef HDTV_SUPPORT
        if((param->es.v_pid != INVALID_PID) && p_node->h264_flag)
        {
            param->es.v_pid |= H264_VIDEO_PID_FLAG;
        }
#endif
        // This for default language is not the first langusge. BUG17206
        if (!p_node->audio_select)
        {
            UINT16  audio_pid = 0,ttx_pid = 0,subt_pid = 0;
            UINT32  audio_idx = 0;
            get_ch_pids(p_node,&audio_pid,&ttx_pid,&subt_pid,&audio_idx);
            if(p_node->cur_audio != audio_idx)
            {
                p_node->cur_audio= audio_idx;
                modify_prog(p_node->prog_id, p_node);
            }
        }
        param->es.a_pid = (p_node->audio_count) ? \
            p_node->audio_pid[p_node->cur_audio] : INVALID_PID;
        param->es.audio_no_act = FALSE;//is_pip_chgch;
        param->es.a_ch = p_node->audio_channel;
#ifdef SYS_VOLUME_FOR_PER_PROGRAM
        param->es.a_volumn = p_node->audio_volume;
#else
        param->es.a_volumn = sys_data_get_volume();//sys_data->volume;
#endif

        if(param->es.a_pid & AC3_DES_EXIST)
        {
            param->es.a_type = AUDIO_AC3;
        }
        else if(param->es.a_pid & AAC_DES_EXIST)
        {
            param->es.a_type = AUDIO_MPEG_AAC;
        }
        else if(param->es.a_pid & ADTS_AAC_DES_EXIST)
        {
            param->es.a_type = AUDIO_MPEG_ADTS_AAC;
        }

        param->es.spdif_enable = 0;
#if(AC3DEC==0)
        param->es.spdif_enable = 1;
#endif
        param->es.dmx_2play = 0;

        param->dev_list.vpo = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        param->dev_list.vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, vdec_id);
        param->dev_list.deca = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        param->dev_list.snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        param->dev_list.dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
#ifdef HDTV_SUPPORT
        if(p_node->h264_flag)
        {
            param->dev_list.vdec_stop = \
                (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        }
        param->dev_list.vdec_hdtv = \
            (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
#endif
    }

    return TRUE;
}

/*****************************************************************************
*           External Function
******************************************************************************/


void set_cc_notify_callback(cc_notify_callback callback)
{
    post_play_callback = callback;
}


BOOL uich_chg_play_prog(UINT8 avflag,UINT32 u_channel_index)
{
    P_NODE p_node;
    struct ft_frontend frontend;
    struct cc_param chchg_param;
    BOOL ci_start_service = FALSE;

    if(get_prog_by_id(u_channel_index, &p_node) != SUCCESS)
    {
        return FALSE;
    }

    if(TRUE != prt_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, FALSE))
    {
        return FALSE;
    }

    cur_channel_index = u_channel_index;
    chchg_play_channel(&frontend, &chchg_param, pub_blk_scrn);

    return TRUE;
}

BOOL uich_chg_stop_prog(BOOL is_need_blk_scrn)
{
    P_NODE p_node;
    struct ft_frontend frontend;
    struct cc_param chchg_param;
    BOOL ci_start_service = FALSE;
    //UINT32 tick= osal_get_tick();

    MEMSET(&p_node, 0, sizeof(p_node));
    MEMSET(&frontend, 0x0, sizeof(struct ft_frontend));
    if(get_prog_by_id(cur_channel_index, &p_node) != SUCCESS)
    {
        return FALSE;
    }

    if(TRUE != prt_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, FALSE))
    {
        return FALSE;
    }

#ifdef HDTV_SUPPORT
    chchg_param.es.v_pid &= 0x1fff;
#endif

    chchg_stop_channel(&(chchg_param.es), &(chchg_param.dev_list), is_need_blk_scrn);

    cur_channel_index = -1;

    return TRUE;
}
BOOL uich_chg_pause_prog(void)
{
    //P_NODE p_node;
    struct vdec_device* v_dec = NULL;

    v_dec = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
#ifdef HDTV_SUPPORT
    //if(get_prog_by_id(cur_channel_index, &p_node) != SUCCESS)
    //  return FALSE;
    //if(p_node.h264_flag)
    //if(is_cur_decoder_avc())
    {
        v_dec = (struct vdec_device *)get_selected_decoder();//(struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    }
#endif
    chchg_pause_video(v_dec);

    return TRUE;
}

BOOL uich_chg_resume_prog(void)
{
    struct vdec_device* v_dec = NULL;

    v_dec = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
#ifdef HDTV_SUPPORT
    //if(get_prog_by_id(cur_channel_index, &p_node) != SUCCESS)
    //  return FALSE;
    //if(p_node.h264_flag)
    //if(is_cur_decoder_avc())
    {
        v_dec = (struct vdec_device *)get_selected_decoder();//(struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    }
#endif
    chchg_resume_video(v_dec);

    return TRUE;
}

BOOL uich_chg_show_logo(UINT8* p_logo_data,UINT32 u_data_size,BOOL is_sync)
{
    P_NODE p_node;
    struct cc_logo_info logo;
    struct cc_device_list dev_list;
    UINT32 tick = osal_get_tick();

    if (NULL == p_logo_data)
    {
        return FALSE;
    }
    MEMSET(&p_node, 0, sizeof(p_node));
    MEMSET(&logo, 0, sizeof(logo));
    MEMSET(&dev_list, 0, sizeof(struct cc_device_list));
    logo.addr = p_logo_data;
    logo.size = u_data_size;
    logo.sto = dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    dev_list.vpo = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    dev_list.vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    dev_list.deca = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    dev_list.snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    dev_list.dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

#ifdef HDTV_SUPPORT
    //if(is_cur_decoder_avc())
    {
        dev_list.vdec_stop=(struct vdec_device *)get_selected_decoder();//(struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        dev_list.vdec_hdtv=(struct vdec_device *)get_selected_decoder();//(struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    }
#endif

    chchg_show_logo(&logo, &dev_list);

    libc_printf("  show_logo %d",osal_get_tick()-tick);

    return TRUE;
}

void uich_chg_set_video_type(UINT32 flag)
{
    //libpub27_config.chchg_blk_enable = flag? FALSE : TRUE;
    pub_blk_scrn = flag? FALSE : TRUE;
}

//just for building
UINT32 uich_chg_set_aud_language(BYTE* sz_lang_group,UINT32 u_lang_num)
{
/*  UINT32 i,j;

    MEMSET(l_cc_global_lang_group,0,sizeof(l_cc_global_lang_group));
    if (uLangNum > CC_MAX_LANG_GROUP_SIZE)
        uLangNum = CC_MAX_LANG_GROUP_SIZE;
    for(i=0; i<uLangNum; i++)
    {
        for (j=0;j<3;j++)
        {
            l_cc_global_lang_group[i][j] = szLangGroup[i*4+j];
        }
    }

    l_cc_global_lang_num = uLangNum;
    return uLangNum;*/
    return 0;
}
BOOL uich_chg_switch_ach(UINT8 u_audio_channel)
{
    struct snd_device* snd = NULL;

    snd = (struct snd_device*)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    chchg_switch_audio_chan(snd, u_audio_channel);

    return TRUE;
}
BOOL uich_chg_switch_apid(UINT32 u_audio_pid)
{
    struct cc_es_info es;
    struct cc_device_list dev_list;

    MEMSET(&es,0,sizeof(es));
    MEMSET(&dev_list,0,sizeof(dev_list));

    if(u_audio_pid & AC3_DES_EXIST)
    {
        es.a_type = AUDIO_AC3;
    }
    else
    {
        es.a_type = AUDIO_MPEG2;
    }
    es.spdif_enable = 1;
    es.a_pid = u_audio_pid;
    //es.a_ch = audio_ch;
    es.a_volumn = sys_data_get_volume(); //sys_data->volume;
    dev_list.vpo = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    dev_list.vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    dev_list.deca = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    dev_list.snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    dev_list.dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
#ifdef HDTV_SUPPORT
    //if(is_cur_decoder_avc())
    {
        dev_list.vdec = (struct vdec_device *)get_selected_decoder();//(struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    }
#endif

    chchg_switch_audio_pid(&es, &dev_list);

    return TRUE;
}

//just for building, it seem not use in 3602 -T
BOOL uich_chg_apply_aud_language(void)
{
    #if 0
    struct mid27_cmd this_cmd;

    ENTER_PUB_API();

    this_cmd.cmd_type = MID27_CMD_CC_SWITCH_AID;
    this_cmd.para32 = 0xFFFFFFFF; /* special case, for apply audio lang only */

    mid27_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME);

    mid27_wait_cmd_finish(MID27_FLG_CC_SWITCH_AID_BIT,OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    #endif
    return TRUE;

}

//just for building, it seem not use in 3602 -T
void set_local_position(double longtitude,double latitude,BOOL ismutex)
{
/*  ENTER_CC_API();

    l_cc_local_longtitude = longtitude;
    l_cc_local_latitude = latitude;
    l_cc_local_ismutex = ismutex;

    LEAVE_CC_API();
    */
}
struct monitor_param mid_para = {0, NULL};
void mid_monitor_init(struct monitor_param para)
{
    if(para.event_hand != NULL)
    {
        mid_para.event_hand = para.event_hand;
    }
}
#if((SYS_PROJECT_FE == PROJECT_FE_ATSC)||( SYS_PROJECT_FE == PROJECT_FE_DVBT ))
BOOL uich_chg_lock_prog(UINT32 cur_channel,BOOL sync)
{
    struct libpub_cmd cmd;

    ENTER_PUB_API();
    MEMSET(&cmd, 0, sizeof(cmd));
    cmd.cmd_type = LIBPUB_CMD_CC_LOCK;
    cmd.sync = sync;
    cmd.para1 = cur_channel;

    libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    if(sync)
    {
        libpub_wait_cmd_finish(MID27_FLG_CC_LOCK_BIT, OSAL_WAIT_FOREVER_TIME);
    }

    LEAVE_PUB_API();
    return SUCCESS;
}
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
BOOL uich_chg_aerial_signal_monitor(UINT32 sfreq, UINT16 bandwidth)
{
    struct libpub_cmd cmd;
    //struct cc_device_list dev_list;

    ENTER_PUB_API();

    MEMSET(&cmd, 0, sizeof(cmd));
    cmd.cmd_type = LIBPUB_CMD_CC_AERIAL_CHG_CH;
    cmd.para1 = sfreq;
    cmd.para2 = bandwidth;
    //MEMSET(&dev_list, 0, sizeof(dev_list));
    //dev_list.vdec = vdec;
    //MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();

    return TRUE;
}
#endif

#endif



