/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_pub27.h
*
* Description:
*     pub middle layer implementation file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef PUB_PORTING_PUB29

#include <api/libpub/lib_pub.h>

#else

#ifndef _M3327_LIB_PUB27_H_
#define _M3327_LIB_PUB27_H_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libpub29/lib_as.h>
#include <api/libpub29/lib_mv.h>
#include <hld/nim/nim_dev.h>


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef PUB_PORTING_PUB29
extern OSAL_ID libpub27_flagid;
#endif

/*********************************/
/*from lib_mid27.h*/
#define MID27_CMD_MAIN_TYPE_MASK        0xFF00
#define MID27_CMD_SUB_TYPE_MASK        0x00FF

#define MID27_CMD_CC                0x0100
#define MID27_CMD_MV                0x0200
#define MID27_CMD_AS                0x0300
#define MID27_CMD_TS                0x0400


#define MID27_CMD_NULL                0x0000
/*******************************************
*!! NOTE: 0x03xx is reserved for channel change!!
********************************************/
#define MID27_CMD_CC_PLAY            0x0101
#define MID27_CMD_CC_RESET        0x0102
#define MID27_CMD_CC_STOP            0x0103
#define MID27_CMD_CC_PAUSE_TV        0x0104
#define MID27_CMD_CC_RESUME_TV    0x0105
#define MID27_CMD_CC_SWITCH_AID    0x0106
#define MID27_CMD_CC_SHOW_LOGO    0x0107
#define MID27_CMD_CC_RESET_QPSK    0x0108
#ifdef MUTI_TTX_SUB
#define MID27_CMD_CC_SWITCH_SID    0x0109
#define MID27_CMD_CC_SWITCH_TID    0x010a
#endif
#define MID27_CMD_CC_SWITCH_ACH        0x010b

#define MID27_CMD_CC_DOLBY_ON_OFF    0x010c
#if(SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) || (defined(PORTING_ATSC))
#define MID27_CMD_CC_AERIAL_CHG_CH    0x010d
#endif

#if( SYS_PROJECT_FE == PROJECT_FE_DVBC )
#define MID27_CMD_CC_NIM_CH_CHG        0x010d
#endif

#define MID27_CMD_CC_SET_XPOND        0x010e

#define MID27_CMD_CC_LOCK                    0x010f

//090417 add for tsg_ci patch
#define MID27_CMD_CC_START_PATCH            0x0110
#define MID27_CMD_CC_STOP_PATCH                0x0111    //shall be deleted
#define MID27_CMD_CC_START_CI                0x0111

/*******************************************
*!! NOTE: 0x03xx is reserved for multiview!!
********************************************/
#define MID27_CMD_MV_ENTER            0x0201
#define MID27_CMD_MV_DRAWONEBOX        0x0202
#define MID27_CMD_MV_EXIT                   0x0203

/*******************************************
*!! NOTE: 0x03xx is reserved for channel scan!!
********************************************/
#define MID27_CMD_AS_START            0x0301
#define MID27_CMD_AS_STOP            0x0302
#ifdef BASE_TP_HALF_SCAN_SUPPORT
#define MID27_CMD_AS_HL_START        0x0303
#endif

/*******************************************
NOTE: 0x07xx is reserved for the Media player module!!
//!! Don't Use it! please
********************************************/
#define MID27_CMD_MPPE                0x0700
#define MID27_FLG_MPPE_BIT            0x00100000


/*******************************************
//!! NOTE: 0x09xx is reserved for the PVR module!!
//!! Don't Use it! please
********************************************/
#define MID27_CMD_NV                0x0900




/* change channel related flag bit pattern */
#define MID27_FLG_CC_BIT                0x00000001
#define MID27_FLG_CC_PLAY_BIT            0x00000001
#define MID27_FLG_CC_STOP_BIT            0x00000100
#define MID27_FLG_CC_LOGO_BIT            0x00000200
#define MID27_FLG_CC_RESET_BIT            0x00000400
#define MID27_FLG_CC_RESET_QPSK_BIT    0x00000800
#define MID27_FLG_CC_PAUSE_BIT            0x00001000
#define MID27_FLG_CC_SWITCH_AID_BIT    0x00002000
#define MID27_FLG_CC_RESET_DISEQC_BIT    0x00004000
#define MID27_FLG_CC_SWITCH_ACH_BIT    0x00008000
#define MID27_FLG_CC_LOCK_BIT            0x00010000


/* multiview related flag bit pattern */
#define MID27_FLG_MV_PAGEUPDOWN         0x00000002
#define MID27_FLG_MV_ENTER_ACTIVE        0x00000004
#define MID27_FLG_MV_SWITCHPIP        0x00000008
#define MID27_FLG_MV_EXITOK              0x00000010
#define MID27_FLG_MV_ACTIVE            0x00000020
#define MID27_FLG_MV_DRAW_ONE             0x00004000  //stanley wang ,Yu chun hung 20050701
#define MID27_FLG_MV_UIENTER            0x00008000
/* auto-scan related flag bit pattern */
#define MID27_FLG_AS_FINISH        0x00000040
#define MID27_FLG_AS_STOP            0x00000080
#define MID27_FLG_AS_NVOD            0x00010000
/* epg related flag bit pattern */
#ifdef _EPG_MULTI_SERVICE
#define MID27_FLG_EPG_TP_UNIT        0x01000000
#endif

#define CMD_DATA_SIZE                60//support 25multiview, it is the max size.

struct mid27_cmd
{
    UINT16 cmd_type;
//    UINT16 para16;
    UINT32 para16;    //dual tuner: program id
    UINT32 para32;
    UINT8 data[CMD_DATA_SIZE];
};


typedef void(*pfun_mid27_cmd_handler)(struct mid27_cmd *);

BOOL mid27_cmd_handler_register(UINT16 cmd_main_type, pfun_mid27_cmd_handler cmd_handler);
BOOL mid27_overlay_cmd_register(UINT16 repeat_cmd);
BOOL mid27_init(void);
INT32 mid27_send_cmd(struct mid27_cmd* cmd,UINT32 tmout);
INT32 mid27_wait_cmd_finish(UINT32 cmd_bit,UINT32 tmout);
void mid27_declare_cmd_finish(UINT32 cmd_bit);


/*****************************************/

/*from lib_chchg.h*/
#define CC_CMD_AS_CLOSE_CH            0xFFFFFFFE
#define CC_CMD_RESET_CRNT_CH        0xFFFFFFFD
#define CC_CMD_UI_CLOSE_CH            0xFFFFFFFC
#define CC_CMD_RESET_CRNT_CH_1        0xFFFFFFFB
#define CC_CMD_PLAY_DIRECT            0xFFFFFFFA    // Added by Roman for 27c.
#define CC_CMD_RESET_DMX            0xFFFFFFF9
//090417 add for tsg_ci patch
#define CC_CMD_START_PATCH          0xFFFFFFF8
#define CC_CMD_STOP_PATCH            0xFFFFFFF7    //shall be delleted
#define CC_CMD_START_CI                0xFFFFFFF7

// Member Variable's Order of Two Struct Below is Disordered for Byte-Align
struct xpond_info
{
    UINT32      lnb_low;
    UINT32      lnb_high;
    UINT16    position;        /* satellite position value */
    UINT16    satellite_id;        /* satellite id */
    UINT16    freq;            /* Down-link frequency */
        UINT16    baud;            /* Symbol rate */
        UINT8    pol;                /* Polarity */
        UINT8    s22k;            /* 22KHz switch */
        UINT8    diseqc_type;    /* DisEqc Type */
        UINT8    diseqc;            /* DisEqC switch */
        UINT8    s12v;            /* 12V switch */
    UINT8    positioner;        /* satellite position flag */
    UINT8       lnb_type;
};

// for new cc , create following structures
struct cc_antenna_info
{
    UINT32      longitude;
    UINT32     lnb_high;
        UINT32     lnb_low;
    UINT16      unicable_freq;      /* centre frequency of user band:1210,1420,... */
        UINT8    positioner_type;    /* 0:no positioner, 1:1.2 positioner, 2:1.3 USALS*/
        UINT8     position;

        UINT8    di_seq_c_type;    /* 1/4,1/8,1/16 */
        UINT8     di_seq_c_port;    /* 4;    */
        UINT8     di_seq_c11_type;  /* diseqc1.1 type1: 2 or 4 port; type2: 8 or 16 port */
        UINT8     di_seq_c11_port;
        UINT8    k22;
       UINT8     v12;
        UINT8     pol;                /* 0: auto,1: H,2: V,3: off*/
        UINT8     toneburst;        /* 0: off, 1: A, 2: B */

/* 0: not DiSEqC LNB, 1:Universal Local freq DiSEqC LNB, 2:DiSEqC LNB Hi local freq,    3:DiSEqC LNB lo local freq*/
        UINT8     lnb_type;

        UINT8    lnb_id;
        /*add for Unicable LNB*/
        UINT8       unicable_pos;       /* position:A/B(value:0/1) */
        UINT8       unicable_ub;        /* user band(slot) or SCR channel:1~8(value:0~7) */
};

struct cc_xpond_info
{
    UINT32 frq;
    UINT32 sym;
    UINT8  pol;
#if     (SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
    UINT8     guard_interval;    /*0x4: 1/4, 0x8: 1/8, 0x10: 1/16, 0x20:1/32*/
    UINT8     FFT;            /*0x2 : 2k, 0x8 : 8k*/
    UINT8     modulation;        /*0x4 : QPSK, 0x10 : 16 QAM, 0x40 : 64 QAM*/
    UINT8    bandwidth;        /**/
    UINT8    inverse;            /*used in nim for channel change*/
#ifdef SMART_ANT_UI
    UINT8    smant_enable;
    UINT8    smant_dir;
    UINT8    smant_pol;
    UINT8    smant_gain;
    UINT8    smant_channel;
#endif
#endif
};

struct xpond_antenna_info
{
    struct cc_antenna_info antenna;
    struct cc_xpond_info xpond;
};

BOOL uich_chg_play_prog(UINT8 avflag,UINT32 index);
BOOL uich_chg_pause_prog(void);//just pause video, audio not include.
BOOL uich_chg_resume_prog(void);//rusume paused-vide to play
BOOL uich_chg_stop_prog(BOOL is_need_blc_scrn);//stop current program's video,audio and other stream play.
BOOL uich_chg_switch_ach(UINT8 u_audio_channel);//switch current program's audio channel
BOOL uich_chg_switch_apid(UINT32 u_audio_pid);//switch current program's audio pid
BOOL uich_chg_show_logo(UINT8* p_logo_data,UINT32 u_data_size,BOOL is_sync);

#ifdef SYS_12V_SWITCH
#if(SYS_12V_SWITCH == SYS_FUNC_ON)
typedef void (*v12set_callback)(UINT32 v12);
void uich_chg_set12vset_cb(v12set_callback v12setcb);
#endif
#endif

void set_antenna(struct cc_antenna_info* p_antenna);
void set_xpond(struct cc_xpond_info* p_xponder);
void cc_init(void);



/*****************************************/
/*****************************************/

#define AS_ACTIVE_FLAG             0x0000001
#define CC_ACTIVE_FLAG            0x0000002
#define DM_ACTIVE_FLAG             0x0000004
#define CC_NEW_CHCHG_FLAG    0x0000008
#define DM_BLOCKCHECK_FLAG     0x0000010
#define CC_SHOW_LOGO            0x0000020
#define DM_CHECK_NIM_FLAG         0x0000040

struct pub27_config
{
    BOOL dm_enable;            //dec monitor
    BOOL chchg_blk_enable;    //show black screen when channel change: TRUE - blackscreen FALSE - freeze current video

    UINT8 patch_enable: 1;        //DMX1 + TSG + CI patch
    UINT8 ci_enable: 1;            //operate CI slot while CC
    UINT8 free_bypass_cam: 1;    //when ci_enable = 1, free program bypass CI CAM
    UINT8 reserved1: 5;
    UINT8 reserved2[3];

    UINT8 reserved[256-12];    //total 256 byte
};

/* Interface for UI*/
RET_CODE libpub27_init();
RET_CODE libpub27_init_ext(struct pub27_config *config);


typedef void (*cc_monitor_prog)();
struct monitor_param
{
    UINT8 port_index;
    cc_monitor_prog event_hand;
};

 enum DEC_STATE
{
     NOT_LOCK = 0,
     AV_DECODED,
     AUDIO_DECODED,
     VIDEO_DECODED,
     AV_DEC_FAIL
 };


 /*BIT 0*/
 #define DM_OFF    0
 #define DM_ON    1
 /*BIT 1~2*/
 #define DM_NIMCHECK_OFF     4
 #define DM_NIMCHECK_ON    6
 #define dm_set_onoff(a) do{PRINTF("DM (%d):________ <%s>\n", a, __FUNCTION__);monitor_switch(a);}while(0)

 /*If Monitor is in wrong status the functions will return FALSE, and target operation is failed.*/
 BOOL monitor_init(void);
 BOOL monitor_proc(void);
 BOOL monitor_rst(void);
 BOOL monitor_switch(INT32 status);
 BOOL monitor_uninit(void);
 /* switch nim device(for dual tuner)*/
 INT32 monitor_set_nim(struct nim_device * nim);


#ifdef __cplusplus
}
#endif


#endif

#endif
