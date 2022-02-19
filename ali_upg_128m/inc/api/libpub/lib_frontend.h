/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_frontend.h
*
* Description:
*     ali STB front end device and ts device manage unit
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIB_FRONTEND_H_
#define _LIB_FRONTEND_H_

#include <api/libtsi/p_search.h>
/*front end*/
//type


#ifdef __cplusplus
extern "C"
{
#endif

#define FRONTEND_TYPE_S    0x00
#define FRONTEND_TYPE_C    0x01
#define FRONTEND_TYPE_T         0x02
#define FRONTEND_TYPE_ISDBT     0x03

#define FRONTEND_LNB_FREQ_LOW       950
#define FRONTEND_LNB_FREQ_HIGH      2150

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    #define FRONTEND_TYPE   FRONTEND_TYPE_S
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
    #define FRONTEND_TYPE   FRONTEND_TYPE_C
#else
    #define FRONTEND_TYPE   FRONTEND_TYPE_T
#endif

#define CC_MAX_TS_ROUTE_NUM    5
#define DEV_DMX_SIM_CNT        10
#define frontend_lnb_22k_ctl(plnb)    ((LNB_CTRL_22K == (plnb)->lnb_type) ? 1:0)

typedef enum _positioner_type_e
{
    POSITIONER_TYPE_NONE                = 0, //no diseqc
    POSITIONER_TYPE_DISEQC_12           = 1, //diseqc 1.2
    POSITIONER_TYPE_DISEQC_13           = 2, //diseqc 1.3
}positioner_type_e;


/*************************************************************
*   front end(antenna+nim) struct
*************************************************************/
struct ft_antenna
{
    /*if no antenna setting, set this field as 0*/
    UINT8 antenna_enable;
    /*if antenna_enable=1,give the settings*/
    UINT8 lnb_type;
    UINT8    positioner_type;
    UINT8 position;
    UINT32 lnb_high;
    UINT32 lnb_low;
    UINT32 longitude;

    UINT8 pol;
    UINT8    di_seq_c_type;
    UINT8 di_seq_c_port;
    UINT8 di_seq_c11_type;
    UINT8 di_seq_c11_port;
    UINT8    k22;
    UINT8 v12;
    UINT8 toneburst;
    /*add for Unicable LNB*/
    UINT8   unicable_pos;   //position:A/B(value:0/1)
    UINT8   unicable_ub;    //user band(slot) or SCR channel:1~8(value:0~7)
    UINT16  unicable_freq;  //centre frequency of user band:1210,1420,...
};

/*only include params related to tuning*/
union ft_xpond
{
    struct
    {
        UINT8 type;
    } common;
    struct
    {
        UINT8 type;
        UINT8 modulation;
        UINT8 pol;
        UINT8 fec_inner;
        UINT32 frq;
        UINT32 sym;
        UINT32 tp_id;
#ifdef MULTISTREAM_SUPPORT
        UINT8  isid_idx;
        UINT8  isid;
        UINT8  change_type;
#endif
#ifdef PLSN_SUPPORT
        UINT8   super_scan;
        UINT32  pls_num;
#endif
    } s_info;
    struct
    {
        UINT8 type;
        UINT8 modulation;
        UINT8 fec_inner;
        UINT8 fec_outer;
        UINT32 frq;
        UINT32 sym;
        UINT32 tp_id;
    } c_info;
    struct
    {
        UINT8 type;
        UINT8 modulation;
        UINT8 fft_mode;
        UINT8 guard_intl;
        UINT8 fec;
        UINT8 usage_type;
        UINT8 inverse;
        UINT8 priority;
        UINT32 band_width;
        UINT32 frq;
        UINT32 sym;
        UINT32 tp_id;

        UINT8 t2_signal;        //0:DVB-T signal, 1:DVB-T2 signal.
        UINT8 plp_index;        //Current selected data PLP index.
        UINT8 plp_num;          //Total number of PLP within this channel.

        UINT8  plp_id;          //plp_id of plp_index.
        UINT16 t2_system_id;    //t2_system_id of this channel.
        UINT8  t2_profile;		//0:SONY_DVBT2_PROFILE_BASE,1:SONY_DVBT2_PROFILE_LITE, 2:SONY_DVBT2_PROFILE_ANY
    } t_info;
};

struct ft_ts_route
{
    UINT8 ts_route_enable;
    /*TSI_SPI_0, 1... or TSI_SSI_0, 1... etc*/
    UINT8 tsiid;
    UINT8 tsi_mode;
    /*TSI_TS_A or TSI_TS_B*/
    UINT8 ts_id;

    UINT8 ci_enable;
//    UINT8 ci_bypass;
//    UINT8 ci_slot;
    UINT8 ci_slot_mask;    //support multi slot

    /*TSI_DMX_0, 1 or 2*/
    UINT8 dmx_id;
};

struct ft_frontend
{
    struct nim_device *nim;
    struct ft_antenna antenna;
    union ft_xpond xpond;
    struct ft_ts_route ts_route;
};


/*************************************************************
*   channel change struct
*************************************************************/
struct cc_logo_info
{
    UINT8 type;
    UINT8 *addr;
    UINT32 size;
    struct sto_device *sto;
};

/*only package info needed by decoder and dmx */
struct cc_es_info
{
    UINT32 prog_id;
    UINT16 sat_id;
    UINT32 tp_id;
    UINT16 service_id;
    UINT16 service_type;

    UINT16 pcr_pid;
    /*video stream type */
    UINT8 v_type;
    /*audio stream type defined in meadiatypes.h*/
    UINT8 a_type;
    UINT16 v_pid;
    /*audio channel defined in PNODE*/
    UINT8 a_ch;
    UINT8 a_volumn;
    UINT16 a_pid;
    UINT8 spdif_enable;
    UINT16 ttx_pid;
    UINT16 subt_pid;
    UINT8 blk_enable;

    /*if need close vpo, layer VPO_LAYER_M or VPO_LAYER_PIP*/
    UINT8 close_vpo_layer;
    UINT8 audio_no_act; //used for PIP change channel
    UINT8 dmx_2play;
    UINT8 dynamic_pid_chgch;
    UINT8 background_play; //used for dual-record timer
    UINT8 not_oprate_subt_ttx;
    UINT8 sim_onoff_enable; // Enable or Disable SI Monitor On & Off
    UINT32 sim_id;//si monitor id
#ifdef AUDIO_DESCRIPTION_SUPPORT
    UINT16 ad_pid;
#endif

#ifdef FSC_SUPPORT  
    UINT8 epg_enable;
#endif
};

struct cc_device_list
{
        struct vpo_device *vpo;
        struct vdec_device *vdec;
        struct vdec_device *vdec_stop;
        struct deca_device *deca;
        struct snd_device *snd_dev;
        struct dmx_device *dmx;
    struct vdec_device *vdec_hdtv;
};

#ifdef FSC_SUPPORT
#define FCC_MAX_PID_NUM             48
#define FCC_MAX_AUIDO_PID_NUM       8
#define FCC_MAX_ECM_PID_NUM         3
#define FCC_MAX_EMM_PID_NUM         3
#define FCC_MAX_TTX_PID_NUM         6
#define FCC_MAX_SUBT_PID_NUM        6
#define FCC_MAX_TTX_SUBT_PID_NUM    6
#define FCC_MAX_PSI_PID_NUM         8

enum fcc_psi_pid_index
{
    FCC_PAT_IDX=0,
    FCC_PMT_IDX=1,
    FCC_SDT_IDX=2,
    FCC_EIT_IDX=3,
    FCC_CAT_IDX=4,
    FCC_NIT_IDX=5,
    FCC_TDT_IDX=6,
    FCC_BAT_IDX=7,
};

enum FCC_PID_TYPE
{
    FCC_PID_TYPE_ECM,
    FCC_PID_TYPE_EMM,
    FCC_PID_TYPE_SUBT,
    FCC_PID_TYPE_TTX,
};

enum FCC_AV_MODE
{
    FCC_AV_ENABLE,
    FCC_AUDIO_ENABLE,
    FCC_VIDEO_ENABLE,
};

struct fcc_cache_pid_info
{
    //num
    UINT8    total_num;
    UINT8    audio_count;
    UINT8    ecm_pid_count;
    UINT8    emm_pid_count;
    UINT8    ttx_pid_count;
    UINT8    subt_pid_count;
    UINT8    ttx_subt_pid_count;
    UINT8    psi_pid_count;
    //pid
    UINT16    video_pid;
    UINT16    audio_pid[FCC_MAX_AUIDO_PID_NUM];
    UINT16    audio_lang[FCC_MAX_AUIDO_PID_NUM];
    UINT16    pcr_pid;
    UINT16    ecm_pids[FCC_MAX_ECM_PID_NUM];
    UINT16    emm_pids[FCC_MAX_EMM_PID_NUM];
    UINT16    ttx_pids[FCC_MAX_TTX_PID_NUM];
    UINT16    subt_pids[FCC_MAX_SUBT_PID_NUM];
    UINT16    ttx_subt_pids[FCC_MAX_TTX_SUBT_PID_NUM];
    UINT16    psi_pids[FCC_MAX_PSI_PID_NUM];//Section.
};

struct fcc_dmx_info
{
    UINT8 dmx_cache_mode;
    UINT8 dmx_av_mode;
};
#endif

struct cc_param
{
    struct cc_es_info es;
    struct cc_device_list dev_list;
#ifdef CC_POST_CALLBACK
    void (*callback) ();
#endif
#ifdef FSC_SUPPORT
    BOOL    is_fsc_play;
    INT32   fsc_play_mode;//enum ts_route_type
    INT8    dmx_prog_id;    
    INT8    stop_mode;//0:stop AV&dmx. 1: only stop dmx.
    struct fcc_cache_pid_info fcc_pid_info;
    struct fcc_dmx_info dmx_info;
#endif
};


/*************************************************************
*   ts route management struct
*************************************************************/

enum
{
    TS_ROUTE_STATUS_PLAY         = 1<<0,
    TS_ROUTE_STATUS_RECORD     = 1<<1,
    TS_ROUTE_STATUS_TMS         = 1<<2,
    TS_ROUTE_STATUS_PLAYACK     = 1<<3,

    TS_ROUTE_STATUS_USE_CI_PATCH = 1<<7, // the route using CI patch
};

enum ts_route_type
{
    TS_ROUTE_MAIN_PLAY = 0,
    TS_ROUTE_PIP_PLAY,
    TS_ROUTE_BACKGROUND,
    TS_ROUTE_PLAYBACK,
#ifdef FSC_SUPPORT	
	TS_ROUTE_PRE_PLAY,
#endif	
};

#ifdef FSC_SUPPORT
enum fsc_state_machine
{
	FSC_MAIN_2_PRE = 0,
	FSC_PRE_2_MAIN,
};
#endif



struct dmx_sim
{
    UINT8 used;
    UINT8 sim_type;
    UINT16 sim_pid;
    UINT32 sim_id;
    UINT32 channel_id;
    UINT32 callback;
};

struct ts_route_info
{
    UINT8 enable;
    UINT32 id; //reserved
    enum ts_route_type type;
    UINT8 is_recording;
    UINT8 dmx_id; //TSI_DMX_0, 1 or 2, if dmx2 don't care frontend devices!
    UINT8 dmx_slot_id; //0 or 1
    UINT8 nim_id; // 0: tuner1; 1: tuner2
    UINT8 tsi_id; //TSI_SPI_QPSK  or TSI_SPI_TSG or TSI_SSI_0
    UINT8 ci_mode; //0: don't care or 1: serial or 2: parallel
    UINT8 ci_num;
    UINT8 cia_included;
    UINT8 cia_used;
    UINT8 cib_included;
    UINT8 cib_used;
    UINT8 ca_info_enable;
    UINT8 ca_slot_ready[2];    //num means how much figure matched between cam and pmt
    enum ts_route_type ca_info_type;
    UINT8    ca_count;
    CA_INFO ca_info[P_MAX_CA_NUM];
    UINT8 ts_id_recover; // for 2-CI control, when stop TSG playback, need recover background ts_id
    UINT8 ts_id; //TSI_TS_A or TSI_TS_B   there are some default rule between TS and CI
    UINT8 vdec_id;
    UINT8 vpo_id; //0: full screen vpo; 1: pip vpo
    UINT8 state; //1-bit to 1-state
    UINT32 tp_id;
    UINT32 prog_id;
    UINT8 stream_av_mode;// 1 TV 0 Radio
    UINT8 stream_ca_mode;
    UINT8 screen_mode; //0: background or 1: main or 2: pip
    UINT8 flag_lock;
    UINT8 flag_scramble;
    UINT8 sim_num;
    struct dmx_sim dmx_sim_info[DEV_DMX_SIM_CNT>>1];
    UINT8 sat2ip_flag;    
    UINT8 lock_chck_en;
    UINT8 bstoping;
};

struct ts_route_config
{
    UINT8 lock_chck_en;
};

struct dmx_config
{
    UINT32 tp_id;
     struct dmx_sim dmx_sim_info[DEV_DMX_SIM_CNT];
};

typedef void (*v12set_callback)(UINT32 v12);


/*************************************************************
*   function declaration
*************************************************************/

void frontend_set_unicable(struct nim_device *nim,struct ft_antenna *antenna,union ft_xpond* xpond);

void frontend_set_nim(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set);

void frontend_set_antenna(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set);

void frontend_tuning(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set);

void frontend_set_ts_route(struct nim_device *nim, struct ft_ts_route *route);

BOOL frontend_check_nim_param(struct nim_device *nim, union ft_xpond *xpond, struct ft_antenna *antenna);

UINT32 ts_route_create(struct ts_route_info *ts_route);
UINT32 ts_route_create_ext(struct ts_route_info *ts_route,struct ts_route_config *pconfig);
RET_CODE ts_route_delete(UINT32 id);
RET_CODE ts_route_get_by_id(UINT32 id, struct ts_route_info *ts_route);
RET_CODE ts_route_get_by_type(UINT32 type, UINT32 *ts_route_id, struct ts_route_info *ts_route);
RET_CODE ts_route_get_record(UINT32 prog_id, UINT32 *ts_route_id, struct ts_route_info *ts_route);
RET_CODE ts_route_update(UINT32 id, struct ts_route_info *ts_route);
INT16 ts_route_check_ci(UINT8 ci_id);
INT16 ts_route_check_ci_route(UINT8 ci_id, UINT8 *route_num, UINT16 *routes);
UINT8 ts_route_check_ts_by_dmx(UINT8 dmx_id);
void ts_route_check_dmx_route(UINT8 dmx_id, UINT8 *route_num, UINT16 *routes);
void chchg_set_local_position(double longtitude,double latitude);
void chchg_get_local_position(double *longtitude,double *latitude);

void ts_route_set_nim_tsiid(UINT8 nim_id, INT8 tsiid);
INT8 ts_route_get_nim_tsiid(UINT8 nim_id);
INT32 ts_route_get_max_num(void);
void ts_route_check_record_route_all(UINT16 *route_num);
UINT8 ts_route_get_by_sim_id(UINT32 sim_id,UINT8 is_pip,    struct ts_route_info* ts_route);
UINT8 is_ca_info_in_ts_route(UINT32 sim_id);
UINT8 ts_route_set_ci_info(UINT32 sim_id,CA_INFO *ca_info,UINT8 ca_count);
UINT32 ts_route_get_by_prog_id(UINT32 prog_id, UINT8 dmx_id,  UINT8 is_pip, struct ts_route_info *ts_route);
void ts_route_check_nim_route(UINT32 nim_id, UINT8 *route_num, UINT16 *routes);
void ts_route_check_tp_route(UINT32 tp_id, UINT8 *route_num, UINT16 *routes);
UINT8 ts_route_get_num(void);
void _ts_route_get_dmx_config(struct dmx_device *dmx, struct dmx_config *config);
void _ts_route_get_dev_state(UINT32 hld_type, void *dev, UINT32 *status);
BOOL ts_route_is_av_scrambled(struct dmx_device *dmx, UINT16 v_pid, UINT16 a_pid);
void uich_chg_set12vset_cb(v12set_callback v12setcb);


#ifdef __cplusplus
}
#endif


#endif

