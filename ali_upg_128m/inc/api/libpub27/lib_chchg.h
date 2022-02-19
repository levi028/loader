/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
 *
 *  File: lib_chchg.h
 *
 *  Description: Channel change routine header file
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.5.10   Liu Lan     0.1.000  Initial
 *    2.    2004.10.9    Tom Gao        0.2.000     move it from M3326 to M3327 prj
 *
 ****************************************************************************/
#ifndef __LIB_PUB27_CC_H__
#define __LIB_PUB27_CC_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define CC_CMD_AS_CLOSE_CH                0xFFFE
#define CC_CMD_RESET_CRNT_CH                0xFFFD
#define CC_CMD_UI_CLOSE_CH                0xFFFC
#define CC_CMD_RESET_CRNT_CH_1                0xFFFB
#define CC_CMD_PLAY_DIRECT                0xFFFA    // Added by Roman for 27c.
#ifdef _MHEG5_SUPPORT_
#define CC_CMD_RESET_MHEG5_APP        0xFFF9
#endif

#define MW_MULTIVIEW_ACTIVE                1
#define MW_MULTIVIEW_PAGEUPDOWN         2
#define MW_MULTIVIEW_ENTER_ACTIVE        4
#define MW_MULTIVIEW_SWITCHPIP            8
#define MW_MULTIVIEW_EXITOK                  16


//extern OSAL_ID l_mv_status_flg;


BOOL uich_chg_task_init(void);
BOOL uich_chg_play_prog(UINT8 avflag,UINT32 index);
BOOL uich_chg_pause_prog(void);//just pause video, audio not include.
BOOL uich_chg_resume_prog(void);//rusume paused-vide to play
BOOL uich_chg_stop_prog(BOOL is_need_blc_scrn);//stop current program's video,audio and other stream play.

BOOL uich_chg_switch_ach(UINT8 u_audio_channel);//switch current program's audio channel
BOOL uich_chg_switch_apid(UINT32 u_audio_pid);//switch current program's audio pid
BOOL uich_chg_show_logo(UINT8* p_logo_data,UINT32 u_data_size,BOOL is_sync);
BOOL uich_chg_audio_key(UINT32 aud_src_id);
#ifdef _MHEG5_SUPPORT_
BOOL uich_chg_reset_mheg5(UINT16 u_channel_index, void* callback);
typedef void (*cc_callback)(UINT32);

#endif
struct xpond_info
{
    UINT16    freq;            /* Down-link frequency */
    UINT16    baud;            /* Symbol rate */
    UINT8    pol;            /* Polarity */
    UINT8    s22k;            /* 22KHz switch */
    UINT8    diseqc_type;    /* DisEqc Type */
    UINT8    diseqc;            /* DisEqC switch */
    UINT8    s12v;            /* 12V switch */
    UINT8    positioner;        /* satellite position flag */
    UINT16    position;        /* satellite position value */
    UINT16    satellite_id;    /* satellite id */
    UINT8   lnb_type;
    UINT32  lnb_low;
    UINT32  lnb_high;
};

BOOL drv_dem_get_xpond(INT16 index, struct xpond_info *xpond);
BOOL drv_dem_set_xpond(struct xpond_info *xpond);

// for new cc , create following structures
struct cc_antenna_info
{
    UINT8    positioner_type;    /*0:no positioner, 1:1.2 positioner, 2:1.3 USALS*/
    UINT8     position;
    UINT32  longitude;

    UINT8    di_seq_c_type;    /* 1/4,1/8,1/16 */
    UINT8      di_seq_c_port;    /* 4;    */
    UINT8      di_seq_c11_type;  //diseqc1.1 type1: 2 or 4 port; type2: 8 or 16 port
    UINT8      di_seq_c11_port;
    UINT8    k22;
    UINT8   v12;
    UINT8      pol;            /* 0: auto,1: H,2: V,3: off*/
    UINT8      toneburst;        /* 0: off, 1: A, 2: B */

    /*0: not DiSEqC LNB, 1:Universal Local freq DiSEqC LNB, 2:DiSEqC LNB Hi local freq,    3:DiSEqC LNB lo local freq*/
    UINT8      lnb_type;
    UINT32    lnb_high;
    UINT32    lnb_low;
};

struct cc_xpond_info
{
    UINT32 frq;
    UINT32 sym;
    UINT8  pol;
    //SN for DVB-T
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
    UINT8     guard_interval;    /*0x4: 1/4, 0x8: 1/8, 0x10: 1/16, 0x20:1/32*/
    UINT8     FFT;            /*0x2 : 2k, 0x8 : 8k*/
    UINT8     modulation;        /*0x4 : QPSK, 0x10 : 16 QAM, 0x40 : 64 QAM*/
    UINT8    bandwidth;        /**/
    UINT8    inverse;            /*used in nim for channel change*///SN, 20050524

    UINT8    priority;//060330 Sam/SN
#ifdef SMART_ANT_SUPPORT
    UINT8    smant_enable;
    UINT8    smant_dir;
    UINT8    smant_pol;
    UINT8    smant_gain;
    UINT8    smant_channel;
#endif
    INT8 offset_step;
#endif
};

/*void set_antenna(struct cc_antenna_info* pAntenna);
void set_xpond(struct cc_xpond_info* pXponder);
void reset_antenna(void);
*/

#ifdef SIGNAL_DETECT_ENABLE
void disable_dtect_signal();
void enable_dtect_signal();
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //__LIB_PUB27_CC_H__

