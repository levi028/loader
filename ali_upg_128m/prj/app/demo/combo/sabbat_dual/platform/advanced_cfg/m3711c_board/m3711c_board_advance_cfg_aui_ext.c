/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: m3503d_board_advance_cfg.c
*
*    Description: Advance config of M3503d board
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "../advanced_cfg.h"
#include <bus/otp/otp.h>
#include <hld/vbi/vbi.h>
#include <hld/decv/decv_hevc.h>
#include <bus/tsg/tsg.h>

#ifdef SUPPORT_AVS_DECODER
#include <hld/decv/decv_avs.h>
#endif

#include <hld/sdec/sdec.h>
#include <api/libclosecaption/lib_closecaption.h>
#include <hld/smc/smc.h>
#include <hld/cf/cf.h>
#include "../../../platform/board.h"

#ifdef _AUI_
#include "aui_common.h"
#include "aui_deca.h"
#include "aui_snd.h"
#include "aui_dsc.h"
#include "aui_tsg.h"
#include "aui_dmx.h"
#include "aui_os.h"
#include "aui_nim.h"
#include "aui_hdmi.h"
#include "aui_smc.h"
#include "aui_dis.h"
#include "aui_osd.h"
#include "aui_dog.h"
#include "aui_rtc.h"
#include "aui_decv.h"
#include "aui_mp.h"
#include "aui_music.h"
#include "aui_image.h"

#ifndef _BUILD_OTA_E_
#include "aui_vbi.h"
#endif

#define AUI_OTP_DEV_CNT_MAX			(1)
#define AUI_DECA_DEV_CNT_MAX			(1)
#define AUI_SND_DEV_CNT_MAX				(1)




/** Deca dev attribute */
typedef struct aui_stDecaAttachAttr
{
	/** device index*/
	unsigned long ulDevIdx;
	/** AUDIO_DESCRIPTION_SUPPORT */
	unsigned char bSupportAD;
	/** support dynamic sample rate */
	unsigned char ucDynaSample;
	/** the deca attach reg function name */
	void (*deca_attach)(struct deca_feature_config* config);
	/** the deca external attach reg function name */
	void (*deca_ext_dec_enable)(struct deca_device*dev, struct deca_feature_config * config);
	/** the deca dvr attach reg function name */
	void (*deca_dvr_enable)(struct deca_device*dev);
}aui_DecaAttachAttr,*aui_pDecaAttachAttr;

/** Deca module attribute */
typedef struct aui_stDecaModuleAttr
{
	/** Whether this module have been initialized,0:not init*/
	unsigned long ul_init_flag;
	/** Device counter*/
	unsigned char ucDevCnt;
	/** Device attach attribution */
	aui_DecaAttachAttr DecaAttachAttr[AUI_DECA_DEV_CNT_MAX];
}aui_DecaModuleAttr,*aui_pDecaModuleAttr;

/** snd dev attach attribute */
typedef struct aui_stSndAttachAttr
{
	/** mute circuit gpio number */
	unsigned char mute_num;
	/** the polarity which will cause circuit mute */
	unsigned char mute_polar;
	/** 24bit or 16bit  */
	unsigned char dac_precision;
	/** CODEC_I2S (0x0<<1), CODEC_LEFT (0x1<<1), CODEC_RIGHT (0x2<<1) */
	unsigned char dac_format;
	/** for M3329 serial, always should be 1. 0: means embedded dac */
	unsigned char is_ext_dac;
	/** FALSE: no mute circuit; TRUE: exists mute circuit controlled by GPIO */
	unsigned short gpio_mute_circuit:1;
	/**the polarity which will cause circuit mute */
	unsigned short ext_mute_mode:2;
	/** FALSE: do not enable M3202 audio HW accelerator;TRUE: Enable M3202 audio HW accelerator; */
	unsigned short enable_hw_accelerator:1;
	/** 1:QFP.0:BGA. */
    unsigned char chip_type_config:1;

	unsigned char support_spdif_mute;

    unsigned char support_desc;

	/** the snd attach reg function name */
	void (*snd_attach)(struct snd_feature_config * config);

}aui_SndAttachAttr,*aui_pSndAttachAttr;

/** Snd module attribute */
typedef struct aui_stSndModuleAttr
{
	/** Whether this module have been initialized,0:not init*/
	unsigned long ul_init_flag;
	/** Device counter */
	unsigned char ucDevCnt;
	/** Device attach attribution */
	aui_SndAttachAttr SndAttachAttr[AUI_SND_DEV_CNT_MAX];
}aui_SndModuleAttr,*aui_pSndModuleAttr;







typedef struct aui_stAttrOtp
{

	unsigned char ucDevIdx;

	OTP_CONFIG * OtpInitCfg;

}aui_AttrOtp,*aui_pAttrOtp;

typedef struct aui_stOtpModuleAttr
{
	/** Whether this module have been initialized,0:not init*/
	unsigned long ul_init_flag;
	unsigned char ucDevCnt;
	aui_AttrOtp AttrOtp[AUI_OTP_DEV_CNT_MAX];
}aui_OtpModuleAttr,*aui_pOtpModuleAttr;

/** Dsc module attribute */
typedef struct aui_stDscModuleAttr
{
	/** Whether this module have been initialized,0:not init*/
	unsigned long ul_init_flag;
	/** Device counter */
	unsigned char uc_dev_cnt;
}aui_DscModuleAttr,*aui_pDscModuleAttr;

extern void deca_m36_attach(struct deca_feature_config* config);
extern void deca_m36_ext_dec_enable(struct deca_device*dev, struct deca_feature_config * config);
extern void deca_m36_dvr_enable(struct deca_device*dev);
extern void snd_m36g_attach(struct snd_feature_config  *config);
extern INT32 hdmi_tx_attach(struct config_hdmi_parm *hdmi_param);
extern void smc_attach(void);

extern void deca_m36_init_tone_voice(struct deca_device *dev);
extern void snd_m36g_init_tone_voice(struct snd_device *dev);

static aui_DscModuleAttr s_DscModuleAttr;

static aui_OtpModuleAttr s_OtpModuleAttr;



static aui_DecaModuleAttr s_DecaModuleAttr;
static aui_SndModuleAttr s_SndModuleAttr;
//static aui_DmxModuleAttr s_DmxModuleAttr;


//extern aui_OSInfo *g_pOSInfo;

static void auiDevAttachDeca(aui_DecaAttachAttr DecaAttachAttr)
{
#ifndef _BUILD_OTA_E_
    struct deca_feature_config deca_config;
	MEMSET(&deca_config, 0, sizeof(struct deca_feature_config));
	deca_config.support_desc = DecaAttachAttr.bSupportAD;
	/* support dynamic sample rate change by default*/
	deca_config.detect_sprt_change = DecaAttachAttr.ucDynaSample;

#ifdef _DD_SUPPORT
	
	deca_config.priv_dec_addr = __MM_PRIV_DEC_ADDR;
	deca_config.priv_dec_size = __MM_PRIV_DEC_LEN;

#endif

#ifdef AD_SEE_OSD2_SHARE_BUFFER

        // AUI project default enable this macro, to use preset memory
        deca_config.ad_static_mem_flag = 1;//ad use the static memory
        deca_config.ad_static_mem_addr = __MM_AD_DECA_MEM_ADDR;
        deca_config.ad_static_mem_size = __MM_AD_DECA_MEM_LEN;
#endif

	(DecaAttachAttr.deca_attach)(&deca_config);
	(DecaAttachAttr.deca_ext_dec_enable)((struct deca_device * )dev_get_by_id(HLD_DEV_TYPE_DECA, DecaAttachAttr.ulDevIdx), &deca_config);
#ifdef DVR_PVR_SUPPORT
    (DecaAttachAttr.deca_dvr_enable)((struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, DecaAttachAttr.ulDevIdx));
#endif
    deca_m36_init_tone_voice((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
#endif
}

/**
*    @brief 		Initialize the deca module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		pDecaModuleAttr Point to the attribute of deca module
*	 @return 		AUI_RTN_SUCCESS Initialize the module successful
*	 @return 		EINVAL  Initialize the module failed,because input parameters invalid
*	 @return 		others  Initialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_init_callback(void *pDecaModuleAttr)
{
	aui_DecaAttachAttr DecaAttachAttr;
	int i=0;

	if((NULL==pDecaModuleAttr)||(((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt<=0))
	{
		return AUI_RTN_FAIL;
	}
	MEMCPY(&s_DecaModuleAttr,pDecaModuleAttr,sizeof(s_DecaModuleAttr));
	s_DecaModuleAttr.ul_init_flag=TRUE;

	MEMSET(&DecaAttachAttr,0,sizeof(DecaAttachAttr));
	for(i=0;i<((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt;i++)
	{
		DecaAttachAttr.ulDevIdx=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].ulDevIdx;
		DecaAttachAttr.bSupportAD=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].bSupportAD;
		DecaAttachAttr.ucDynaSample=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].ucDynaSample;
		DecaAttachAttr.deca_attach=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].deca_attach;
		DecaAttachAttr.deca_ext_dec_enable=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].deca_ext_dec_enable;
		DecaAttachAttr.deca_dvr_enable=((aui_pDecaModuleAttr)pDecaModuleAttr)->DecaAttachAttr[((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt-1].deca_dvr_enable;
		auiDevAttachDeca(DecaAttachAttr);
	}

	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		DeInitialize the deca module
*    @author		ray.gong
*    @date			2013-5-20
*	 @return 		AUI_RTN_SUCCESS DeInitialize the module successful
*	 @return 		EINVAL  DeInitialize the module failed,because input parameters invalid
*	 @return 		others  DeInitialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_deinit_callback(void *pDecaModuleAttr)
{
	struct deca_device *pDevDeca=NULL;
	int i=0;

	if((NULL==pDecaModuleAttr)||(((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt<=0))
	{
		return AUI_RTN_FAIL;
	}

	for(i=0;i<((aui_pDecaModuleAttr)pDecaModuleAttr)->ucDevCnt;i++)
	{
		pDevDeca=(struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, i);
		pDevDeca->detach(&pDevDeca);
	}
	s_DecaModuleAttr.ul_init_flag=FALSE;

	return AUI_RTN_SUCCESS;
}

void auiDevAttachSnd(aui_SndAttachAttr SndAttachAttr)
{
#ifndef _BUILD_OTA_E_
	struct snd_feature_config snd_config;
	MEMSET(&snd_config, 0, sizeof(struct snd_feature_config));
	snd_config.output_config.mute_num = SndAttachAttr.mute_num;
	snd_config.output_config.mute_polar 	= SndAttachAttr.mute_polar;
	snd_config.output_config.dac_precision = SndAttachAttr.dac_precision;
	snd_config.output_config.dac_format 	= SndAttachAttr.dac_format;
	snd_config.output_config.is_ext_dac 	= SndAttachAttr.is_ext_dac;
	snd_config.output_config.ext_mute_mode = SndAttachAttr.ext_mute_mode;
	snd_config.support_spdif_mute = SndAttachAttr.support_spdif_mute;
    snd_config.output_config.chip_type_config = SndAttachAttr.chip_type_config;
	snd_config.support_desc = SndAttachAttr.support_desc;
	snd_config.conti_clk_while_ch_chg = 1;//effect the 3503 beep tone voice

#ifdef SUPPORT_AUDIO_DELAY
    snd_config.support_desc |= SND_SUPPORT_DDPLUS;
#endif

#ifdef AD_SEE_OSD2_SHARE_BUFFER
    // AUI project default enable this macro, to use preset memory
    snd_config.ad_static_mem_flag = 1;//ad use the static memory
    snd_config.ad_static_mem_addr = __MM_AD_SND_MEM_ADDR;
    snd_config.ad_static_mem_size = __MM_AD_SND_MEM_LEN;
#else
    snd_config.ad_static_mem_flag = 0;//ad use the static memory
#endif

    (SndAttachAttr.snd_attach)(&snd_config);
    snd_m36g_init_tone_voice((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0));
#endif
}
#if 0
static void aui_set_subt_parameter(struct subt_config_par *pconfig_par)
	{
		pconfig_par->sys_sdram_size = SYS_SDRAM_SIZE;
		pconfig_par->max_subt_height = 576;
		pconfig_par->osd_layer_id = 1;	
		
    #ifdef CC_BY_VBI
		pconfig_par->cc_by_vbi = TRUE;
    #else
		pconfig_par->cc_by_vbi = FALSE;
    #endif
	
    #ifdef CC_BY_OSD
		pconfig_par->cc_by_osd = TRUE;
    #else
		pconfig_par->cc_by_osd = FALSE;
    #endif
	
    #ifdef OSD_BLOCKLINK_SUPPORT
		pconfig_par->osd_blocklink_enable = TRUE;
		pconfig_par->g_ps_buf_addr = 0;
		pconfig_par->g_ps_buf_len = 0;
    #else
		pconfig_par->osd_blocklink_enable = FALSE;
		pconfig_par->g_ps_buf_addr = (UINT8*)__MM_SUB_PB_START_ADDR;
		pconfig_par->g_ps_buf_len = __MM_SUB_PB_LEN;
    #endif
	
    #ifdef SPEED_UP_SUBT
		pconfig_par->speed_up_subt_enable = TRUE;
    #else
		pconfig_par->speed_up_subt_enable = FALSE;
    #endif
	
    #ifdef HD_SUBTITLE_SUPPORT
		pconfig_par->hd_subtitle_support = TRUE;
		pconfig_par->osd_get_scale_para = NULL; 
    #else
		pconfig_par->hd_subtitle_support = FALSE;
		pconfig_par->osd_get_scale_para = NULL;
    #endif
	}
#endif

static void aui_set_vbi_parameter(void)
{
#if    (TTX_ON == 1)
    struct vbi_config_par vbi_config;
    MEMSET(&vbi_config,0,sizeof(struct vbi_config_par));

    vbi_config.ttx_by_vbi = TRUE;  //vbi data is transmitted through vbi
    vbi_config.cc_by_vbi = FALSE;
    vbi_config.vps_by_vbi = FALSE;
    vbi_config.wss_by_vbi = FALSE;

    vbi_config.hamming_24_16_enable = FALSE;
    vbi_config.hamming_8_4_enable = FALSE;
    vbi_config.erase_unknown_packet = TRUE; //By default, unused packets are cleared

    vbi_config.user_fast_text = FALSE;
    vbi_config.mem_map.sbf_start_addr = __MM_TTX_BS_START_ADDR; //stream buffer start addr
    vbi_config.mem_map.sbf_size = __MM_TTX_BS_LEN;
	vbi_attach(&vbi_config);

    struct vbi_device *vbi_config_dev = NULL;
    vbi_config_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI,0);
    vbi_hld_enable_ttx_by_osd(vbi_config_dev);
#endif

#if(CC_ON == 1)
	vbi_cc_init();
#endif
}

/**
*    @brief 		Initialize the snd module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		pSndModuleAttr Point to the attribute of snd module
*	 @return 		SUCCESS Initialize the module successful
*	 @return 		EINVAL  Initialize the module failed,because input parameters invalid
*	 @return 		others  Initialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_init_callback(void *pSndModuleAttr)
{
	aui_SndAttachAttr SndAttachAttr;
	int i=0;

	if(NULL==pSndModuleAttr)
	{
		return AUI_RTN_FAIL;
	}
	MEMCPY(&s_SndModuleAttr,pSndModuleAttr,sizeof(s_SndModuleAttr));
	s_SndModuleAttr.ul_init_flag=TRUE;

	MEMSET(&SndAttachAttr,0,sizeof(SndAttachAttr));
	for(i=0;i<((aui_pSndModuleAttr)pSndModuleAttr)->ucDevCnt;i++)
	{
		MEMCPY(&SndAttachAttr,&(((aui_pSndModuleAttr)pSndModuleAttr)->SndAttachAttr[i]),sizeof(SndAttachAttr));
		auiDevAttachSnd(SndAttachAttr);
	}
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		DeInitialize the snd module
*    @author		ray.gong
*    @date			2013-5-20
*	 @return 		SUCCESS DeInitialize the module successful
*	 @return 		EINVAL  DeInitialize the module failed,because input parameters invalid
*	 @return 		others  DeInitialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_deinit_callback(void *pSndModuleAttr)
{
	struct snd_device *pDevSnd=NULL;
	int i=0;

	if((NULL==pSndModuleAttr)||(((aui_pSndModuleAttr)pSndModuleAttr)->ucDevCnt<=0))
	{
		return AUI_RTN_FAIL;
	}

	for(i=0;i<((aui_pSndModuleAttr)pSndModuleAttr)->ucDevCnt;i++)
	{
		pDevSnd=(struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, i);
		pDevSnd->detach(&pDevSnd);
	}
	s_SndModuleAttr.ul_init_flag=FALSE;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_init_callback(void *pNimModuleAttr)
{
    (void)pNimModuleAttr;
    //nim_init_cb(pSndModuleAttr);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gfx_init_callback(void *pGfxModuleAttr)
{
    (void)pGfxModuleAttr;
    ge_dev_attach();
	return AUI_RTN_SUCCESS;
}

void aui_dis_init_callback()
{
    vpo_dev_attach();
}

/**
*    @brief 		Initialize the dmx module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		pDmxModuleAttr Point to the attribute of snd module
*	 @return 		SUCCESS Initialize the module successful
*	 @return 		EINVAL  Initialize the module failed,because input parameters invalid
*	 @return 		others  Initialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_dmx_init_callback(void *pSndModuleAttr)
{
    (void)pSndModuleAttr;
	dmx_dev_attach();
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		DeInitialize the snd module
*    @author		ray.gong
*    @date			2013-5-20
*	 @return 		SUCCESS DeInitialize the module successful
*	 @return 		EINVAL  DeInitialize the module failed,because input parameters invalid
*	 @return 		others  DeInitialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_dmx_deinit_callback(void *pDmxModuleAttr)
{
    (void)pDmxModuleAttr;
	s_SndModuleAttr.ul_init_flag=FALSE;
	return AUI_RTN_SUCCESS;
}







AUI_RTN_CODE aui_otp_init_callback(void *pOtpModuleAttr)
{
	int i=0;
	aui_OtpModuleAttr *mod_attr_otp=pOtpModuleAttr;

	if(NULL==pOtpModuleAttr)
	{
		return AUI_RTN_FAIL;
	}
	for(i=0;i<mod_attr_otp->ucDevCnt;i++)
	{
		otp_init(mod_attr_otp->AttrOtp[i].OtpInitCfg);
	}
	s_OtpModuleAttr.ul_init_flag=TRUE;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_otp_deinit_callback()
{
	s_OtpModuleAttr.ul_init_flag=FALSE;
	return AUI_RTN_SUCCESS;
}

//static aui_tsg_module_attr s_TsgModuleAttr;
/**
*    @brief 		Initialize the tsg module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		pTsgModuleAttr Point to the attribute of tsg module
*	 @return 		SUCCESS Initialize the module successful
*	 @return 		EINVAL  Initialize the module failed,because input parameters invalid
*	 @return 		others  Initialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_tsg_init_callback(void *pTsgModuleAttr)
{
    (void)pTsgModuleAttr;
    AUI_RTN_CODE ret= AUI_RTN_FAIL;
#ifndef _BUILD_OTA_E_
#ifdef SUPPORT_CAS_A
        ret = tsg_init(0x5a);
#else
        ret = tsg_init(0x18);
#endif
#endif
	return ret;
}

/**
*    @brief 		DeInitialize the tsg module
*    @author		ray.gong
*    @date			2013-5-20
*	 @return 		SUCCESS DeInitialize the module successful
*	 @return 		EINVAL  DeInitialize the module failed,because input parameters invalid
*	 @return 		others  DeInitialize the module failed
*    @note
*
*/
AUI_RTN_CODE aui_tsg_deinit_callback(void *pTsgModuleAttr)
{
    (void)pTsgModuleAttr;
	return AUI_RTN_SUCCESS;
}



/**
*    @brief 		3?：o???��DSC??┷?：|
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		pDscModuleAttr 	??┷?：|3?：o???��2?：oy
*	 @return 		AUI_RTN_SUCCESS 3：|1|
*	 @return 		AUI_RTN_EINVAL  ?TD?━：o?：：?2?：oy
*	 @return 		others  		：o?━??：1
*    @note
*
*/
AUI_RTN_CODE aui_dsc_init_callback(void *pDscModuleAttr)
{
    (void)pDscModuleAttr;
	dsc_api_attach();
	s_DscModuleAttr.ul_init_flag=TRUE;

	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		：：♀∴3?：o???��DSC??┷?：|
*    @author		ray.gong
*    @date			2013-5-20
*	 @return 		AUI_RTN_SUCCESS 3：|1|
*	 @return 		AUI_RTN_EINVAL  ?TD?━：o?：：?2?：oy
*	 @return 		others  		：o?━??：1
*    @note
*
*/
AUI_RTN_CODE aui_dsc_deinit_callback(void *pDscModuleAttr)
{
    (void)pDscModuleAttr;
	dsc_api_detach();
	s_DscModuleAttr.ul_init_flag=FALSE;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_hdmi_init_callback(void *pHdmiModuleAttr)
{
    (void)pHdmiModuleAttr;
    struct config_hdmi_parm hdmi_param;
    memset(&hdmi_param,0,sizeof(struct config_hdmi_parm));
    get_hdmi_config(&hdmi_param);
    hdmi_tx_attach(&hdmi_param);
	return AUI_RTN_SUCCESS;
}

void aui_decv_init_callback(void)
{
	vdec_dev_attach();
}

#ifndef _BUILD_OTA_E_
extern AUI_RTN_CODE aui_mp_memory_info_set(unsigned long addr1,unsigned long len1,unsigned long addr2,
                                                                                   unsigned long len2, unsigned long addr3,unsigned long len3);
AUI_RTN_CODE aui_mp_init_callback(void *pMpModuleAttr)
{
	(void)pMpModuleAttr;
    /*member to store address that client malloc for mp module.
    The work buffer of video engine, which will be devided into some different size of buffer:
    pe_cache_buf, used by pe cache; ring buf:used to store PCM data after decode audio; 
    frame buf: used by decoding video; display buf and cmd queue buf.*/
    unsigned long mp_decoder_buf = 0;
    //the size of decoder_buf, the recommend value is 0x754f00
    unsigned long mp_decoder_buf_len= 0;
    //buffer used to download the source data for see CPU reading.
    unsigned long mp_pecache_buf = 0;
    //the length of pe cache buffer, and if set ul_pe_cache_buf= p_cfg->ul_frame_buf, 
    //media player engine will allocate pe_cache buffer from ul_frame_buf. the size is 
    //0x120000 byte
    unsigned long mp_pecache_buf_len = 0;
    //buffer used to store VFB address
    unsigned long mp_private_buf = 0;
    //the length of mp_private_buf
    unsigned long mp_private_buf_len = 0;

    mp_decoder_buf = __MM_VIDEO_FILE_BUF_ADDR;
    mp_decoder_buf_len = __MM_VIDEO_FILE_BUF_LEN;

     //video file buffer is seperated with frame buffer
     /*the decoder buffer of mp do not include the frame buffer, and the frame buffer address and length will be
     introduced by struct video_cfg_extra. So the length of mp is 14.6M (not including the length of frame buffer:38M)*/
    #if (__MM_FB_TOP_ADDR > __MM_VIDEO_FILE_BUF_ADDR)
        mp_decoder_buf_len = __MM_FB_TOP_ADDR - __MM_VIDEO_FILE_BUF_ADDR;
    #endif

    #if (defined (SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_) || defined (_C1800A_CA_ENABLE_))
        #ifdef _USE_64M_MEM_
            mp_pecache_buf = (__MM_PVR_VOB_BUFFER_ADDR+0xfff)&0xfffff000;
            mp_pecache_buf_len = __MM_NIM_BUFFER_ADDR - mp_pecache_buf;
        #else
            // if set ul_pe_cache_buf= p_cfg->ul_frame_buf, media player engine will allocate pe_cache buffer from ul_frame_buf
            // the size is 0x120000 byte
            mp_pecache_buf = __MM_FB_BOTTOM_ADDR;
            mp_pecache_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
        #endif
    #endif

    #ifdef VFB_SUPPORT
        /* private buffer */
        mp_private_buf = VDEC_PRIVATE_START_ADDR;
        mp_private_buf_len = __MM_PRIVATE_TOP_ADDR - VDEC_PRIVATE_START_ADDR;
    #else
        /* private buffer */
        #if defined(_M3505_) && defined(SD_PVR)
            //for 3505 sd
            mp_private_buf = __MM_DMX_SEE_BLK_ADDR; //DMX SEE + VBV +  MAF.
        #else
            mp_private_buf = __MM_MAF_START_ADDR;
        #endif    
            mp_private_buf_len = __MM_PRIVATE_TOP_ADDR - mp_private_buf;
    #endif

    aui_mp_memory_info_set(mp_decoder_buf, mp_decoder_buf_len, mp_pecache_buf, mp_pecache_buf_len, mp_private_buf, mp_private_buf_len);
    return AUI_RTN_SUCCESS;
}

extern AUI_RTN_CODE aui_music_memory_info_set(unsigned long addr,unsigned long len);
AUI_RTN_CODE aui_music_init_callback(void *pMusicModuleAttr)
{	
	(void)pMusicModuleAttr;
    /*member to store address that client malloc for music module, this buffer will
    be used to allocate memory for two buffers,one is used to store decoded data, and the
    size should not be less than 0x5000; another is used to store the processed decoded
    data, and the size should not be less than 0x1f5400*/
    unsigned long music_buf;
    /*the length of ul_music_buffer, the value should not be less than 0x1fa400*/
    unsigned long music_buf_len;

    music_buf =  __MM_OSD_BK_ADDR2 ;
    music_buf_len = __MM_OSD2_LEN;//__MM_OSD2_LEN;

    aui_music_memory_info_set(music_buf, music_buf_len);
    return AUI_RTN_SUCCESS;
}

extern AUI_RTN_CODE aui_image_memory_info_set(unsigned long addr1,unsigned long len1, unsigned long addr2,unsigned long len2);
AUI_RTN_CODE aui_image_init_callback(void *pMusicModuleAttr)
{
	(void) pMusicModuleAttr;
     /*member to store address that client malloc for image module, the driver will allocate
    memory for three buffer used for DE displaying.*/
    unsigned long image_de_buf;
     /*the length of ul_image_buffer, the recommend value is 0x8F8200*/
    unsigned long image_de_buf_len;
     /*buffer used for decoder*/
    unsigned long image_decoder_buf;
     /*the length of image_decoder_buffer, the recommend value is 0x752800*/
    unsigned long image_decoder_buf_len;
     
     //fill the image engine config
#if defined(HW_SECURE_ENABLE) || !defined(_RETAILER_PROJECT_)
    #if defined(_USE_64M_MEM_)
             //image decode in MAIN side.

        #if defined(_M3505_)
            image_de_buf = (__MM_EPG_BUFFER_START+0xfff)&0xfffff000;
        #else
            image_de_buf = (__MM_FB_START_ADDR);
        #endif
        image_de_buf_len = 9*__MM_IMG_FB_C_LEN;
        image_decoder_buf = image_decoder_buf + image_decoder_buf_len;
        
        #if defined(_M3505_)
            image_decoder_buf_len = (__MM_FB_TOP_ADDR -(unsigned long)image_decoder_buf)
        #else
            image_decoder_buf_len = (__MM_NIM_BUFFER_ADDR -(unsigned long)image_decoder_buf);
        #endif
        
    #else
           //image decode in MAIN side.    
        #if defined(_S3281_) 
            //IMAGE decode in MAIN cpu.
            //IMAGE USE ( PVR BUFFER+FRAME BUFFER ).
            image_de_buf= __MM_PVR_VOB_BUFFER_ADDR;
            image_de_buf_len = 9*__MM_FB0_C_LEN;
            image_decoder_buf = (unsigned char *)image_de_buf + image_decoder_buf_len;
            image_decoder_buf_len = (__MM_PVR_VOB_BUFFER_LEN );
        #elif (defined(_M3505_) || defined(_M3702_) || defined(_M3503D_)|| defined(_M3711C_))
            image_decoder_buf = __MM_FB_BOTTOM_ADDR;
            image_decoder_buf_len = 0xb00000;
            if (image_decoder_buf_len < 0xb00000){
                image_decoder_buf_len = 0xb00000;
            }
            image_de_buf = (unsigned long) image_decoder_buf + image_decoder_buf_len;
            image_de_buf_len = 9*__MM_FB0_C_LEN;
        #else
            image_decoder_buf= (unsigned char *)(__MM_PVR_VOB_BUFFER_ADDR+__MM_PVR_VOB_BUFFER_LEN-0x200000);
            image_decoder_buf_len =  __MM_FB_BOTTOM_ADDR - (unsigned long)(image_decoder_buf);
            if (image_decoder_buf_len < 0xb00000){
                image_decoder_buf_len = 0xb00000;
            }
            image_de_buf = image_decoder_buf + image_decoder_buf_len;
            image_de_buf_len = 9*__MM_FB0_C_LEN;
        #endif
    #endif
#else
    image_de_buf = __MM_FB_START_ADDR;
    image_de_buf_len = 9*__MM_FB0_C_LEN;
        if(sys_ic_get_chip_id() == ALI_S3811)
    {
        image_decoder_buf = (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR;
        image_decoder_buf_len = __MM_PVR_VOB_BUFFER_LEN;
    }
    else
    {
        image_decoder_buf = (unsigned char *)__MM_MP_BUFFER_ADDR;
        image_decoder_buf_len = __MM_MP_BUFFER_LEN;
    }
 #endif
    aui_image_memory_info_set(image_de_buf, image_de_buf_len, image_decoder_buf, image_decoder_buf_len);   
    return AUI_RTN_SUCCESS;
}
#endif

AUI_RTN_CODE aui_smc_init_callback(void *pSmcModuleAttr)
{
    (void)pSmcModuleAttr;
	smc_attach();
	return AUI_RTN_SUCCESS;
}
#ifdef AUI_CI_SUPPORT
AUI_RTN_CODE aui_cic_init_callback(void *pSmcModuleAttr)
{
    (void)pSmcModuleAttr;
	cic_m3602_attach();
	return AUI_RTN_SUCCESS;
}
#endif

AUI_RTN_CODE aui_vbi_init_callback(void *pVbiModuleAttr)
{
    (void)pVbiModuleAttr;
	aui_set_vbi_parameter();
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE dog_test(void *pv_param)
{
    (void)pv_param;
	return 0;
}
void aui_board_advance_init(advance_cfg_para* cfg_para)
{
    advance_cfg_para* p_cfg_para = NULL;

    p_cfg_para = get_board_advance_cfg();

    if (NULL == p_cfg_para) {
        return;
    }
    
    if(NULL != cfg_para)
    {
        MEMCPY(p_cfg_para, cfg_para, sizeof(advance_cfg_para));
    }

	aui_DecaModuleAttr DecaModuleAttr;
	MEMSET(&DecaModuleAttr,0,sizeof(aui_DecaModuleAttr));
	aui_SndModuleAttr SndModuleAttr;
	MEMSET(&SndModuleAttr,0,sizeof(aui_SndModuleAttr));
	aui_dmx_module_attr DmxModuleAttr;
	MEMSET(&DmxModuleAttr,0,sizeof(aui_dmx_module_attr));
    aui_nim_attr attr_nim;
    MEMSET(&attr_nim,0,sizeof(aui_nim_attr));
    #ifndef _BUILD_OTA_E_
	DecaModuleAttr.ucDevCnt=1;
	DecaModuleAttr.DecaAttachAttr[0].ulDevIdx=0;
	DecaModuleAttr.DecaAttachAttr[0].bSupportAD=p_cfg_para->audio_description_support;
	DecaModuleAttr.DecaAttachAttr[0].ucDynaSample=1;
	DecaModuleAttr.DecaAttachAttr[0].deca_attach=deca_m36_attach;
	DecaModuleAttr.DecaAttachAttr[0].deca_ext_dec_enable=deca_m36_ext_dec_enable;
	DecaModuleAttr.DecaAttachAttr[0].deca_dvr_enable=deca_m36_dvr_enable;
	if (aui_deca_init(aui_deca_init_callback,(void *)(&DecaModuleAttr)))
    {
		AUI_PRINTF("\n aui_deca_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_deca_init success---------------------------\n");
	SndModuleAttr.ucDevCnt=1;
	SndModuleAttr.SndAttachAttr[0].mute_num = p_cfg_para->sys_mute_gpio;
	SndModuleAttr.SndAttachAttr[0].mute_polar 	= p_cfg_para->sys_mute_polar;
	SndModuleAttr.SndAttachAttr[0].dac_precision = 24;
	SndModuleAttr.SndAttachAttr[0].dac_format 	= CODEC_I2S;
	SndModuleAttr.SndAttachAttr[0].is_ext_dac 	= 0;
	SndModuleAttr.SndAttachAttr[0].ext_mute_mode = p_cfg_para->ext_mute_mode;
	SndModuleAttr.SndAttachAttr[0].support_spdif_mute = 1;
	SndModuleAttr.SndAttachAttr[0].chip_type_config = p_cfg_para->snd_output_chip_type;
	SndModuleAttr.SndAttachAttr[0].support_desc = p_cfg_para->audio_description_support;
	SndModuleAttr.SndAttachAttr[0].snd_attach= snd_m36g_attach;
	if (aui_snd_init(aui_snd_init_callback,(void *)(&SndModuleAttr)))
	{
		AUI_PRINTF("\n aui_snd_init failed\n");
		return ;
	}
	AUI_PRINTF("\n-----------------------------------------aui_snd_init success---------------------------\n");

	if (aui_tsg_init(aui_tsg_init_callback,NULL))
	{
		AUI_PRINTF("\n aui_snd_init failed\n");
		return ;
	}
	AUI_PRINTF("\n-----------------------------------------aui_tsg_init success---------------------------\n");

    if (aui_decv_init(aui_decv_init_callback))
    {
		AUI_PRINTF("\n aui_decv_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_decv_init success---------------------------\n");

#ifndef _BUILD_OTA_E_
    if (aui_mp_init((void *)aui_mp_init_callback))
    {
		AUI_PRINTF("\n aui_mp_init failed\n");
		return;
	}
    AUI_PRINTF("\n-----------------------------------------aui_mp_init success---------------------------\n");
    
    if (aui_music_init((void *)aui_music_init_callback))
    {
		AUI_PRINTF("\n aui_music_init failed\n");
		return;
	}
    AUI_PRINTF("\n-----------------------------------------aui_music_init success---------------------------\n");
 
    if (aui_image_init((void *)aui_image_init_callback))
    {
		AUI_PRINTF("\n aui_image_init failed\n");
		return;
	}
    AUI_PRINTF("\n-----------------------------------------aui_image_init success---------------------------\n");
#endif
    if (AUI_RTN_SUCCESS != aui_rtc_init())
    {
		AUI_PRINTF("\n aui_rtc_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_rtc_init success---------------------------\n");

    if (aui_dog_init(dog_test, NULL) != AUI_RTN_SUCCESS) {
           AUI_PRINTF("watch dog module init error.\n");
           return ;
    }
    AUI_PRINTF("\n-----------------------------------------aui_dog_init success---------------------------\n");
    #endif
    
    if (aui_gfx_init(aui_gfx_init_callback, NULL))
    {
		AUI_PRINTF("\n aui_nim_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_gfx_init success---------------------------\n");
  
    DmxModuleAttr.ul_dev_cnt=4;
        DmxModuleAttr.ul_init_flag=TRUE;
        DmxModuleAttr.dmx_capability[0].ul_dev_idx=0;
        DmxModuleAttr.dmx_capability[0].dev_type=AUI_DMX_DEV_TYPE_HARDWARE;
        DmxModuleAttr.dmx_capability[1].ul_dev_idx=1;
        DmxModuleAttr.dmx_capability[1].dev_type=AUI_DMX_DEV_TYPE_HARDWARE;
        DmxModuleAttr.dmx_capability[2].ul_dev_idx=2;
        DmxModuleAttr.dmx_capability[2].dev_type=AUI_DMX_DEV_TYPE_HARDWARE;
        DmxModuleAttr.dmx_capability[3].ul_dev_idx=3;
        DmxModuleAttr.dmx_capability[3].dev_type=AUI_DMX_DEV_TYPE_SOFTWARE;
    if (aui_dmx_init(aui_dmx_init_callback,(void *)(&DmxModuleAttr)))
    {
                AUI_PRINTF("\n aui_dmx_init failed\n");
                return ;
        }
    AUI_PRINTF("\n-----------------------------------------aui_dmx_init success---------------------------\n");

	if (aui_nim_init(aui_nim_init_callback))
    {
		AUI_PRINTF("\n aui_nim_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_nim_init success---------------------------\n");

    if (aui_dis_init(aui_dis_init_callback))
    {
		AUI_PRINTF("\n aui_nim_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_dis_init success---------------------------\n");
#ifndef _BUILD_OTA_E_
    if (aui_smc_init(aui_smc_init_callback))
    {
		AUI_PRINTF("\n aui_smc_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_smc_init success---------------------------\n");
#endif

#ifdef AUI_CI_SUPPORT
    if (aui_cic_init(aui_cic_init_callback))
    {
		AUI_PRINTF("\n aui_cic_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_cic_init success---------------------------\n");
#endif


    if (aui_hdmi_init(aui_hdmi_init_callback))
    {
		AUI_PRINTF("\n aui_hdmi_init failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_hdmi_init success---------------------------\n");



    if(AUI_RTN_SUCCESS!=aui_dsc_init(aui_dsc_init_callback,NULL))
	{
	    libc_printf("\r\n aui_dsc_init fail\n");
	}
    AUI_PRINTF("\n-----------------------------------------aui_dsc_init success---------------------------\n");

#ifndef _BUILD_OTA_E_
    if (aui_vbi_init(aui_vbi_init_callback, NULL))
    {
		AUI_PRINTF("\n aui_vbi_init_callback failed\n");
		return ;
	}
    AUI_PRINTF("\n-----------------------------------------aui_vbi_init success---------------------------\n");
#endif

}
#endif

