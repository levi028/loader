#ifndef _ADVANCED_CFG_H_
#define _ADVANCED_CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../head_file.h"
#include <api/libwfd/nmp_hdcp2.h>
//#include "../../copper_common/com_api.h"
#include <hld/smc/smc.h>
#include <hld/ge/ge.h>

typedef void (*smc_class_select)(enum class_selection sel);

typedef struct
{
    UINT8 front_end_num;

    //for hdmi
    BOOL  hdcp_disable;
    UINT32 i2c_for_eddc;
     UINT8 hdmi_cm_scl;
    UINT8 hdmi_cm_sda;

    //for snd, daca
    BOOL audio_description_support;  //AUDIO_DESCRIPTION_SUPPORT
    UINT8 sys_mute_gpio;
    UINT8 ext_mute_mode;
    UINT8 sys_mute_polar;
    UINT8 snd_output_chip_type;

    //for osd
    BOOL hd_subtitle_support;        //HD_SUBTITLE_SUPPORT

    //for vpo
    BOOL tve_full_current_mode;     //TVE_USE_FULL_CURRENT_MODE
    VP_SRCASPRATIO_CHANGE   scart_aspect_change;
    T_TVE_ADJUST_TABLE *tve_tbl;
#if defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    struct tve_adjust *tve_adjust;
    struct tve_adjust *tve_adjust_adv;
#endif

    //for ttx
    BOOL ttx_packet_26_support;    //SUPPORT_PACKET_26
    BOOL ttx_sub_page;             //TTX_SUB_PAGE

    //for smc
    smc_class_select   class_select;
    UINT32 cas_type;               //CAS_TYPE
    BOOL cas9_support;             //SUPPORT_CAS9
    UINT8 invert_detect;
    //for vdec
    BOOL afd_scale_support;         //SUPPORT_AFD_SCALE

    //for avc config
    BOOL h264_support_mulit_bank;    // H264_SUPPORT_MULTI_BANK
    BOOL chanchg_video_type_support; // CHANCHG_VIDEOTYPE_SUPPORT
    BOOL avc_unify_mem_support;     // AVC_SUPPORT_UNIFY_MEM

    //
    BOOL ce_api_enabled;

    BOOL vpo_active;
    UINT8 sys_mute_gpio_clock;
    UINT8 sys_mute_gpio_data;
}advance_cfg_para;

void board_advance_init(advance_cfg_para* cfg_para);
void aui_board_advance_init(advance_cfg_para* cfg_para);

advance_cfg_para *get_board_advance_cfg(void);
void ge_dev_attach(void);
void vpo_dev_attach(void);
void dmx_dev_attach(void);
void get_hdmi_config(struct config_hdmi_parm *hdmi_param);
void get_hdmi20_config(tReceiverKeySets *h2_KeySets);
void vdec_dev_attach(void);
void osd_dev_api_attach(struct ge_device *ge_dev, UINT32 osd_layer_id);

#ifdef __cplusplus
}
#endif
#endif

