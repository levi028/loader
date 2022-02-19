/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_pub.h
*
* Description:
*     pub middle layer implementation file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIB_PUB_H_
#define _LIB_PUB_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <api/libpub/lib_as.h>
#include <api/libpub/lib_mv.h>
#include <hld/nim/nim_dev.h>
#include <api/libpub/lib_frontend.h>



 /*BIT 0*/
 #define DM_OFF    0
 #define DM_ON    1
 /*BIT 1~2*/
 #define DM_NIMCHECK_OFF     4
 #define DM_NIMCHECK_ON    6

 #define dm_set_onoff(a) do{PRINTF("DM (%d):________ <%s>\n", a, __FUNCTION__);monitor_switch(a);}while(0)

//#define CC_PRINT_LINE(fmt, arg...) libc_printf(fmt, ##arg)
#if 1
#define PUB_PRINT_LINE(...)
#else
#define PUB_PRINT_LINE(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#endif

#define CC_CMD_RESET_CRNT_CH        0xFFFFFFFD

/*******************************************
libpub cmd bit pattern
********************************************/
#define LIBPUB_CMD_MAIN_TYPE_MASK    0xFF00
#define LIBPUB_CMD_SUB_TYPE_MASK        0x00FF

#define LIBPUB_CMD_NULL            0x0000
#define LIBPUB_CMD_CC                0x0100
#define LIBPUB_CMD_MV                0x0200
#define LIBPUB_CMD_AS                0x0300


/****!! NOTE: 0x01xx is reserved for channel change!!***/
#define LIBPUB_CMD_CC_PLAY        0x0101
#define LIBPUB_CMD_CC_RESET        0x0102
#define LIBPUB_CMD_CC_STOP        0x0103
#define LIBPUB_CMD_CC_PAUSE_VIDEO    0x0104
#define LIBPUB_CMD_CC_RESUME_VIDEO    0x0105
#define LIBPUB_CMD_CC_SWITCH_APID       0x0106
#define LIBPUB_CMD_CC_SWITCH_ACH        0x0107
#define LIBPUB_CMD_CC_SHOW_LOGO         0x0108
#define LIBPUB_CMD_CC_SET_FRONTEND      0x0109
//ci patch cmd
#define LIBPUB_CMD_CC_START_PATCH      0x010a
#define LIBPUB_CMD_CC_AERIAL_CHG_CH      0x010b
#if (defined(PUB_PORTING_PUB29)) && ((SYS_PROJECT_FE == PROJECT_FE_ATSC) \
    || (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))
#define LIBPUB_CMD_CC_LOCK                0x010c
#endif
#ifdef FSC_SUPPORT
#define LIBPUB_CMD_CC_FSC_MAIN_PLAY         0x010d
#define LIBPUB_CMD_CC_FSC_PRE_PLAY          0x010e
#define LIBPUB_CMD_CC_FSC_CHANNEL_CHANGE       0x010f
#define LIBPUB_CMD_CC_FSC_STOP              0x0110
#endif

/****!! NOTE: 0x02xx is reserved for multiview!!*****/
#define LIBPUB_CMD_MV_ENTER                 0x0201
#define LIBPUB_CMD_MV_DRAWONEBOX    0x0202
#define LIBPUB_CMD_MV_EXIT                  0x0203


/****!! NOTE: 0x03xx is reserved for channel scan!!*****/
#define LIBPUB_CMD_AS_START        0x0301
#define LIBPUB_CMD_AS_STOP            0x0302


/****!! NOTE: 0x07xx is reserved for media player!!*****/
#define LIBPUB_CMD_MPPE                0x0700
#define LIBPUB_FLG_MPPE_BIT            0x00100000


/****!! NOTE: 0x09xx is reserved for pvr!!*****/
#define LIBPUB_CMD_NV                0x0900


/****!!channel change cmd bit!!****/
#define CC_CMD_NULL                    0x00000000
#define CC_CMD_CLOSE_VPO            0x00000001
#define CC_CMD_STOP_AUDIO            0x00000002
#define CC_CMD_STOP_VIDEO            0x00000004
#define CC_CMD_STOP_DMX                0x00000008
#define CC_CMD_FILL_FB                0x00000010
#define CC_CMD_START_AUDIO            0x00000020
#define CC_CMD_START_VIDEO            0x00000040
#define CC_CMD_START_DMX            0x00000080
#define CC_CMD_DMX_CHG_APID            0x00000100
#define CC_CMD_AUDIO_SWITCH_CHAN    0x00000200
#define CC_CMD_SET_FRONTEND            0x00000400
#define CC_CMD_PAUSE_VIDEO            0x00000800
#define CC_CMD_RESUME_VIDEO            0x00001000
#define CC_CMD_SET_ANTENNA            0x00002000
#define CC_CMD_SET_XPOND            0x00004000
#define CC_CMD_START_SI_MONITOR        0x00008000
#define CC_CMD_INFO_VPO_CHANCHNG    0x00100000
#define CC_CMD_STOP_MCAS_SERVICE    0x00200000
#define CC_CMD_DMX_DISABLE            0x00400000

#ifdef FSC_SUPPORT
#define CC_CMD_FSC_MAIN_PLAY			0x80000000
#define CC_CMD_FSC_PRE_PLAY             0x40000000
#define CC_CMD_FSC_STOP                 0x20000000

typedef BOOL (*SEND_MSG)(UINT32 param1, UINT32 param2);
void chchg_fsc_set_ap_callback(SEND_MSG p_call_back);
#endif


/*******************************************
libpub flag bit patterns
********************************************/
/* change channel related flag bit pattern */
#define LIBPUB_FLG_CC_PLAY_BIT            0x00000001
#define LIBPUB_FLG_CC_STOP_BIT            0x00000002
#define LIBPUB_FLG_CC_PAUSE_BIT        0x00000004
#define LIBPUB_FLG_CC_SWITCH_APID_BIT    0x00000008
#define LIBPUB_FLG_CC_SWITCH_ACH_BIT    0x00000010
#define LIBPUB_FLG_CC_LOGO_BIT            0x00000020
//set front end
#define LIBPUB_FLG_CC_SETFT_BIT        0x00000040
//ci patch
#define LIBPUB_FLG_CC_PATCH_BIT        0x00000080
#if (defined(PUB_PORTING_PUB29)) && ((SYS_PROJECT_FE == PROJECT_FE_ATSC)  \
    || (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))
  #define MID27_FLG_CC_LOCK_BIT        0x00000100
#endif


/* multiview related flag bit pattern */
//recieve mv_enter cmd
#define LIBPUB_FLG_MV_ENTER    0x00000800
#define LIBPUB_FLG_MV_BUSY     0x00001000
#define LIBPUB_FLG_MV_CANCEL    0x00002000
#define LIBPUB_FLG_MV_ACTIVE    0x00004000
#define LIBPUB_FLG_MV_EXIT        0x00008000


/*monitor flag bit*/
#define LIBPUB_MONITOR_MUTEX            0x00010000
#define LIBPUB_MONITOR_CHECK_VDEC   0x00020000
#define LIBPUB_MONITOR_CHECK_NIM    0x00040000
#define LIBPUB_MONITOR_INVALID_NUM  0xFFFFFFFF

/*monitor device msg*/
#define LIB_MONITOR_NIM_UNLOCK      1
#define LIB_MONITOR_VDEC_HOLD       2

#define LIBPUB_CMD_DATA_SIZE    80
#ifdef FSC_SUPPORT
#define LIBPUB_CMD_DATA2_SIZE   200
#else
#define LIBPUB_CMD_DATA2_SIZE   LIBPUB_CMD_DATA_SIZE
#endif

struct libpub_cmd
{
    UINT16 cmd_type;
    UINT8 sync;
    UINT8 reserve;
    UINT32 cmd_bit;
    UINT32 para1;
    UINT32 para2;
    UINT8 data1[LIBPUB_CMD_DATA_SIZE];
    UINT8 data2[LIBPUB_CMD_DATA2_SIZE];
};

/*monitor callback to send device status msg. msg-message type;param-device handle*/
typedef void (*libpub_monitor_callback)(UINT32 msg, UINT32 param);

struct pub_module_config
{
    BOOL dm_enable;
    UINT32 monitor_interval;//unit ms
    libpub_monitor_callback function;
};

/////////////////////////////porting api////////////////////////////////////////////////
#ifdef PUB_PORTING_PUB29


// Member Variable's Order of Two Struct Below is Disordered for Byte-Align
struct cc_xpond_info
{
    UINT32 frq;
    UINT32 sym;
    UINT8  pol;
#if     (SYS_PROJECT_FE == PROJECT_FE_DVBT)
        || (SYS_PROJECT_FE == PROJECT_FE_ATSC)
        || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
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

typedef void (*cc_monitor_prog)();
struct monitor_param
{
    UINT8 port_index;
    cc_monitor_prog event_hand;
};

#endif



#ifdef __cplusplus
extern "C"
{
#endif

typedef void(*libpub_cmd_handler)(struct libpub_cmd *);

BOOL libpub_cmd_handler_register(UINT16 cmd_main_type, libpub_cmd_handler handler);
BOOL libpub_overlay_cmd_register(UINT16 repeat_cmd);
RET_CODE libpub_init(struct pub_module_config *config);
INT32 libpub_send_cmd(struct libpub_cmd *cmd,UINT32 tmout);
INT32 libpub_wait_cmd_finish(UINT32 cmd_bit,UINT32 tmout);
void libpub_declare_cmd_finish(UINT32 cmd_bit);


/*****************************************/
/**
 * Description:
 *      This function is used to do channel change init.
 * name: chchg_init
 * Parameter: NULL
 * Return Value: NONE
 *
*/
void chchg_init(void);
/**
 * Description:
 *      This function is used to play a channel service.
 * name: chchg_play_channel
 * Parameter: 
 *      ft   : Pointer to structure ft_frontend  includes parameters of front end config(antenna+tp)
                and TS route, if not need configure antenna and TS route, 
                corresponding field should be set 0.
 *  param: Pointer to structure cc_param includes two parts: 
 *           ES level PID&info of the service and related driver device handles.
 *  blk_scrn: show black screen or not
 * Return Value:
 *      RET_FAILURE: function error
 *      RET_SUCCESS: function successfully
 *
*/
INT32 chchg_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn);

/**
 * Description:
 *      This function is used to stop current playing channel service.
 *      name: chchg_stop_channel
 * Parameter: 
 *      es   :  Pointer to structure cc_es_info includes ES level PID&info of the service
 *      dev_list: Pointer to structure cc_device_list includes related driver device handles
 *      blk_scrn: show black screen or not
 * Return Value:
 *      RET_FAILURE: Fail
 *      RET_SUCCESS: success
 *
*/
INT32 chchg_stop_channel(struct cc_es_info *es, struct cc_device_list *dev_list, BOOL blk_scrn);

/**
 * Description:
 *       This function is used to pause current playing video.
 * name: chchg_pause_video
 * Parameter: 
 *      vdec   :  Video decoder device handle
 *
 * Return Value:
 *      RET_FAILURE: Fail
 *    RET_SUCCESS: success
 *
*/
INT32 chchg_pause_video(struct vdec_device *vdec);

/**
 * Description:
 *       This function is used to resume current paused video.
 * name: chchg_resume_video
 * Parameter: 
 *      vdec   :  the vdec that current used.
 *
 * Return Value:
 *    RET_FAILURE: Fail
 *    RET_SUCCESS: success
 *
*/
INT32 chchg_resume_video(struct vdec_device *vdec);

/**
 * Description:
 *      This function is used to change current playing service's audio PID.
 * name:
 *      chchg_switch_audio_pid
 * Parameter: 
 *      es   :  Pointer to structure cc_es_info, Caller cant give only the audio PID,
 *               other field can be 0
 *      dev_list: Pointer to structure cc_device_list includes related driver device handles
 * Return Value:
 *      RET_FAILURE: Fail
 *      RET_SUCCESS: success
 *
*/
INT32 chchg_switch_audio_pid(struct cc_es_info *es, struct cc_device_list *dev_list);

/**
 * Description:
 *      This function is used to change current playing service's audio channel.
 * name: chchg_switch_audio_chan
 * Parameter: 
 *      snd   : Sound device handle 
 *      audio_chan: Audio channel AUDIO_CH_L, AUDIO_CH_R, AUDIO_CH_MONO or AUDIO_CH_STEREO
 *
 * Return Value:
 *    RET_FAILURE: Fail
 *    RET_SUCCESS: Success
*/
INT32 chchg_switch_audio_chan(struct snd_device *snd, UINT8 audio_chan);

/**
 * Description:
 *       This function is used to show logo which stored in storage device such as flash.
 * name: chchg_switch_audio_pid
 * Parameter: 
 *    plogo   : Pointer to structure cc_lgo_info
 *    dev_list: related driver device handles, show logo need vdec, deca and dmx handles,
 *              other field can be NULL
 *
 * Return Value:
 *    RET_FAILURE: Fail
 *    RET_SUCCESS: Success
*/
INT32 chchg_show_logo(struct cc_logo_info *plogo, struct cc_device_list *dev_list);

//start ci patch
/**
 * Description:
 *       This function is used to start the DMX->PVR->TSG->CI patch.
 * Name: chchg_start_patch
 * Parameter: 
 *      slot_mask   : Mask of slots selected.
 *
 * Return Value:
 *      RET_FAILURE: Fail
 *    RET_SUCCESS: Success
*/
INT32 chchg_start_patch(UINT8 slot_mask);


/**
 * Description:
 *       This function is used to clear pu message queue.
 * Name: libpub_clear_message_queue
 * Parameter: 
 *      NONE
 *
 * Return Value:
 *    RET_FAILURE: Fail
 *    RET_SUCCESS: Success
*/
INT32  libpub_clear_message_queue(void);


/*************************************************************/


/**
 * Description:
 *       This function is used to init monitor.monitor nim device status & vdec status.
 * Name: monitor_init
 * Parameter: 
 *    interval: monitor interval, unit:ms.
 *    func    : app registerd to pub when do pub_init.
 * Return Value:
 *      NONE
*/
void monitor_init(UINT32 interval, libpub_monitor_callback func);

/**
 * Description:
 *       This function is used to check nim device status & vdec status.
 * Name: monitor_proc
 * Parameter: 
 *      NONE.
 * Return Value:
 *      NONE
*/
void monitor_proc(void);

/**
 * Description:
 *       This function is used to reset the monitor proc.
 * Name: monitor_proc
 * Parameter: 
 *      NONE.
 * Return Value:
 *      NONE
*/
void monitor_rst(void);

/**
 * Description:
 *       This function is used to switch the monitor proc on/off.
 * Name: monitor_switch
 * Parameter: 
 *      NONE.
 * Return Value:
 *      NONE
*/
void monitor_switch(UINT32 onoff);

/**
 * Description:
 *       This function is used to open or close LIBPUB monitor 
 *      which periodically check NIM and video decoders' status.
 * Name: monitor_switch
 * Parameter: 
 *      device_flag: nim/vdec.
 *      onoff      : on/off.
 * Return Value:
 *      NONE
*/
void monitor_switch_dev_monitor(UINT32 device_flag, UINT8 onoff);

#ifdef FSC_SUPPORT
INT32 chchg_fsc_channel_change(struct ft_frontend *ft, struct cc_param *param,enum fsc_state_machine state, BOOL blk_scrn);
INT32 chchg_fsc_channel_stop(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn);
INT32 chchg_fsc_main_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn);
INT32 chchg_fsc_pre_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn);
INT32 fcc_set_dmx_cache_mode(struct dmx_device *dev_dmx, UINT8 cache_mode,struct fcc_cache_pid_info *fcc_pid_info);
#endif


 #ifdef __cplusplus
 }
 #endif


#endif


