#include "../advanced_cfg.h"
#ifdef SUPPORT_AVS_DECODER
#include <hld/decv/decv_avs.h>
#endif
#include <bus/otp/otp.h>

#ifdef ALICAST_SUPPORT
#include <api/libwfd/nmp_hdcp2.h>
#endif
#include <api/libsubt/subt_osd.h>

static advance_cfg_para g_cfg_para; 

advance_cfg_para *get_board_advance_cfg(void)
{
	    return &g_cfg_para;
}

extern void deca_m36_attach(struct deca_feature_config *config);
extern void deca_m36_ext_dec_enable(struct deca_device *dev, struct deca_feature_config *config);
extern void deca_m36_dvr_enable(struct deca_device *dev);
extern void snd_m36g_attach(struct snd_feature_config * config);
#ifdef HDTV_SUPPORT
static void avc_attach(void)
{
#ifndef _BUILD_OTA_E_	
	struct vdec_avc_config_par vdec_avc_cfg;

	MEMSET(&vdec_avc_cfg, 0, sizeof(struct vdec_avc_config_par));   
	vdec_avc_cfg.max_additional_fb_num = MAX_EXTRA_FB_NUM;
	vdec_avc_cfg.memmap.support_multi_bank = g_cfg_para.h264_support_mulit_bank;  //H264_SUPPORT_MULTI_BANK
    
    if(g_cfg_para.avc_unify_mem_support)  //AVC_SUPPORT_UNIFY_MEM
    {
    	vdec_avc_cfg.memmap.support_conti_memory = 1;   	
    	vdec_avc_cfg.memmap.vbv_buffer_base = (AVC_VBV_ADDR & 0x0fffffff) | 0x80000000;
    	vdec_avc_cfg.memmap.vbv_buffer_len = AVC_VBV_LEN;
    	vdec_avc_cfg.memmap.cmd_queue_buffer_base = (vdec_avc_cfg.memmap.vbv_buffer_base - AVC_CMD_QUEUE_LEN) | 0xa0000000;
    	vdec_avc_cfg.memmap.cmd_queue_buffer_len = AVC_CMD_QUEUE_LEN;
        #ifdef DUAL_ENABLE 
        vdec_avc_cfg.memmap.mb_col_buffer_base = AVC_MB_COL_ADDR;
        #else
    	vdec_avc_cfg.memmap.mb_col_buffer_base = vdec_avc_cfg.memmap.cmd_queue_buffer_base - AVC_MB_COL_LEN - AVC_MV_LEN;
        #endif
    	vdec_avc_cfg.memmap.mb_col_buffer_len = AVC_MB_COL_LEN + AVC_MV_LEN;
    	vdec_avc_cfg.memmap.mb_neighbour_buffer_base = vdec_avc_cfg.memmap.mb_col_buffer_base - AVC_MB_NEI_LEN;
    	vdec_avc_cfg.memmap.mb_neighbour_buffer_len = AVC_MB_NEI_LEN;
    	vdec_avc_cfg.memmap.avc_mem_len = AVC_MEM_LEN;
    	vdec_avc_cfg.memmap.avc_mem_addr = (vdec_avc_cfg.memmap.mb_neighbour_buffer_base - vdec_avc_cfg.memmap.avc_mem_len + 1023) & 0xfffffc00;
        vdec_avc_cfg.memmap.avc_mem_len = vdec_avc_cfg.memmap.mb_neighbour_buffer_base - vdec_avc_cfg.memmap.avc_mem_addr;
    }
    else
    {
        vdec_avc_cfg.memmap.frame_buffer_base = AVC_FB_ADDR;
        vdec_avc_cfg.memmap.frame_buffer_len = AVC_FB_LEN;
        vdec_avc_cfg.memmap.dv_frame_buffer_base = AVC_DVIEW_ADDR;
        vdec_avc_cfg.memmap.dv_frame_buffer_len = AVC_DVIEW_LEN;        
        vdec_avc_cfg.memmap.mv_buffer_base = AVC_MV_ADDR;
        vdec_avc_cfg.memmap.mv_buffer_len = AVC_MV_LEN;
        vdec_avc_cfg.memmap.mb_col_buffer_base = AVC_MB_COL_ADDR;
        vdec_avc_cfg.memmap.mb_col_buffer_len = AVC_MB_COL_LEN;
        vdec_avc_cfg.memmap.mb_neighbour_buffer_base = AVC_MB_NEI_ADDR;
        vdec_avc_cfg.memmap.mb_neighbour_buffer_len = AVC_MB_NEI_LEN;
        vdec_avc_cfg.memmap.cmd_queue_buffer_base = AVC_CMD_QUEUE_ADDR;
        vdec_avc_cfg.memmap.cmd_queue_buffer_len = AVC_CMD_QUEUE_LEN;
        //vdec_avc_cfg.memmap.vbv_buffer_base = 0x84000000;
        //vdec_avc_cfg.memmap.vbv_buffer_len = 0x1000000;      
        vdec_avc_cfg.memmap.vbv_buffer_base = (AVC_VBV_ADDR & 0x0fffffff) | 0x80000000;
        vdec_avc_cfg.memmap.vbv_buffer_len = AVC_VBV_LEN;
        vdec_avc_cfg.memmap.laf_rw_buf = AVC_LAF_RW_BUF_ADDR;
        vdec_avc_cfg.memmap.laf_rw_buffer_len = AVC_LAF_RW_BUF_LEN;
        vdec_avc_cfg.memmap.laf_flag_buf = AVC_LAF_FLAG_BUF_ADDR;
        vdec_avc_cfg.memmap.laf_flag_buffer_len = AVC_LAF_FLAG_BUF_LEN;

        #ifdef DYNAMIC_RESOLUTION_SUPPORT
        vdec_avc_cfg.memmap.support_conti_memory = 2;
        #endif
    }

#ifndef _BUILD_OTA_E_
	//print_memory_layout();
#endif

    vdec_avc_cfg.dtg_afd_parsing = g_cfg_para.afd_scale_support;
    
    vdec_avc_attach(&vdec_avc_cfg);
#endif
}

#ifdef SUPPORT_AVS_DECODER
static void avs_config(void)//struct vdec_avc_config_par *par)
{
struct vdec_avs_config_par  vdec_avs_cfg;


	MEMSET(&vdec_avs_cfg, 0, sizeof(struct vdec_avs_config_par));
    	vdec_avs_cfg.memmap.support_multi_bank = FALSE;
    vdec_avs_cfg.memmap.vbv_buffer_base = (AVS_VBV_ADDR & 0x0fffffff) | 0x80000000;
    vdec_avs_cfg.memmap.vbv_buffer_len = AVS_VBV_LEN;
    vdec_avs_cfg.memmap.cmd_queue_buffer_base = AVS_CMD_QUEUE_ADDR;
    vdec_avs_cfg.memmap.cmd_queue_buffer_len = AVS_CMD_QUEUE_LEN;
    	vdec_avs_cfg.memmap.mb_neighbour_buffer_base = AVS_MB_NEI_ADDR;
    	vdec_avs_cfg.memmap.mb_neighbour_buffer_len = AVS_MB_NEI_LEN;
    vdec_avs_cfg.memmap.avs_mem_addr = AVS_MEM_ADDR;
    vdec_avs_cfg.memmap.avs_mem_len= AVS_MEM_LEN;

	vdec_avs_attach(&vdec_avs_cfg);
}
#endif

#if 0 //def HDCP_FROM_CE
/********************************************************************************
*				Pls follow the release rule as following									*
*	when want to release code or bin file to customer									*
*	1. Set the vendor name, and product description  									*
*	2. Assigned the gpio pin number for AT88SC0404C I2C_SCL and I2C_SDA  				*
*	3. For mass production, customer should assigned password( 3 bytes ) 					*
*         and Secret Seed ( 8 bytes ) array. 												*
*	    The assigned password/secret seed value should be the same with programmed in		*
*	    AT88SC0404C crypto memory.							 						*
*	   For piolt production, please leave these point to NULL								*
*********************************************************************************/
static RET_CODE ce_generate_hdcp()
{
	UINT16 i,j;
	UINT32 id = HDCPKEY_CHUNK_ID;
	UINT32 hdcp_chunk;
	UINT8 *hdcp_internal_keys;
	UINT8 temp_data[16];

	CE_DATA_INFO Ce_data_info ;
	pOTP_PARAM opt_info;
	pCE_DEVICE ce_dev = (pCE_DEVICE)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
	CE_DEBUG_KEY_INFO	ce_param;

	opt_info = MALLOC(sizeof(OTP_PARAM));
	MEMSET(opt_info ,0 , sizeof(OTP_PARAM) );
	
	opt_info->otp_addr = OTP_ADDESS_4; // load m2m2
	opt_info->otp_key_pos = KEY_0_3;
	
	if( RET_SUCCESS != ce_key_load(ce_dev , opt_info))
	{
		FREE( opt_info );
		return !RET_SUCCESS;	
	}

				
	hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*288);
	hdcp_chunk = (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1);
	sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
		hdcp_chunk+ CHUNK_HEADER_SIZE, 288);
	for(i=0;i<18;i++)
	{
		MEMCPY(temp_data,(hdcp_internal_keys+i*16),16);

		MEMCPY(&(Ce_data_info.otp_info),opt_info,sizeof(OTP_PARAM));
		Ce_data_info.data_info.data_len 				= 16;
		Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_AES ; 	
		Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	
		Ce_data_info.des_aes_info.des_low_or_high 	= 0;					
		Ce_data_info.key_info.first_key_pos 			= KEY_0_3;
		Ce_data_info.key_info.hdcp_mode 			= TARGET_IS_HDCP_KEY_SRAM; 
		Ce_data_info.key_info.second_key_pos 		= i; 
		MEMCPY(Ce_data_info.data_info.crypt_data,temp_data,16);
	
		if(RET_SUCCESS != ce_generate_single_level_key(ce_dev, &Ce_data_info))
		{
			FREE( opt_info );
			FREE(hdcp_internal_keys);
			return !RET_SUCCESS;
		}
#if 0//for debug
		ce_param.len = 4; 
		ce_param.sel = HDCP_KEY_READ ;
		ce_ioctl(ce_dev,IO_CRYPT_DEBUG_GET_KEY,&ce_param);  
		osal_task_sleep(100);
		libc_printf("hdcp %08x,%08x,%08x,%08x\n",ce_param.buffer[0],ce_param.buffer[1],ce_param.buffer[2],ce_param.buffer[3]);
#endif
	}

	FREE( opt_info );
	FREE(hdcp_internal_keys);
	return RET_SUCCESS;
}
#endif

void get_hdmi_config(struct config_hdmi_parm *hdmi_param)
{
    if(NULL == hdmi_param)
        return;
    
	MEMSET(hdmi_param, 0, sizeof(struct config_hdmi_parm));

	hdmi_param->spd_vendor_name = (UINT8*)"ALi Corp";
	hdmi_param->spd_product_desc = (UINT8*)"M3602 HD STB";
	hdmi_param->i2c_for_eddc = g_cfg_para.i2c_for_eddc; //I2C_FOR_EDDC;	//I2C_TYPE_SCB2, I2C_TYPE_GPIO1
	hdmi_param->otp_ce_load_hdcp_key = FALSE;	
	hdmi_param->hdcp_disable = g_cfg_para.hdcp_disable; 
	hdmi_param->boot_media = g_cfg_para.vpo_active;

  hdmi_param->hdcp_disable =TRUE;
	if(hdmi_param->hdcp_disable == FALSE)
	{
		hdmi_param->cm_scl = g_cfg_para.hdmi_cm_scl; //SYS_I2C_SCL2;
		hdmi_param->cm_sda = g_cfg_para.hdmi_cm_sda; //SYS_I2C_SDA2;	
        
//		hdmi_param->hdcp_cstm_key = CUSTOMER_HDCP_KEY;	 <=== CUSTOMER_HDCP_KEY is a 286 Bytes UINT8 arrary.
#ifndef HDCP_IN_FLASH	// Customer assigned HDCP Key from interface. (286 Bytes)
		hdmi_param->hdcp_cstm_key = NULL;// 					<=== if load key from external cryptomemory, please assign NULL to it.
		//hdmi_param->cm_scl = SYS_I2C_SCL2;
		//hdmi_param->cm_sda = SYS_I2C_SDA2;		
	
		hdmi_param->cm_password = NULL;   // ALi Demo
		hdmi_param->cm_secretseed = NULL; // ALi Demo 
#else
#ifdef HDCP_FROM_CE
		hdmi_param->otp_ce_load_hdcp_key = TRUE;
		hdmi_param->hdcp_cstm_key = NULL;// 					<=== if load key from external cryptomemory, please assign NULL to it.
		//hdmi_param->cm_scl = SYS_I2C_SCL2;
		//hdmi_param->cm_sda = SYS_I2C_SDA2;		
		hdmi_param->cm_password = NULL;   // ALi Demo
		hdmi_param->cm_secretseed = NULL; // ALi Demo 
#else
		hdmi_param->otp_ce_load_hdcp_key = FALSE; // For 3603 decrypt hdcp by otp root password, shuold assign TRUE
		{
			UINT8	*hdcp_internal_keys, all_is_0xff[286];
			__MAYBE_UNUSED__ UINT8	Key1[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
			__MAYBE_UNUSED__ UINT8	Key2[8] = {0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01};
			#ifdef C3041
			__MAYBE_UNUSED__ UINT8	Key3[8] = {0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x01};
			#else
			__MAYBE_UNUSED__ UINT8	Key3[8] = {0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23};		
			#endif
			UINT8	Plaintext[8];
			UINT16	 i;
			UINT32 id = HDCPKEY_CHUNK_ID;
			UINT32 hdcp_chunk;
				
			//Load ALi licsensed key here from ATMEL Crypto memory
			hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*286);
			memset(all_is_0xff, 0xff, 286);
			hdcp_chunk = (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) ;
			if((UINT32)ERR_FAILUE == hdcp_chunk)
			{
				hdmi_param->hdcp_cstm_key = NULL;
				//hdmi_param->cm_scl = SYS_I2C_SCL2;
				//hdmi_param->cm_sda = SYS_I2C_SDA2; 
				hdmi_param->cm_password = NULL;   // ALi Demo
				hdmi_param->cm_secretseed = NULL; // ALi Demo										
			}
			else
			{
				sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
						hdcp_chunk+ CHUNK_HEADER_SIZE, 286);

				//hdmi_param->cm_scl = SYS_I2C_SCL2;
				//hdmi_param->cm_sda = SYS_I2C_SDA2; 
				hdmi_param->cm_password = NULL;   // ALi Demo
				hdmi_param->cm_secretseed = NULL; // ALi Demo				
	
				if(MEMCMP(hdcp_internal_keys, all_is_0xff, 286) == 0)
				{	
					// No HDCP Key in flash
					hdmi_param->hdcp_cstm_key = NULL;
				}							
				else
				{
						 // Skip the first 6 bytes. 0x00 + 5bytes KSV	 
					for(i=6; i<286; i+=8)
					{
						triple_des_decryption(hdcp_internal_keys+i, Plaintext, Key1, Key2, Key3);//chuhua temp closed for compiler error.
						memcpy(hdcp_internal_keys+i, Plaintext, 8); 
						
					#if 0
					    libc_printf("HDCP key: %08x,%08x\n",((UINT32*)Plaintext)[0],((UINT32*)Plaintext)[1]);
					#endif
				 	}
					hdmi_param->hdcp_cstm_key = hdcp_internal_keys; 			 
				}
			}			
		}
#endif
#endif
		
	}
}

#endif

#ifdef ALICAST_SUPPORT
#define HDCP20KEY_CHUNK_ID 0x09F60200
 tReceiverKeySets g_H2KeySets;

static void get_hdmi20_config(tReceiverKeySets *h2_KeySets)
{
	UINT8	*hdcp_internal_keys, all_is_0xff[858];
	UINT8	Key1[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
	UINT8	Key2[8] = {0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01};			
	UINT8	Key3[8] = {0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23};		
		
	UINT8	Plaintext[8];
	UINT16	 i;
	UINT32 id = HDCP20KEY_CHUNK_ID;//HDCPKEY20_CHUNK_ID;
	UINT32 hdcp_chunk;
    
	MEMSET(h2_KeySets, 0, sizeof(tReceiverKeySets));		
        
	//hdmi_param->hdcp_cstm_key = CUSTOMER_HDCP_KEY;	 <=== CUSTOMER_HDCP_KEY is a 286 Bytes UINT8 arrary.		
				
	//Load ALi licsensed key here from ATMEL Crypto memory
	hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*858);
	memset(all_is_0xff, 0xff, 858);
	hdcp_chunk = (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) ;
			
	sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,hdcp_chunk+ CHUNK_HEADER_SIZE, 858);
	if(MEMCMP(hdcp_internal_keys, all_is_0xff, 858) == 0)
	{	
		// No HDCP Key in flash
		//hdmi_param->hdcp_cstm_key = NULL;
	}							
	else
	{
		for(i=0; i<856; i+=8)//last remain 2 bytes ,it don't decrypt
		{
			TripleDES_Decryption(hdcp_internal_keys+i, Plaintext, Key1, Key2, Key3);	   
			memcpy(hdcp_internal_keys+i, Plaintext, 8); 			
			
		 }

		memcpy( &h2_KeySets->lc,&hdcp_internal_keys[0] , 16 );
		memcpy( &h2_KeySets->rxPubKeyCert,&hdcp_internal_keys[16] , 522 );
		memcpy( &h2_KeySets->kPrivRx,&hdcp_internal_keys[538] , 320 );
			//hdmi_param->hdcp_cstm_key = hdcp_internal_keys; 	

	}
}
#endif

static void deca_dev_attach(void)
{
#ifndef _BUILD_OTA_E_
    struct deca_feature_config deca_config;
	MEMSET(&deca_config, 0, sizeof(struct deca_feature_config));   
	deca_config.support_desc = g_cfg_para.audio_description_support;   
	/* support dynamic sample rate change by default*/
	deca_config.detect_sprt_change = 1; 

#if(defined(_SUPPORT_64M_MEM)&& defined(AD_MV_SHARE_BUFFER))
    deca_config.ad_static_mem_flag = 1;//ad use the static memory
    deca_config.ad_static_mem_addr = __MM_AD_DECA_MEM_ADDR;
    deca_config.ad_static_mem_size = __MM_AD_DECA_MEM_LEN;
#else
    deca_config.ad_static_mem_flag = 0;//ad not use the static memory, ad malloc
#endif
#ifdef _DD_SUPPORT
    deca_config.priv_dec_addr = __MM_PRIV_DEC_ADDR;
    deca_config.priv_dec_size = __MM_PRIV_DEC_LEN;
#endif    
	deca_m36_attach(&deca_config);
	deca_m36_ext_dec_enable((struct deca_device * )dev_get_by_id(HLD_DEV_TYPE_DECA, 0), &deca_config);
	deca_m36_ext_dec_enable2((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0), &deca_config);
#ifdef DVR_PVR_SUPPORT
    deca_m36_dvr_enable((struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
#endif    
#endif  
}


static void snd_dev_attach(void)
{
#ifndef _BUILD_OTA_E_
	 struct snd_feature_config snd_config;
	 MEMSET(&snd_config, 0, sizeof(struct snd_feature_config));    
	snd_config.output_config.mute_num = g_cfg_para.sys_mute_gpio;
	snd_config.output_config.mute_polar 	= g_cfg_para.sys_mute_polar;
	snd_config.output_config.dac_precision = 24;
	snd_config.output_config.dac_format 	= CODEC_I2S;
	snd_config.output_config.is_ext_dac 	= 0;
	snd_config.output_config.ext_mute_mode = g_cfg_para.ext_mute_mode;	
	snd_config.support_spdif_mute = 1;
    	snd_config.output_config.chip_type_config = g_cfg_para.snd_output_chip_type; //QFP
	snd_config.support_desc = g_cfg_para.audio_description_support;
    snd_config.conti_clk_while_ch_chg = 1;//effect the 3503 beep tone voice

#ifdef SUPPORT_AUDIO_DELAY
    snd_config.support_desc |= SND_SUPPORT_DDPLUS;
#endif

#if(defined(_SUPPORT_64M_MEM)&& defined(AD_MV_SHARE_BUFFER))   
    snd_config.ad_static_mem_flag = 1;//ad use the static memory
    snd_config.ad_static_mem_addr = __MM_AD_SND_MEM_ADDR;
    snd_config.ad_static_mem_size = __MM_AD_SND_MEM_LEN;
#else
    snd_config.ad_static_mem_flag = 0;//ad not use the static memory, ad malloc
#endif

    snd_m36g_attach(&snd_config);
#endif
}


void dmx_dev_attach(void)
{
    struct dmx_pre_attach_param dmx_pre_attach;
    struct dmx_feature_config dmx_config;

    MEMSET(&dmx_config, 0, sizeof(struct dmx_feature_config));
    MEMSET(&dmx_pre_attach, 0, sizeof(struct dmx_pre_attach_param));

	dmx_config.mm_map.dmx0_dma_buf = __MM_DMX_AVP_START_ADDR;
	dmx_config.mm_map.dmx1_dma_buf = __MM_DMX_REC_START_ADDR;
	dmx_config.mm_map.ts_audio_cb_len = (__MM_DMX_AUDIO_LEN/188);	
	dmx_config.mm_map.ts_video_cb_len = (__MM_DMX_VIDEO_LEN/188);
	dmx_config.mm_map.ts_pcr_cb_len = (__MM_DMX_PCR_LEN/188);
	dmx_config.mm_map.ts_ttx_cb_len = (__MM_DMX_DATA_LEN/188);
	dmx_config.mm_map.ts_common_cb_len = (__MM_DMX_SI_LEN/188); 
#ifndef _BUILD_OTA_E_
    dmx_config.mm_map.ts_united_cb_len = (__MM_DMX_AVP_LEN/188);
#else
    dmx_config.mm_map.ts_united_cb_len = (__MM_DMX_AVP_LEN/188);//(__MM_DMX_REC_LEN/188);
#endif
	dmx_config.mm_map.total_pid_filt_num = (4+__MM_DMX_SI_TOTAL_LEN/__MM_DMX_SI_LEN);
//	dmx_config.video_bitrate_detect = 1;
	dmx_config.sync_times = 3;
    dmx_config.clock_polarity = 0;
    dmx_config.sync_mode = 1;
    dmx_pre_attach.my_size = sizeof(struct dmx_pre_attach_param);
    dmx_pre_attach.dmx_ts_blk_buf1 = __MM_DMX_CPU_BLK_ADDR;
    dmx_pre_attach.dmx_ts_blk_buf2 = __MM_DMX_SEE_BLK_ADDR;
    dmx_pre_attach.dmx_ts_blk_buf1_size = __MM_DMX_BLK_BUF_LEN;
    dmx_pre_attach.dmx_ts_blk_buf2_size = __MM_DMX_BLK_BUF_LEN;


#ifndef FPGA_TEST
    dma_init();
#endif   
    dmx_m36f_pre_attach(&dmx_pre_attach);
    dmx_m36f_attach(&dmx_config);
#ifdef DVR_PVR_SUPPORT
    /*enable DVR feature. Only DVR proj can call this function, this function will enlarge dmx driver code size*/
    MEMSET(&dmx_config, 0, sizeof(struct dmx_feature_config));
    dmx_m36f_dvr_attach(&dmx_config);
#endif  

    if(g_cfg_para.front_end_num > 1)
        dmx_api_enhance_attach(&dmx_config);    
}


#ifdef DUAL_ENABLE
static void osd_m36f_dev_attach(void)
{
    extern void osd_m36g_attach(char *name, OSD_DRIVER_CONFIG *attach_config);    
	OSD_DRIVER_CONFIG osd_config;
	MEMSET((void *)&osd_config,0,sizeof(OSD_DRIVER_CONFIG));
	osd_config.u_mem_base = __MM_OSD_BK_ADDR1;
	osd_config.u_mem_size = __MM_OSD_LEN;
    osd_config.b_static_block = TRUE;

	osd_config.b_direct_draw = FALSE;
	osd_config.b_cacheable = TRUE;
	osd_config.b_vfilter_enable = TRUE;
	osd_config.b_p2nscale_in_normal_play = FALSE;
	osd_config.b_p2nscale_in_subtitle_play = TRUE;
	osd_config.u_dview_scale_ratio[OSD_PAL][0] = 384;
	osd_config.u_dview_scale_ratio[OSD_PAL][1] = 510;
	osd_config.u_dview_scale_ratio[OSD_NTSC][0] = 384;
	osd_config.u_dview_scale_ratio[OSD_NTSC][1] = 426;
	osd_config.af_par[0].id = 0;
	osd_config.af_par[0].vd = 1;
	osd_config.af_par[0].af = 1;
	osd_config.af_par[0].id = 1;
	osd_config.af_par[0].vd = 1;
	osd_config.af_par[0].af = 1;
	osd_m36g_attach("OSD_M36F_0", &osd_config);

    if(g_cfg_para.hd_subtitle_support)               //HD_SUBTITLE_SUPPORT
    {
	    osd_config.b_p2nscale_in_normal_play = TRUE;
	    osd_config.b_p2nscale_in_subtitle_play = TRUE;  
    }
    
    osd_config.u_mem_base = __MM_OSD_BK_ADDR2;
	osd_config.u_mem_size = __MM_OSD2_LEN;
	osd_config.b_p2nscale_in_normal_play = TRUE;
	osd_m36g_attach("OSD_M36F_1", &osd_config);

    //soc_printf("osd attach ok\n");
}
#endif

void ge_dev_attach(void)
{
    ge_layer_config_t osd_config[2];
    MEMSET((void *)&osd_config, 0, sizeof(osd_config));

    osd_config[0].mem_base = __MM_OSD_BK_ADDR1;
    osd_config[0].mem_size = __MM_OSD1_LEN;
    osd_config[0].b_direct_draw = FALSE;
    osd_config[0].b_cacheable = FALSE;
    osd_config[0].b_scale_filter_enable = TRUE;
    osd_config[0].b_p2nscale_in_normal_play = FALSE;
    osd_config[0].b_p2nscale_in_subtitle_play = TRUE;

    osd_config[0].hw_layer_id = GMA_HW_LAYER0;
    osd_config[0].color_format = GE_PF_ARGB8888;
    osd_config[0].pixel_pitch = OSD_MAX_WIDTH;
    osd_config[0].width = OSD_MAX_WIDTH;
    osd_config[0].height = OSD_MAX_HEIGHT;
    osd_config[0].b_newfeature = TRUE;

//#if (defined(_CAS9_CA_ENABLE_) || defined(_VMX_CA_ENABLE_))
//    osd_config[1].mem_base = __MM_OSD_BK_ADDR2_MAIN;
//#else
    osd_config[1].mem_base = __MM_OSD_BK_ADDR2_MAIN;//__MM_OSD_BK_ADDR2;
//#endif
    osd_config[1].mem_size = __MM_OSD2_LEN;
    osd_config[1].b_direct_draw = FALSE;
    osd_config[1].b_cacheable = FALSE;
    osd_config[1].b_scale_filter_enable = TRUE;
    osd_config[1].b_p2nscale_in_normal_play = TRUE;
    osd_config[1].b_p2nscale_in_subtitle_play = TRUE;

    osd_config[1].hw_layer_id = GMA_HW_LAYER1;
    osd_config[1].color_format = GE_PF_CLUT8;
    osd_config[1].pixel_pitch = 1920;
    osd_config[1].width = 1920;
    osd_config[1].height = 1080;

    ge_m36f_attach(osd_config, ARRAY_SIZE(osd_config));
    struct ge_device* ge_dev = (struct ge_device* )dev_get_by_id(HLD_DEV_TYPE_GE, 0);

#ifndef SEE_CPU
    // #define GE_SIMULATE_OSD
    osd_dev_api_attach(ge_dev, GMA_HW_LAYER0);
    osd_dev_api_attach(ge_dev, GMA_HW_LAYER1);
#endif

#ifdef DUAL_ENABLE
    osd_m36f_dev_attach();
#endif
};

void vpo_dev_attach(void)
{
	struct vp_feature_config vp_config;
	struct tve_feature_config tve_config;
	struct tve_feature_config sd_tve_config;
	UINT32 otp_tve_fs_value  = 0;

	MEMSET((void *)&tve_config,0,sizeof(struct tve_feature_config));
	MEMSET((void *)&sd_tve_config,0,sizeof(struct tve_feature_config));
	MEMSET((void *)&vp_config,0,sizeof(struct vp_feature_config));

	vp_config.b_avoid_mosaic_by_freez_scr = FALSE;    /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
#if 0 //(defined(_MHEG5_V20_ENABLE_))   
	vp_config.b_mheg5enable = TRUE;                  /* macro  _MHEG5_ENABLE_ */
#else
    vp_config.b_mheg5enable = FALSE;                  /* macro  _MHEG5_ENABLE_ */
#endif
	vp_config.b_osd_mulit_layer = FALSE;                  /* macro OSD_MULTI_LAYER */
	vp_config.b_osd_mulit_layer = FALSE;            /* macro VIDEO_OVERSHOOT_SOLUTION */


    vp_config.b_p2ndisable_maf= FALSE;

	vp_config.b_support_extra_win = FALSE;                /* macro VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW */
	vp_config.b_adpcmenable= FALSE;                    /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE) */
	vp_config.p_mpget_mem_info = NULL;  
    vp_config.p_src_asp_ratio_change_call_back = g_cfg_para.scart_aspect_change;//api_Scart_Aspect_Switch;

    vp_config.bl_fireware_smoothly = g_cfg_para.vpo_active;
#ifdef CHANCHG_VIDEOTYPE_SUPPORT
#ifdef VFB_SUPPORT
    //b_config_still_picture_memroy means see to backup automatically,
    //but need too much buffer,VFB must use see to backup, others don't need.
    vp_config.b_config_still_picture_memroy = TRUE;
    vp_config.still_picture_memory_addr = (UINT32)(NULL);
    vp_config.still_picture_memory_len = 0x308000;
#endif
#endif    
    tve_config.config = YUV_SMPTE|TVE_TTX_BY_VBI|TVE_CC_BY_VBI;
#ifdef TVE_VDEC_PLUG_DETECT
    tve_config.config |= TVE_PLUG_DETECT_ENABLE;
#endif
    if(g_cfg_para.tve_full_current_mode)
        tve_config.config |= TVE_FULL_CUR_MODE;

#ifdef SUPPORT_AFD_WSS_OUTPUT
	tve_config.config |=TVE_WSS_BY_VBI;
#endif
	
    tve_config.config |= TVE_NEW_CONFIG_1;
    tve_config.tve_tbl_all = g_cfg_para.tve_tbl;
	m36g_vpo_attach(&vp_config, &tve_config);
    
#if defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    vcap_attach_t vcap_param;
    vcap_param.fb_addr = __MM_VCAP_FB_ADDR;
    vcap_param.fb_size = __MM_VCAP_FB_SIZE;
    m36g_vcap_attach(&vcap_param);
#endif

    sd_tve_config.config = YUV_SMPTE|TVE_TTX_BY_VBI|TVE_CC_BY_VBI;
    #ifdef SUPPORT_AFD_WSS_OUTPUT
    sd_tve_config.config |=TVE_WSS_BY_VBI;
    #endif
    sd_tve_config.tve_adjust = g_cfg_para.tve_adjust;
    sd_tve_config.tve_adjust_adv = g_cfg_para.tve_adjust_adv;
    m36g_vpo_sd_attach(&vp_config, &sd_tve_config);

    otp_init(NULL);
    otp_read(0xDF*4,(UINT8*)(&otp_tve_fs_value),4);
    otp_tve_fs_value = (otp_tve_fs_value>>6) & 0x0000000F;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),VPO_IO_OTP_SET_VDAC_FS,otp_tve_fs_value);
}

void vdec_dev_attach(void)
{
#ifndef _BUILD_OTA_E_
	/* !!!!!Note !!!!!
		If you copy this code to other project, please check feature configuration firstly
	     !!!!!Note !!!!!
	*/
	struct vdec_config_par vdec_config_par;

#ifdef FPGA_TEST
    return;
#endif

	MEMSET((void *)&vdec_config_par,0,sizeof(struct vdec_config_par));

	//vdec_config_par.mem_map.frm0_Y_size  = 0;
	vdec_config_par.mem_map.frm0_y_size  = __MM_FB_LEN;
	vdec_config_par.mem_map.frm0_c_size = 0;
	vdec_config_par.mem_map.frm1_y_size  = 0;
	vdec_config_par.mem_map.frm1_c_size = 0;
	vdec_config_par.mem_map.frm2_y_size = 0;
	vdec_config_par.mem_map.frm2_c_size = 0;

	vdec_config_par.mem_map.frm0_y_start_addr = __MM_FB_START_ADDR;
	vdec_config_par.mem_map.frm0_c_start_addr = 0;
	vdec_config_par.mem_map.frm1_y_start_addr = 0;
	vdec_config_par.mem_map.frm1_c_start_addr = 0;
	vdec_config_par.mem_map.frm2_y_start_addr = 0;
	vdec_config_par.mem_map.frm2_c_start_addr = 0;


	vdec_config_par.mem_map.dvw_frm_size = 0;
	vdec_config_par.mem_map.dvw_frm_start_addr = 0;

	vdec_config_par.mem_map.maf_size = __MM_MAF_LEN;
	vdec_config_par.mem_map.maf_start_addr = __MM_MAF_START_ADDR;

	vdec_config_par.mem_map.vbv_size = ((__MM_VBV_LEN-4)&0xFFFFFF00);
	vdec_config_par.mem_map.vbv_start_addr = ((__MM_VBV_START_ADDR&0xfffffff)|0x80000000);
	vdec_config_par.mem_map.vbv_end_addr = ((vdec_config_par.mem_map.vbv_start_addr) +  vdec_config_par.mem_map.vbv_size - 1);

	vdec_config_par.user_data_parsing = TRUE; /* macro USER_DATA_PARSING */
    vdec_config_par.dtg_afd_parsing = g_cfg_para.afd_scale_support;     //SUPPORT_AFD_SCALE
	vdec_config_par.drop_freerun_pic_before_firstpic_show = FALSE; /* macro VDEC27_DROP_FREERUN_BEFORE_FIRSTSHOW */
	vdec_config_par.reset_hw_directly_when_stop = TRUE;  /* macro VDEC27_STOP_REST_HW_DIRECTLY*/
	vdec_config_par.show_hd_service = FALSE;      /* macro SUPPORT_SHOW_HD_SERVICE */
	vdec_config_par.still_frm_in_cc = FALSE;          /* macro STILL_FRAME_IN_CC */
	vdec_config_par.not_show_mosaic = FALSE;      /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
	vdec_config_par.adpcm.adpcm_mode = FALSE;  /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE)*/
	vdec_config_par.adpcm.adpcm_ratio = 0;          
	vdec_config_par.sml_frm.sml_frm_mode = FALSE;  /* macro VDEC27_SML_FRM_ONOFF(VDEC27_SML_FRM_OFF: FALSE   VDEC27_SML_FRM_ON: TRUE)*/
	vdec_config_par.return_multi_freebuf = TRUE;        /* macro VDEC27_SUPPORT_RETURN_MULTI_FREEBUF */ 
	vdec_config_par.sml_frm.sml_frm_size = 0;           /* macro VDEC27_SML_FRM_SIZE*/
	vdec_config_par.lm_shiftthreshold = 2;                   /* macro VDEC27_LM_SHIFTTHRESHOLD*/
	vdec_config_par.vp_init_phase = 0;                        /* macro DEFAULT_MP_FILTER_ENABLE*/
	vdec_config_par.preview_solution = VDEC27_PREVIEW_DE_SCALE; /* macro VDEC27_PREVIEW_SOLUTION(VDEC27_PREVIEW_VE_SCALE or VDEC27_PREVIEW_DE_SCALE)*/
	vdec_m36_attach(&vdec_config_par);

#ifdef DVR_PVR_SUPPORT
	struct vdec_device *vdec_config_dev = (struct vdec_device *)dev_get_by_name("DECV_S3601_0");    
	vdec_enable_advance_play(vdec_config_dev);
#endif   

#ifdef HDTV_SUPPORT
    avc_attach();
#endif
#ifdef SUPPORT_AVS_DECODER
	avs_config();
#endif
#endif
}

#ifndef _AUI_
extern UINT32 ap_get_osd_scale_param(enum tvsystem eTVMode, INT32 nScreenWidth);
#endif

static void ttx_subt_attach(void)
{
#if	(TTX_ON == 1)

	struct vbi_config_par vbi_config;
	MEMSET(&vbi_config,0,sizeof(struct vbi_config_par));

    vbi_config.ttx_by_vbi = TRUE;
	vbi_config.cc_by_vbi = FALSE;
    vbi_config.vps_by_vbi = FALSE;
	vbi_config.wss_by_vbi = FALSE;
	
	vbi_config.hamming_24_16_enable = FALSE;
	vbi_config.hamming_8_4_enable = FALSE;
    vbi_config.erase_unknown_packet = TRUE;

    if(g_cfg_para.ttx_packet_26_support)   //SUPPORT_PACKET_26
    {
        vbi_config.parse_packet26_enable = TRUE;
        vbi_config.mem_map.p26_data_buf_start_addr = __MM_TTX_P26_DATA_BUF_ADDR;
        vbi_config.mem_map.p26_data_buf_size = __MM_TTX_P26_DATA_LEN;
        vbi_config.mem_map.p26_nation_buf_start_addr = __MM_TTX_P26_NATION_BUF_ADDR;
        vbi_config.mem_map.p26_nation_buf_size = __MM_TTX_P26_NATION_LEN;
    }
    if(g_cfg_para.ttx_sub_page)          //TTX_SUB_PAGE
    {
        vbi_config.ttx_sub_page = TRUE;
        vbi_config.mem_map.sub_page_start_addr = __MM_TTX_SUB_PAGE_BUF_ADDR;
        vbi_config.mem_map.sub_page_size = __MM_TTX_SUB_PAGE_LEN;
    }


	vbi_config.user_fast_text = FALSE;	
	vbi_config.mem_map.sbf_start_addr = __MM_TTX_BS_START_ADDR;
	vbi_config.mem_map.sbf_size = __MM_TTX_BS_LEN;
	vbi_config.mem_map.pbf_start_addr = __MM_TTX_PB_START_ADDR;
	vbi_attach(&vbi_config);

	struct vbi_device *vbi_config_dev = NULL;
	vbi_config_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI,0);	
	vbi_hld_enable_ttx_by_osd(vbi_config_dev);
#endif

#if	(SUBTITLE_ON == 1)
#ifdef SUBT_FUNC_POINTER_ENABLE
	lib_subt_init();
#endif

	struct sdec_feature_config cfg_param;
	MEMSET(&cfg_param, 0, sizeof(struct sdec_feature_config));
	cfg_param.temp_buf_len = 0xa000;//(8*1024);  // > segment len 	
	cfg_param.temp_buf = NULL;
  cfg_param.bs_hdr_buf_len = 1000;
	cfg_param.bs_hdr_buf = 0;
	cfg_param.bs_buf = (UINT8*)__MM_SUB_BS_START_ADDR;
	cfg_param.bs_buf_len = __MM_SUB_BS_LEN;
	cfg_param.pixel_buf = (UINT8*)__MM_SUB_PB_START_ADDR;

//	#ifdef OSD_BLOCKLINK_SUPPORT
//	cfg_param.pixel_buf_len = 0;
//	#else
//	cfg_param.pixel_buf_len = __MM_SUB_PB_LEN;//(100*1024)
//	#endif

	cfg_param.tsk_qtm = 2;//10;	//fix BUG05435
	cfg_param.tsk_pri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;//OSAL_PRI_HIGH
	cfg_param.transparent_color = OSD_TRANSPARENT_COLOR;

//	#ifdef OSD_BLOCKLINK_SUPPORT
#ifdef DUAL_ENABLE
    cfg_param.sdec_hw_buf = (UINT8*)__MM_SUB_HW_DATA_ADDR;
    cfg_param.sdec_hw_buf_len = __MM_SUB_HW_DATA_LEN;
#else	
	cfg_param.subt_draw_pixel = osd_subt_draw_pixel;
	cfg_param.region_is_created = osd_region_is_created;
	cfg_param.subt_create_region = osd_subt_create_region;
	cfg_param.subt_region_show = osd_subt_region_show;
	cfg_param.subt_delete_region = osd_subt_delete_region;
#endif    
//	#else
//	cfg_param.subt_draw_pixelmap = osd_subt_draw_pixelmap;
//	#endif
    
	sdec_m33_attach(&cfg_param);
	
//	#ifdef OSD_BLOCKLINK_SUPPORT
	//subt_disply_bl_init((struct sdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC));
	subt_disply_bl_init((struct sdec_device *)dev_get_by_name("SDEC_M3327_0"));
//	#else
//	subt_disply_init((struct sdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC));
//	#endif
#endif

#ifdef ATSC_SUBTITLE_SUPPORT
	struct atsc_subt_config_par atsc_subt_par;
	struct atsc_subt_config_par* patsc_subt_par=&atsc_subt_par;
	memset(&atsc_subt_par,0x00,sizeof(struct atsc_subt_config_par));

	patsc_subt_par->bs_buf_len=__MM_DCII_SUB_BS_LEN;
	patsc_subt_par->bs_buf_addr=(UINT8*)__MM_DCII_SUB_BS_START_ADDR;	
	patsc_subt_par->sec_buf_len=__MM_SUBT_ATSC_SEC_LEN;
	patsc_subt_par->sec_buf_addr=(UINT8*)__MM_SUBT_ATSC_SEC_ADDR;
	patsc_subt_par->outline_thickness_from_stream=FALSE; //TRUE;
	patsc_subt_par->drop_shadow_right_from_stream=FALSE;
	patsc_subt_par->drop_shadow_bottom_from_stream=FALSE;
	patsc_subt_par->outline_thickness=2;
	patsc_subt_par->drop_shadow_right=2;
	patsc_subt_par->drop_shadow_bottom=2;
	lib_subt_atsc_attach(patsc_subt_par);
#endif

#ifndef _AUI_
//vicky20101229 dbg
#if (CC_ON==1)
    #include <api/libclosecaption/closecaption_osd.h> //vicky20110128
    #include <api/libclosecaption/atsc_cc_init.h> 
    struct atsc_cc_config_par cc_param;
    cc_param.pb_buf_addr=(UINT8*)__MM_ATSC_CC_PB_START_ADDR;    
    cc_param.pb_buf_len=__MM_ATSC_CC_PB_LEN;
    cc_param.bs_buf_addr=(UINT8*)__MM_ATSC_CC_BS_START_ADDR;
    cc_param.bs_buf_len=__MM_ATSC_CC_BS_LEN;
    cc_param.osd_layer_id=1;
    cc_param.cc_width=OSD_CC_WIDTH;
    cc_param.cc_height=OSD_CC_HEIGHT;
    cc_param.w_polling_time=500;      // 500ms
    cc_param.b_ccscan=4;         // 4*500ms ->2sec
    cc_param.b_dtvccscan=4;      // 4*500ms ->2sec
    cc_param.osd_get_scale_para = ap_get_osd_scale_param;
    lib_atsc_cc_attach(&cc_param);  //vicky20110224

	vbi_cc_init();
#endif
#endif

#if (ISDBT_CC ==1)
    #include <hld/sdec/sdec.h>
	struct sdec_feature_config cc_cfg_param;	
	// fill data to [cc_cfg_param]
	MEMSET(&cc_cfg_param, 0, sizeof(struct sdec_feature_config)); //set default value of  mem as 0

	cc_cfg_param.bs_hdr_buf = 0;
	cc_cfg_param.bs_hdr_buf_len = 200;	//size under checking
	
	cc_cfg_param.bs_buf =(UINT8*)__MM_ISDBTCC_BS_START_ADDR;
	cc_cfg_param.bs_buf_len = 184*cc_cfg_param.bs_hdr_buf_len;	
	
	cc_cfg_param.tsk_qtm = 10;
	cc_cfg_param.tsk_pri = OSAL_PRI_NORMAL	;
	cc_cfg_param.transparent_color = OSD_TRANSPARENT_COLOR;
	
    	lib_isdbtcc_init();
	isdbtcc_dec_attach(&cc_cfg_param);	
	isdbtcc_disply_init((struct sdec_device *)dev_get_by_name("ISDBT_CC_0"));	
#endif

}


void smc_attach(void)
{
#ifdef MULTI_CAS    
    struct smc_dev_config smc_config;
	static UINT32 init_clk = 6000000;    
	OTP_CONFIG my_otp_cfg;
    
	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));    
	MEMSET((void *)(&my_otp_cfg), 0, sizeof(OTP_CONFIG));
	otp_init(&my_otp_cfg);


	smc_config.init_clk_trigger= 1;
	smc_config.init_clk_number = 1;
	smc_config.force_tx_rx_trigger = 1;
	//smc_config.apd_disable_trigger = 1;
	//smc_config.disable_pps = 1;
    if(g_cfg_para.cas_type == CAS_CONAX)
    {
    	smc_config.def_etu_trigger = 1; 
    	smc_config.default_etu = 372;
       #ifdef CAS9_V6
       smc_config.warm_reset_trigger = 0;//v6 need all reset as cold reset //sync from M3281
       #else 
       smc_config.warm_reset_trigger = 1;
       #endif
    	smc_config.force_tx_rx_cmd = 0xdd;//CONAX command start.
    	smc_config.force_tx_rx_cmd_len = 5; //CONAX command length.	
    	init_clk = 3600000;
    	smc_config.invert_detect=0;//1;        
    }
	smc_config.init_clk_array = &init_clk;
	smc_config.class_selection_supported = 1;
	smc_config.board_supported_class = BOARD_SUPPORT_CLASS_A|BOARD_SUPPORT_CLASS_B;
	smc_config.class_select =  g_cfg_para.class_select; //board_class_select;

    if(!g_cfg_para.cas9_support)
    {
        smc_config.disable_pps = 1;
        smc_config.apd_disable_trigger = 1;
        *((volatile UINT32 *)0xb8000080) = *((volatile UINT32 *)0xb8000080) | (0x01 <<20);
        osal_delay(100);
        *((volatile UINT32 *)0xb8000080) = *((volatile UINT32 *)0xb8000080) &(~ (0x01 <<20));
    }
    smc_config.invert_detect = g_cfg_para.invert_detect;
#ifdef SMC1_SUPPORT
    smc_dev_attach(0, &smc_config);
#elif defined(SMC2_SUPPORT)
    smc_dev_attach(1, &smc_config);
#endif
#endif    

#if defined(_VMX_CA_ENABLE_) || defined(SUPPORT_VMX_STD)
#ifndef _BUILD_OTA_E_
	struct smc_dev_config smc_config;
	static UINT32 init_clk = 4800000;

	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));
	smc_config.init_clk_trigger= 1;
	smc_config.init_clk_number = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
	smc_config.disable_pps = 0; 
	smc_config.init_clk_array = &init_clk;        
	smc_config.invert_detect = g_cfg_para.invert_detect;
	smc_config.default_etu = 372;
	smc_config.def_etu_trigger = 1;

#ifdef SMC1_SUPPORT
	smc_dev_attach(0, &smc_config);
#elif defined(SMC2_SUPPORT)
	smc_dev_attach(1, &smc_config);
#endif
#endif
#endif

}

void board_advance_init(advance_cfg_para* cfg_para)
{
	if(NULL != cfg_para)
	{
		MEMCPY(&g_cfg_para, cfg_para, sizeof(advance_cfg_para));
	}

	deca_dev_attach();
	snd_dev_attach();
	dmx_dev_attach();
    
#ifdef CI_SUPPORT
#if (SYS_CIC_MODULE == CIMAX_27)
	struct cic_config_param cic_param = {0x86, I2C_TYPE_GPIO};
	cic_cimax_attach(&cic_param);
#else
	cic_m3602_attach();
#endif
#endif

	ge_dev_attach(); //waitting dbg!!!
	vpo_dev_attach();	
	vdec_dev_attach();
	ttx_subt_attach();
	smc_attach();
    
#ifdef CI_PLUS_SUPPORT
	otp_init(NULL);
#endif

#ifdef HDTV_SUPPORT
	struct config_hdmi_parm hdmi_param; 
	get_hdmi_config(&hdmi_param);
	hdmi_tx_attach(&hdmi_param);
#endif

#ifdef ALICAST_SUPPORT
	//get_hdmi20_config(&g_H2KeySets);
	//hdcp2_load_KeySets(&g_H2KeySets);
#endif

	if(g_cfg_para.ce_api_enabled)
	{
		ce_api_attach();
	}
}



