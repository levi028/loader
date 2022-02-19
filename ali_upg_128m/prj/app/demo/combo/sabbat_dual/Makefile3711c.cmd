#
# -= Makefile for application compile =-
#
# Note:
# . This is the 1st part of two makefile components;
#   Please refer "Makefile.lin" for the other part.
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as application's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Take application as a module, define the name in MOD;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#

# Destination to the root of software tree
ROOT = ../../../../..

include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

ifdef _AUI_
ROOT_PRJ = ${BASE_DIR}/prj/app/demo/combo/aui_demo
else
ROOT_PRJ = ${BASE_DIR}/prj/app/demo/combo/sabbat_dual
endif

MAKEFILE = Makefile3711c

EXTRA_FLAGS=
ifdef _WARNING_TO_ERROR
EXTRA_FLAGS=-Werror
endif

CONAX_AP_DIR =
ifdef _CAS7_CA_ENABLE_
CONAX_AP_DIR = conax_ap7
endif
CA_AP_DIR=conax_ap

ifdef _BC_CA_ENABLE_
CA_AP_DIR = bc_ap
endif

ifdef _SMI_ENABLE_
SMI_AP_DIR = smi_ap
endif

ifdef _GEN_CA_ENABLE_
CA_AP_DIR = gen_ap
endif

ifdef _C1800A_CA_ENABLE_
CA_AP_DIR = ird_ap
endif

ifdef _C1800A_LOADERCORE_ENABLE_
LOADER_SPI_DIR = ird_ap/LoaderCore/src/spi
LOADER_BL_DIR  = ird_ap/LoaderCore/src/ird_blcore
VD_DIR = ird_ap/vd
endif


# Module Name
MOD = SABBAT_DEMO
BL_RMOD = LIB_BOOTUPG

# Link Script File

ifdef _BUILD_OTA_E_
	ifdef _BUILD_USB_LOADER_RET2GAC
		LSCR= ldscript_3711c_usb_ret2gca.ld
	else
	  LSCR = ldscript_3711c.ld
  endif
else
LSCR = ldscript_3711c.ld
endif

# Default is retailer
SEE_LSCR = see_ldscript_m3711c.ld

ifdef _CAS9_VSC_ENABLE_
	SEE_LSCR = see_ldscript_m3711c_VSC.ld
else
	ifdef _USE_64M_MEM_
		SEE_LSCR = see_ldscript_m3711c_64M.ld
	endif
	ifdef _SUPPORT_256M_MEM
		SEE_LSCR = see_ldscript_m3711c_256M.ld
	endif
	ifdef _CAS9_CA_ENABLE_
		SEE_LSCR = see_ldscript_m3711c.ld
	endif
	ifdef _GEN_CA_ENABLE_
		SEE_LSCR = see_ldscript_m3711c.ld
	endif
	ifdef _BC_CA_ENABLE_
		SEE_LSCR = see_ldscript_m3711c.ld
	endif
	ifdef _C1800A_CA_ENABLE_
		SEE_LSCR = see_ldscript_m3711c.ld
	endif
	ifdef _C0700A_VSC_ENABLE_
		SEE_LSCR = see_ldscript_m3711c_c0700a_vsc.ld
	endif
endif
# List of source files
ifdef _AUI_
SRCS += aui_root.c hdcp_des.c
ifdef _NOCS3_CSD_SEC_TEST_
SRCS += csd_sec_cert.c
endif
else
ASMS =
SRCS = control.c ctrl_task_init.c ctrl_util.c ap_dynamic_pid.c ctrl_pip.c ctrl_debug.c ctrl_network.c \
       ctrl_play_channel.c ctrl_cas_ap.c ap_ctrl_mheg5.c ap_ctrl_ci.c ctrl_key_proc.c \
       ap_ctrl_ttx.c ap_ctrl_dtg.c ap_ctrl_display.c ap_ctrl_time.c ctrl_msg_proc.c \
       ap_ctrl_dual_ci.c ap_ctrl_ci_plus.c ctrl_popup_msg.c ctrl_play_si_proc.c ctrl_preplay_channel.c \
       cec_link.c game_othello.c game_sudoku.c key.c lib_ash.c menus_root.c \
       mobile_input.c osd_rsc.c ota_ctrl.c power.c \
       pvr_ctrl_basic.c pvr_ctrl_cnx9_v6.c pvr_ctrl_parent_lock.c pvr_ctrl_ca.c pvr_ctrl_timer.c \
       pvr_ctrl_record_internal.c pvr_ctrl_record.c pvr_ctrl_play.c pvr_ctrl_tms.c pvr_ctrl_ts_route.c pvr_control.c\
       system_init.c root.c satcodx.c hdcp_des.c \
       nim_test.c sfu_test.c si_monitor_ap.c si_auto_update_tp.c time_zone_name.c ui_debug.c win_ali_demo.c \
       win_audio.c \
       win_chan_rename.c win_chunk_info.c win_ci_common.c win_ci_dlg.c \
       win_ci_info.c win_ci_slots.c win_com_list_callback.c win_com_parental.c win_com_frontend.c \
       win_com.c win_com_list.c \
       win_com_popup_inner.c win_com_popup_cas.c win_com_popup.c win_diag_i2c.c win_diag_register.c win_diagnosis.c \
       win_display_set.c win_factory_set.c win_factory_test.c \
       win_favgroup_list.c win_filelist_inner.c win_filelist_display.c win_filelist_act_proc.c win_filelist_api.c win_filelist.c \
       win_find.c win_game_com.c win_game_othello.c \
       win_game_sudoku.c win_hdmi_factory.c win_hdtv_mode.c \
       win_imagepreview.c win_imageslide.c win_language.c \
       win_mainmenu.c win_mainmenu_submenu.c \
       win_misc_setting.c win_mpeg_player_audio_track.c win_mp_subtitle.c \
       win_multiview.c win_multiview_ext.c win_multiviewset.c\
       win_mpeg_player.c win_mpspectrum.c win_mpvolume.c win_mpzoom.c \
       win_mute.c win_net_local_seting.c win_net_upg.c \
       win_osd_set.c win_ota_set.c win_otaupg.c win_otaupg_private.c \
       win_password.c win_multiview_password.c win_pause.c \
       win_playlist.c win_playlist_setup.c \
       win_prog_detail.c \
       win_pvr_ctrl_bar_basic.c win_pvr_ctrl_bar_key.c win_pvr_ctrl_bar_draw.c win_pvr_ctrl_bar.c \
       win_pvr_hdd_format.c win_pvr_hint.c \
       win_pvr_jumpstep.c Win_pvr_record_manager.c win_pvr_tms.c \
       Win_rs232upg.c win_search_ext_function.c win_signal.c \
       win_signalstatus.c win_sleeptimer.c win_submenu.c win_subtitle.c \
       win_time.c win_time_ciplus.c win_timer_list.c win_timer_msg.c win_timer_set.c \
       win_tvsys.c \
       win_usbupg.c win_mp_move_file.c win_pvr_setting.c \
       storage_control.c win_storage_setting.c media_control_es.c media_control.c win_no_menu.c \
       win_disk_info.c win_disk_operate.c win_copying.c win_zoom.c \
       win_automatic_standby.c win_multifeed.c win_hdmi_certification.c mp_subtitle_osd.c \
       disk_manager.c win_audio_description.c win_mheg5_v20.c \
       win_usblogo.c win_diag_energy.c win_chapter.c win_security_upg.c \
       win_ci_authinfo.c ad_pic_file.c ad_pic_logo.c ad_pic_img.c ad_pic_src.c win_av_delay.c win_PALNTSC.c  win_combo_ota.c win_adc.c win_nimreg.c\
       win_welcom.c win_closedcaption.c win_calen_time.c win_com_popup_smsg.c  win_subtitle_setting.c \
       nit_info.c scpi.c scpi_cmd_drv.c scpi_cmd.c lwip_ping.c  win_cc.c \
       win_mainmenu_submenu_data.c beep_mp2.c win_net_choose.c 
ifdef _DVBS_ENABLE_
SRCS += win_antenna_connect.c win_antenna_set_fixed.c win_unicable_setting.c \
       win_antenna_set_motor.c win_motor_limitsetup.c win_motor_setting.c \
       win_sat_edit.c win_sat_list.c win_satsearch_set.c \
       win_tp_edit.c win_ascan_edit.c win_tp_list.c win_tpsearch_set.c win_parental_old.c 
else
SRCS += win_parental.c
endif

ifdef _DVBC_ENABLE_
SRCS += win_quickscan.c win_country_net.c win_countryband.c
ifdef _RD_DEBUG_
#SRCS += win_dvbc_nim_debug.c
endif
endif

ifdef _DVBT_ENABLE_
SRCS +=  win_auto_scan.c win_chan_scan.c win_countryband_t.c win_lcn.c   prog_find.c  t2malloc.c \
		 win_automatic_upgrade_setting.c
endif

ifdef _ISDBT_ENABLE_
SRCS +=  win_auto_scan.c win_chan_scan.c win_countryband_t.c win_lcn.c
endif

ifdef _PLAY_ENABLE_
	SRCS += win_play.c
endif

ifdef _INTEST_ENABLE_
	SRCS += win_internal_test.c
endif

ifdef _YOUTUBE_ENABLE
    SRCS += win_youtube_category.c win_youtube_keyword_input.c win_youtube_play.c
endif
ifdef _WIFI_ENABLE_
SRCS += win_wifi_manager.c win_wifi_ap_select.c win_wifi_input_password.c 
endif

ifdef _USB3G_SUPPORT_
SRCS += win_usb3g_dongle.c win_usb3g_dongle_dial.c
export LIB_USB3G_MODULE = LIB_USB3G
endif

ifdef _HILINK_SUPPORT_
SRCS += win_hilink.c win_hilink_profile.c win_hilink_traffic.c win_hilink_automode.c
export LIB_HILINK_MODULE = LIB_HILINK
endif

ifdef _MAC_TEST
	SRCS +=iol_test_mainmenu.c iol_test_mainmenu_submenu.c iol_test_submenu.c iol_test_submenu_system.c\
	win_mac_rx_config.c win_mac_rxtx_state.c win_mac_submenu.c win_mac_test.c win_mac_tx_config.c
endif

ifdef _WIFI_DIRECT_ENABLE_
	SRCS += win_wifi_direct_setting.c
endif

ifdef _WIFI_DISPLAY_ENABLE_
	SRCS += win_alicast.c
endif
ifdef _VPN_ENABLE_
	SRCS += win_vpn_setting.c
endif
endif

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.

# ==================================================

SMOD =
export SMOD_LIB =
NIM_S2_MOD =
NIM_S2_LIB =
NIM_T_MOD =
NIM_T_LIB =
NIM_S3811_MOD =
NIM_S3811_LIB =

NIM_S2_MOD = LLD_NIM_S2
NIM_S2_LIB = libnim_s2.a
SMOD += ${NIM_S2_MOD}
ifdef _DVBS_ENABLE_
SMOD_LIB += ${NIM_S2_LIB}
endif
ifdef _NAND_ENABLE_
SMOD_LIB += libnand.a
endif

NIM_T_MOD = LLD_NIM_M3101
NIM_T_LIB = libnim_t.a
SMOD += ${NIM_T_MOD}
ifdef _DVBT_ENABLE_
SMOD_LIB += ${NIM_T_LIB}
endif

# S3821 supports _DVBT2_ENABLE_
NIM_S3821_MOD = LLD_NIM_S3821
NIM_S3821_LIB = libnim_s3821.a
SMOD += ${NIM_S3821_MOD}
ifdef _DVBT2_ENABLE_
SMOD_LIB += ${NIM_S3821_LIB}
endif


NIM_S3281_C_MOD = LLD_NIM_S3281_C
NIM_S3281_C_LIB = libnim_s3281_c.a
SMOD += ${NIM_S3281_C_MOD}
ifdef _DVBC_ENABLE_
SMOD_LIB += ${NIM_S3281_C_LIB}
endif
ifdef DUAL_ENABLE
SECLIB_MAIN = libmsec
else
SECLIB_MAIN = libsec_single
endif
# ==================================================


# List of dependent modules

# Libplus pvr+fs modules
#by keven
ifdef _BUILD_USB_LOADER_RET2GAC
PMOD = LIB_DOSFS LIB_VFS  \
       VERSION_INFO_PLUS LIB_STDIO 
       
PPMOD = PLUGIN_ALL
OPMOD = 
IMGMOD = 
else
ifdef _BUILD_UPG_LOADER_
PMOD = LIB_DOSFS LIB_VFS \
       VERSION_INFO_PLUS LIB_STDIO

PPMOD = PLUGIN_ALL
OPMOD = LIB_PVR3_ENG
IMGMOD = LIB_MP
else
ifndef _BUILD_OTA_E_
PMOD = LIB_DOSFS LIB_VFS \
       VERSION_INFO_PLUS LIB_STDIO LIB_PVR3_CRYPTO

PPMOD = PLUGIN_ALL
OPMOD = LIB_PVR3_ENG
IMGMOD = LIB_MP
else
ifdef _BUILD_USB_LOADER_GEN
PMOD = LIB_DOSFS LIB_VFS VERSION_INFO_PLUS LIB_STDIO
PPMOD = PLUGIN_ALL
IMGMOD =
OPMOD = 
else
ifdef _C1800A_LOADERCORE_ENABLE_
PMOD = LIB_DOSFS LIB_VFS \
       VERSION_INFO_PLUS LIB_STDIO 
       
PPMOD = PLUGIN_ALL
IMGMOD = 
else
PMOD = 
PPMOD =
IMGMOD =
endif
endif
endif
endif
endif

ifdef _WIFI_ENABLE_
  export LMOD_BUS_USB_MODULE = BUS_USB_LINUX 
   export LIB_WIFI_MODULE = HLD_WIFI LIB_WIFI
else
  export LIB_WIFI_MODULE = HLD_WIFI
  export LMOD_BUS_USB_MODULE = BUS_USB_LINUX 
endif

ifndef _BUILD_OTA_E_
export CORE_MODULE = LLD_VP_M36G LLD_SND_M36G LLD_DECA_M36F
else
export CORE_MODULE = LLD_VP_M36G
endif

# OS library
ifndef _BUILD_OTA_E_
OSMOD = ARCH_M63 OS_TDS3 LLD_PMU_M36 \
		${CORE_MODULE} \
		BUS_EROM_M36\
		VERSION_INFO \
		LLD_DECV_M36 LLD_DECV_AVC LLD_DCEV_CFG \
		BUS_DPLL \
		BUS_PINMUX_M36 LLD_PAN_HWSCAN LIB_CIPLUS_CC \
		LIB_RSA LIB_NETCTL BUS_RTC_M36 LLD_SBM \
		LIB_ALI_PLAY LIB_LIVE555 LIB_UPNP LIB_STANDBY\
		LIB_WIFI_STORAGE LIB_SATIP_SERVER LIB_MEDIA_PROVIDER LIB_HTTP_MG_SERVER \
		LIB_SATIP_CLIENT_CORE LIB_JSON LIB_ALI_PLAY LLD_HHE \
		LLD_AVSYNC LLD_SBM 
#USB_3G_MDM

else
OSMOD = ARCH_M63 OS_TDS3 LLD_PMU_M36 \
		${CORE_MODULE} \
		LLD_PAN_HWSCAN \
		BUS_DPLL \
		BUS_RTC_M36 \
		VERSION_INFO \
		LIB_RSA
endif

ifndef DUAL_ENABLE
	OSMOD += LLD_TRIM
endif

ifdef _C0700A_VSC_ENABLE_
	OSMOD += LLD_C0700A_VSC
else
	OSMOD += LLD_VSC
endif

# Libcore library ( for ddk core )
ifndef _BUILD_OTA_E_
LMOD = LLD_GE_M36F \
        HLD_GE \
        LLD_CIC_M36 \
        LLD_DMX_M36F \
        BUS_DMA_M36F \
        BUS_TSI_M36 \
        BUS_TSG_M36 \
        BUS_DOG_M33 \
        BUS_OTP_M33 \
        $(LMOD_BUS_USB_MODULE) \
        LLD_NET_ETHTOE2 LIB_LWIP140 BUS_HDMI_M36  
        
else
#by keven
ifdef _BUILD_USB_LOADER_RET2GAC
LMOD = BUS_DMA_M36F \
         BUS_TSI_M36 \
         BUS_DOG_M33 \
         BUS_OTP_M33   
else 
LMOD = LLD_GE_M36F \
        LLD_DMX_M36F \
         BUS_DMA_M36F \
         BUS_TSI_M36 \
         BUS_DOG_M33 \
         BUS_OTP_M33 \
         HLD_GE BUS_HDMI_M36
endif
 
ifdef _C1800A_LOADERCORE_ENABLE_ 
LMOD += $(LMOD_BUS_USB_MODULE)
endif   
       
ifdef _BUILD_USB_LOADER_
LMOD += $(LMOD_BUS_USB_MODULE)
endif         
ifdef _BUILD_UPG_LOADER_
LMOD += $(LMOD_BUS_USB_MODULE)
endif  
ifdef _BUILD_USB_LOADER_GEN
LMOD += $(LMOD_BUS_USB_MODULE)
endif 
 
ifdef _BUILD_USB_LOADER_RET2GAC
LMOD += $(LMOD_BUS_USB_MODULE)
endif  
endif

# Release library
ifndef _BUILD_OTA_E_
RMOD = BUS_I2C_SCB BUS_I2C_GPIO BUS_SCI_UART BUS_CT1642 \
       BUS_IDE_M36_HD\
       HLD_BASE LIB_C \
       HLD_PAN  \
       HLD_SCART  \
       HLD_STO  \
       HLD_VP \
       HLD_OSD \
       HLD_DECV \
       HLD_AVSYNC \
       HLD_VENC \
       HLD_VBI \
       HLD_DMX \
       HLD_SND \
       HLD_DECA \
       HLD_SDEC \
       HLD_CIC  \
       HLD_RFM  \
       HLD_NIM   \
       HLD_SCART \
       HLD_SEC_BOOT \
       HLD_NET \
       LLD_IRC_M63 \
       LLD_SCART_AK4707 \
       LLD_SCART_MX9671 \
       LLD_PAN_FD650\
       LLD_STO_FLASH_SQI \
       LLD_VBI_M33 \
       LLD_RFM_MCBS373\
       LIB_NIM  \
       LIB_CHUNK LIB_LIB7ZIP \
       LIB_CHAR LIB_OSDO \
       LIB_DBC LIB_OTA HLD_SMC LLD_SMARTCARD \
       LIB_USBUPG LIB_VFS2 \
       LIB_PVR3_MGR LIB_TSG LIB_PVR3_API36 \
       LLD_NET_ENC28J60 LLD_NET_SMSC9220  \
       HLD_DSC HLD_CRYPTO HLD_CERT HLD_CF HLD_PVR\
       VERSION_INFO_APP LIB_PE LIB_LISTS LIB_CURL_S\
       LIB_CEC LIB_NIC\
       LIB_CLOSE_CAPTION \
       $(LIB_WIFI_MODULE) \
       LIB_PE_EXTERNAL_SUBTITLE LIB_GE_OSD $(LIB_USB3G_MODULE) $(LIB_HILINK_MODULE) LIB_GA 

ifndef _AUI_
# AUI projects default no need these modules
RMOD += LLD_ISDBTCC LIB_ISDBTCC \
	LIB_SUBT LLD_SDEC_SW LIB_TTX \
	LIB_XML2 \
	LIB_NETUPG LIB_CIPLUS \
        LIB_TSI3_CON_PSI LIB_TSI3_DESCRIPTOR LIB_TSI3_PARSER LIB_TSI3_PLUGIN_EIT \
        LIB_TSI3_ENGINE \
        LIB_TSI3_PLUGIN_SEARCH LIB_TSI3_PLUGIN_MONITOR LIB_TSI3_PLUGIN_TDT LIB_TSI3_UTILITY \
        LIB_EPG  LIB_DB4 LIB_UPG3  \
        LIB_PUB_ALL LIB_PUB_AS_ENGINE

endif

ifdef _MHEG5_V20_ENABLE_
ifdef _CIPLUS_CC_ENABLE_
# Need to add this module in CI Plus project
RMOD += UDI_MHEG_V20_CIPLUS_APP
endif
endif
ifdef _LIB_UPNP_ENABLE_		
# Notes: This module will be released by command dvb2ip_rel. 
	RMOD += UDI_PTHREAD
endif
ifdef _NOCS3_SUPPORT_
	RMOD += NOCS3_CSD NOCS3_SEC NOCS3_CERT
endif

ifdef _NAND_ENABLE_
	RMOD += LLD_STO_NAND
endif

ifdef _SMI_CF_ENABLE_
RMOD += LIB_SMK
endif

ifdef _C1800A_CA_ENABLE_
RMOD += HLD_CASI
endif

else

#by keven
ifdef _BUILD_USB_LOADER_RET2GAC
RMOD = BUS_I2C_SCB BUS_I2C_GPIO BUS_SCI_UART BUS_CT1642 \
       HLD_BASE LIB_C \
       HLD_PAN  \
       HLD_STO  \
       HLD_SEC_BOOT \
       LLD_PAN_FD650 \
       LLD_STO_FLASH_SQI \
       LIB_CHUNK LIB_LIB7ZIP  \
       HLD_DSC HLD_CRYPTO \
       VERSION_INFO_APP    
else      
RMOD = BUS_I2C_SCB BUS_I2C_GPIO BUS_SCI_UART BUS_CT1642 \
       BUS_IDE_M36_HD\
       HLD_BASE LIB_C \
       HLD_PAN  \
       HLD_SCART  \
       HLD_STO  \
       HLD_VP \
       HLD_OSD \
       HLD_DECV \
       HLD_AVSYNC \
       HLD_VENC \
       HLD_DMX \
       HLD_SND \
       HLD_DECA \
       HLD_CIC  \
       HLD_RFM  \
       HLD_NIM   \
       HLD_SEC_BOOT \
       LLD_IRC_M63 \
       LLD_SCART_AK4707 \
       LLD_SCART_MX9671 \
       LLD_PAN_FD650 \
       LLD_STO_FLASH_SQI \
       LLD_RFM_MCBS373\
       LIB_NIM \
       LIB_TSI3_PLUGIN_MONITOR \
       LIB_TSI3_ENGINE LIB_TSI3_PARSER LIB_TSI3_UTILITY LIB_TSI3_PLUGIN_TDT\
       LIB_CHUNK LIB_UPG3 LIB_LIB7ZIP \
       LIB_CHAR LIB_OSDO \
       LIB_DBC LIB_OTA \
       HLD_DSC HLD_CRYPTO HLD_PVR \
       LIB_PUB_ALL \
       VERSION_INFO_APP \
       LIB_CEC \
       LIB_GE_OSD
endif       
ifdef _BUILD_USB_LOADER_
	ifdef _BUILD_USB_LOADER_GEN
		RMOD +=   LIB_USBUPG LIB_VFS2 
	else
	 ifdef _BUILD_USB_LOADER_RET2GAC
	 RMOD +=   LIB_USBUPG LIB_VFS2  
   else
	 RMOD +=  HLD_VBI HLD_SDEC LLD_SDEC_SW LLD_VBI_M33 LIB_USBUPG LIB_VFS2 LIB_LISTS LIB_PE
	 endif
	endif
endif
endif      

ifdef _C1800A_LOADERCORE_ENABLE_
   RMOD += LIB_USBUPG LIB_VFS2 
endif
ifdef _DVBS_ENABLE_
RMOD += LLD_NIM_S2_TUNER LLD_NIM_S2_3501 LLD_NIM_S2_C3505 LLD_NIM_S2_C3501H LIB_PUB_AS_S LIB_DISEQC LLD_NIM_LNB
endif

ifdef _DVBT2_ENABLE_
RMOD += LLD_NIM_DVBT2
endif

ifdef _DVBT_ENABLE_
RMOD += LLD_NIM_DVBT LIB_PUB_AS_T
endif

ifdef _DVBC_ENABLE_
RMOD += LLD_NIM_C LIB_PUB_AS_C LLD_NIM_MXL603 LLD_NIM_TUN_TDA18250 LLD_NIM_C_TDA10025 LLD_NIM_C_ATMB2040 LLD_NIM_C_ATMB253
endif

RMOD += LLD_NIM_COMBO

ifdef _ALI_FSC_ENABLE_
RMOD += LLD_NIM_MXL_214C
endif

ifdef _ISDBT_ENABLE_
RMOD += LLD_NIM_ISDBT 
#RMOD += LLD_NIM_ISDBT LLD_ISDBTCC LIB_ISDBTCC
endif
SAT2IP_APP_DIR=
FSC_APP_DIR=


ifdef _SAT2IP_SERVER_ENABLE_
#RMOD += LIB_MEDIA_PROVIDER
SAT2IP_APP_DIR=sat2ip
endif

ifdef _SAT2IP_CLIENT_ENABLE_
RMOD += LIB_SATIP_CLIENT
SAT2IP_APP_DIR=sat2ip
endif

ifdef _ALI_FSC_ENABLE_
FSC_APP_DIR = fsc_ap
endif

ifdef DUAL_ENABLE
RMOD += DUAL_MODULES
endif

ifdef _C0700A_VSC_ENABLE_
RMOD += LIB_ABVCA
endif

ifdef _VPN_ENABLE_
RMOD += LIB_VPN
endif

RMOD += LIB_STANDBY

ifdef _NAND_ENABLE_
NAND_LIB = -lnand
else
NAND_LIB = 
endif

LINKER=link
ifdef _CAS9_VSC_ENABLE_
SEE_LINKER=see_link_sp
else
SEE_LINKER=see_link
endif
###############################################################
ifdef _BUILD_OTA_E_

ifdef _AUI_
SRCS = aui_root.c hdcp_des.c
else

ASMS =
SRCS = control.c ctrl_util.c osd_rsc.c power.c hdcp_des.c root.c key.c win_com_popup.c \
         win_otaupg.c win_ota_set.c ota_ctrl.c win_signal.c win_signalstatus.c  win_mute.c win_pause.c win_tvsys.c win_com.c \
         win_com_list.c ota_link.c ctrl_key_proc.c  ctrl_task_init.c  ctrl_msg_proc.c  ctrl_popup_msg.c \
         system_init.c ap_ctrl_display.c ap_ctrl_time.c win_com_popup_inner.c win_otaupg_private.c win_com_frontend.c
         
ifdef _BUILD_USB_LOADER_
SRCS += win_usbupg.c disk_manager.c ctrl_debug.c
endif
ifdef _DVBT_ENABLE_
SRCS += win_countryband_t.c
endif
ifdef _ISDBT_ENABLE_
SRCS += win_countryband_t.c
endif
ifdef _DVBC_ENABLE_
SRCS += win_countryband.c
endif
endif

LINKER=link_ota
SEE_LINKER=see_link_ota
endif

ifdef _BUILD_USB_LOADER_RET2GAC
SRCS = root_erom.c
endif
###############################################################

ifdef _BOOT_LOGO_ENABLE_
	LOGO_EN=_logo
else
	LOGO_EN=
endif

VSC_POSTFIX=
SCU_POSTFIX=
ifdef _CAS9_VSC_ENABLE_
	VSC_POSTFIX=_vsc
	ifdef _CAS9_SC_UPG_
		SCU_POSTFIX=_scu
	else
		SCU_POSTFIX=
	endif
else
	VSC_POSTFIX=
	SCU_POSTFIX=
endif

RELPROJ = ${BASE_DIR}
RELLIB = libstb

ifndef _BUILD_OTA_E_
	ifdef _CAS9_VSC_ENABLE_
		ifdef _CAS9_SC_UPG_
			SEELIB = libsee_m3711c_vsc_scu
		else
			SEELIB = libsee_m3711c_vsc
		endif
	else
		ifdef _C0700A_VSC_ENABLE_
			SEELIB = libsee_m3711c_c0700a_vsc
		else
			SEELIB = libsee_m3711c
		endif
	endif
else
  ifdef _BUILD_USB_LOADER_
	ifdef _CAS9_VSC_ENABLE_
		ifdef _CAS9_SC_UPG_
			SEELIB = libsee_m3711c_vsc_scu
		else
			SEELIB = libsee_m3711c_ota_vsc
		endif
	else
		SEELIB = libsee_m3711c_ota
	endif
  else
	ifdef _CAS9_VSC_ENABLE_
		ifdef _CAS9_SC_UPG_
			SEELIB = libsee_m3711c_vsc_scu
		else
			SEELIB = libsee_m3711c_ota_vsc
		endif
	else
		SEELIB = libsee_m3711c_ota
	endif
  endif
  
  ifdef _C0700A_VSC_ENABLE_
  	SEELIB = libsee_m3711c_ota_c0700a_vsc
  endif
endif


BOOT_SEE = boot_see_m3711c
ifdef _M36F_SINGLE_CPU
  ifndef _BUILD_OTA_E_
    export DDKLIB = libcore_v40e_m3711c_single
    export DDKOSLIB = libcore_v40e_m3711c_os_single
  else
    export DDKLIB = libcore_v40e_m3711c_single_ota
    export DDKOSLIB = libcore_v40e_m3711c_os_single_ota
  endif
	export PVR_PLUS=${LIBPLUG_NAME_EXT}
	export IMG_ENG = libimg_eng_v40b_single
	export PLUS_OPEN=libplus_v40b_single_open
	BOOT_MOD = boot_v40b_single
else
	ifndef _BUILD_OTA_E_
  		ifdef _C0700A_VSC_ENABLE_
			export DDKLIB = libcore_v40e_m3711c_c0700a_vsc
			export DDKOSLIB = libcore_v40e_m3711c_os_c0700a_vsc
		else
			export DDKLIB = libcore_v40e_m3711c
			export DDKOSLIB = libcore_v40e_m3711c_os
		endif
		export PVR_PLUS=libplus_v40b
		export IMG_ENG =libimg_eng_v40b
		export PLUS_OPEN=libplus_v40b_open
	else
		ifdef _BUILD_USB_LOADER_RET2GAC
			export BURN_OTP=libupg_usb_burn_otp
			export DDKLIB = libcore_v40e_m3711c_usb_ret2gac
			export DDKOSLIB = libcore_v40e_m3711c_os_usb_ret2gac
			export PVR_PLUS= libplus_v40b_ret2gac
			export IMG_ENG =
			export PLUS_OPEN= 
		else
			ifdef _BUILD_USB_LOADER_
		  		ifdef _BUILD_USB_LOADER_GEN
					ifdef _C0700A_VSC_ENABLE_
						export DDKLIB = libcore_v40e_m3711c_c0700a_vsc_usb
						export DDKOSLIB = libcore_v40e_m3711c_c0700a_vsc_os_usb
					else
						export DDKLIB = libcore_v40e_m3711c_usb
						export DDKOSLIB = libcore_v40e_m3711c_os_usb
					endif
				else
		        	export DDKLIB = libcore_v40e_m3711c
					export DDKOSLIB = libcore_v40e_m3711c_os
				endif
				export PVR_PLUS=libplus_v40b_usb
				export IMG_ENG =libimg_eng_v40b_usb
				export PLUS_OPEN=libplus_v40b_usb_open
			else
				export DDKLIB = libcore_v40e_m3711c_ota
				export DDKOSLIB = libcore_v40e_m3711c_os_ota
				export PVR_PLUS=libplus_v40b_ota
				export IMG_ENG = libimg_eng_v40b_ota
				export PLUS_OPEN=libplus_v40b_ota_open
		  	endif  	#_BUILD_USB_LOADER_
	  	endif	 	#_BUILD_USB_LOADER_RET2GAC
  	endif  			#_BUILD_OTA_E_
	BOOT_MOD = boot_v40b
endif				#_M36F_SINGLE_CPU

export C_STDBY= boot3711c_stby_c
ifdef _NESTOR_SUPPORT_
LWIP_LIB = liblwip_hd.a
endif

ifdef _BUILD_OTA_E_
LWIP_LIB =
else
LWIP_LIB = liblwip_hd.a
endif

ifdef _HTTPS_ENABLE_ 
export OPENSSL_LIB = ${BASE_DIR}/ddkt2/libopenssl/libssl.a ${BASE_DIR}/ddkt2/libopenssl/libcrypto.a
endif
ifdef _MHEG5_V20_ENABLE_
export MHEG5_V2_LIB = $(DDK_DIR)/libmheg5app.a $(DDK_DIR)/libdsmcc.a $(DDK_DIR)/libengcore.a $(DDK_DIR)/libclass.a $(DDK_DIR)/libfont.a $(DDK_DIR)/libasn.a $(DDK_DIR)/libmheg5app.a $(DDK_DIR)/libpng.a $(DDK_DIR)/libzlib.a
endif

ifdef _OPENSSL_ENABLE
export OPENSSL_LIB = libopenssl.a
export CIPLUSCC_LIB = 
ifdef _CIPLUS_CC_ENABLE_
export CIPLUSCC_LIB = libciplus_cc.a
endif
else
export OPENSSL_LIB = 
export CIPLUSCC_LIB = 
endif

ifdef _WIFI_DISPLAY_ENABLE_
WFD_LIB = libnmpwfd.a 
#PTHREAD_LIB = lib_ali_pthread.a
OPENSSL2_LIB = libopenssl.a
HDCP2_LIB=libhdcp2.a
else
WFD_LIB =  
PTHREAD_LIB = 
OPENSSL2_LIB = 
HDCP2_LIB=
endif

ifdef _CAS7_CA_ENABLE_
	export CA_LIB = libconax7.a
	export FONT_LIB =  $(DDK_DIR)/libfreetype_m32.a
else
	ifdef _CAS9_CA_ENABLE_
		ifdef _CAS9_VSC_ENABLE_
			ifdef _CAS9_VSC_RAP_ENABLE_
				export CA_LIB = libconax_vsc.a
			else
				ifdef _CAS9_SC_UPG_
					export CA_LIB = libconax_scu.a libupg_rs232_3711c_vsc.a
				else
					export CA_LIB = libconax_vsc.a libupg_rs232_3711c_vsc.a
				endif
			endif
		else
		export CA_LIB = libconax.a libupg_rs232_3711c.a
		endif
		export FONT_LIB = $(DDK_DIR)/libfreetype_m32.a
	else
                ifdef _BC_CA_ENABLE_
                	ifdef _AUI_
                		export CA_LIB = $(DDK_DIR)/libaui.a lib_bc.a lib_bc_nsc.a
                	else
                        	export CA_LIB = lib_bc.a lib_bc_nsc.a
                        	ifdef _BC_AUI_
                        		CA_LIB += libaui_bc.a
                        	endif
                	endif
                        export FONT_LIB =
                else
		export CA_LIB =
		export FONT_LIB =	
	endif
endif
endif

ifdef _C0700A_VSC_ENABLE_
	export CA_LIB = lib_c0700a_vsc.a
	export FONT_LIB = $(DDK_DIR)/libfreetype_m32.a
endif
				
export SAT2IP_LIB=
export HTTPSERVER_LIB =
ifdef _SAT2IP_SERVER_ENABLE_
	 SAT2IP_LIB += libsat2ipserver.a libprovider.a
	 HTTPSERVER_LIB = libhttpmgserver.a
endif
ifdef _SAT2IP_CLIENT_ENABLE_
	SAT2IP_LIB += libsat2ipclient.a
endif
ifdef _LIB_UPNP_ENABLE_
	SAT2IP_LIB += libupnp.a
endif
ifdef _ALI_PLAY_VERSION_
	export ALIPLAY_LIB = libaliplay.a
	export JSON_LIB = libjson.a
endif

export WIFISTORAGE_LIB =
ifdef _WIFISTORAGE_ENABLE_
	WIFISTORAGE_LIB += libwifistorage.a
	HTTPSERVER_LIB = libhttpmgserver.a
	export JSON_LIB = libjson.a	
endif

ifdef _BUILD_OTA_E_
CPP_LIBS =
else
ifdef _RTSP_SUPPORT_
ifdef _LINUX_COMPILER_ENVIRONMENT_
export CPP_LIBS = ${DDK_DIR}/liblive555_linux.a
else
export CPP_LIBS = ${DDK_DIR}/liblive555.a ${DDK_DIR}/libsupc++.a
endif
else
CPP_LIBS =
endif
endif

ifdef _YOUTUBE_ENABLE
export U2B_LIB  = libyoutube.a
export JSON_LIB = libjson.a
export OPENSSL_LIB = ${BASE_DIR}/ddkt2/libopenssl/libssl.a ${BASE_DIR}/ddkt2/libopenssl/libcrypto.a
endif
ifdef _DISABLE_MEDIA_PLAYER_
	export IMG_ENG_LIB = 
else
	export IMG_ENG_LIB = ${IMG_ENG}.a
endif

ifdef DUAL_ENABLE
export ALL_LIBS = ${CPP_LIBS} $(U2B_LIB) $(JSON_LIB) $(ALIPLAY_LIB) $(CIPLUSCC_LIB) $(OPENSSL_LIB) ${PVR_PLUS}.a ${IMG_ENG_LIB} ${DDKLIB}.a \
			liblog.a ${SMOD_LIB} ${LWIP_LIB} ${CA_LIB} ${FONT_LIB}
else
export ALL_LIBS = ${CPP_LIBS} $(U2B_LIB) $(JSON_LIB) $(OPENSSL_LIB) ${PVR_PLUS}.a ${DDKLIB}.a libmpg2c.a \
			liblog.a ${SMOD_LIB} ${LWIP_LIB} ${CA_LIB} ${FONT_LIB}
endif

ALL_TOOLS = LZMA.EXE ObjSize.exe binsert.exe bmerger.exe dumpmerge_7z.exe \
			fidmerger.exe fidmerger_ca.exe fmerge.exe gzip.exe module_dif.exe code_build.exe \
			lzox99.exe bin2headfile.exe bin2txt.exe HEX2BIN.EXE bl_tool.exe universal_key_encrypt.exe \
			bl_tool1.exe ALiCrypto.exe uk_encrypt.exe

#BOOT_LIBS_3503 = liblog.a boot35.o libslot3602f.a libstandby36.a libboot_m35.a

export CHIP_CFG_PATH = platform/chip/m3711c
export CHIP_AVDCFG_PATH = platform/advanced_cfg/m3711c_board

ifdef _BOARD_DB_M3711C_01V01_
export BOARD_CFG_PATH = platform/board_cfg/BOARD_DB_M3711C_01V01
endif

ifeq ($(BOARD_CFG_PATH),)
$(error "no board select!!!")
endif

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}
LMOD_ = ${LMOD:=_}
OSMOD_ = ${OSMOD:=_}
RMOD_ = ${RMOD:=_}
PMOD_ = ${PMOD:=_}
OPMOD_ = ${OPMOD:=_}
IMGMOD_ = ${IMGMOD:=_}
PPMOD_ = ${PPMOD:=_}
SMOD__ = ${SMOD:=__}
LMOD__ = ${LMOD:=__}
OSMOD__ = ${OSMOD:=__}
RMOD__ = ${RMOD:=__}
PMOD__ = ${PMOD:=__}
OPMOD__ = ${OPMOD:=__}
IMGMOD__ = ${IMGMOD:=__}
PPMOD__= ${PPMOD:=__}
BL_RMOD_= ${BL_RMOD:=_}
BL_RMOD__= ${BL_RMOD:=__}

all : ${MOD}.mk

	cp ${DDK_DIR}/${BOOT_MOD}.o ${LIB_DIR}/boot.o
	cp ${DDK_DIR}/lib7zip.a ${LIB_DIR}/
	cp ${DDK_DIR}/${C_STDBY}.a ${LIB_DIR}/

ifdef _HTTPS_ENABLE_
	cp -f  ${BASE_DIR}/ddkt2/libopenssl/libssl.a ${LIB_DIR}
	cp -f  ${BASE_DIR}/ddkt2/libopenssl/libcrypto.a ${LIB_DIR}
endif
ifdef _OPENSSL_ENABLE
	cp ${DDK_DIR}/libopenssl.a ${LIB_DIR}/
endif

ifdef _WIFI_DISPLAY_ENABLE_
	cp -f ${DDK_DIR}/wfd/${WFD_LIB} ${LIB_DIR}/
	cp -f ${DDK_DIR}/wfd/${HDCP2_LIB} ${LIB_DIR}/
	cp -f ${DDK_DIR}/wfd/$(OPENSSL2_LIB) ${LIB_DIR}/
#	cp -f ${DDK_DIR}/wfd/${PTHREAD_LIB} ${LIB_DIR}/
endif	

ifdef _CAS7_CA_ENABLE_
	cp -f ${ROOT_PRJ}/${CONAX_AP_DIR}/libconax7.a ${LIB_DIR}/
endif
ifdef _CAS9_CA_ENABLE_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libconax*.a ${LIB_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libupg_rs232*.a ${LIB_DIR}/
endif
ifdef _C0700A_VSC_ENABLE_
	cp -f ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/${CA_AP_DIR}/../c0700a_ap/lib_c0700a_vsc.a ${LIB_DIR}/
endif
ifdef _BC_CA_ENABLE_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/lib_bc*.a ${LIB_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/lib_bc_nsc*.a ${LIB_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/tool/padding.exe ${ROOT_PRJ}/padding.exe
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/tool/ftell.exe ${ROOT_PRJ}/ftell.exe
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/tool/create_signature.exe ${ROOT_PRJ}/create_signature.exe
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/tool/fmerge.exe ${ROOT_PRJ}/fmerge.exe
	cp -rf ${ROOT_PRJ}/${CA_AP_DIR}/script_3527/. ${ROOT_PRJ}/
ifdef _SMI_ENABLE_
	cp -rf ${ROOT_PRJ}/${SMI_AP_DIR}/script_3527/. ${ROOT_PRJ}/
else	
ifdef _BC_AUI_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libaui_bc.a ${LIB_DIR}/
endif
endif
endif
	${MAKE} -f ${MOD}.mk ${OBJS}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	rm -f ${LIB_DIR}/list.mod
ifndef _AUI_
	cd copper_common; \
	  ${MAKE} all
	cd gos_ui; \
	  ${MAKE} all
endif
ifndef _BUILD_USB_LOADER_RET2GAC	
	cd platform; \
	  ${MAKE} all
endif
	cd platform/chip; \
	  ${MAKE} all
	cd ${CHIP_CFG_PATH}; \
	  ${MAKE} all
ifndef _BUILD_USB_LOADER_RET2GAC		  
	cd ${CHIP_AVDCFG_PATH}; \
	  ${MAKE} all
endif  
	@echo Entering Board: ${BOARD_CFG_PATH}
ifndef _BUILD_USB_LOADER_RET2GAC	
	cd ${BOARD_CFG_PATH}; \
	 ${MAKE} all
endif
ifeq ($(SAT2IP_APP_DIR),sat2ip)
	cd sat2ip;\
	${MAKE} all
endif
ifdef _MAC_TEST
	cd ethernetmac; \
	  ${MAKE} all
endif 
ifndef _AUI_
ifdef _CAS9_VSC_RAP_ENABLE_
	cd vsc_rap; \
	  ${MAKE} all	
endif

ifdef _CAS7_CA_ENABLE_
ifndef _BUILD_OTA_E_
	cd ${CONAX_AP_DIR}; \
  		${MAKE} all
endif
endif
ifdef _CAS9_CA_ENABLE_
ifndef _BUILD_OTA_E_
	cd ${CA_AP_DIR}; \
  		${MAKE} all
endif
endif
ifdef _GEN_CA_ENABLE_
ifndef _BUILD_OTA_E_
	cd ${CA_AP_DIR}; \
  		${MAKE} all
endif
endif
ifdef _BC_CA_ENABLE_
	cd ${CA_AP_DIR}; \
  		${MAKE} all
endif


ifdef _SMI_ENABLE_
ifndef _APP_LIB_
	cd ${SMI_AP_DIR}; \
  		${MAKE} all
endif
endif
endif
ifeq ($(FSC_APP_DIR), fsc_ap)
ifndef _BUILD_OTA_E_
	cd fsc_ap;\
	${MAKE} all
endif	
endif
ifdef _C1800A_LOADERCORE_ENABLE_
	cd ${CA_AP_DIR}; \
  		${MAKE} all
endif

ifdef _C1800A_LOADERCORE_ENABLE_
	cd ${LOADER_SPI_DIR}; \
  		${MAKE} all
	cd ${LOADER_BL_DIR}; \
  		${MAKE} all  		
endif

ifdef _C1800A_LOADERCORE_ENABLE_
	cd ${VD_DIR}; \
  		${MAKE} all
endif

	cp -f ${LSCR} ${LIB_DIR}/lscript.ld

	cd ${LIB_DIR}; \
	  echo include ../src/path.def > ${MOD}.lk; \
	  echo include ../src/compiler.def >> ${MOD}.lk; \
	  echo NEED = \\ >> ${MOD}.lk; \
	    echo cat \\ > cat.sh; \
	    cat list.mod >> cat.sh; \
	    echo ${MOD}.lst \\ >> cat.sh; \
	    echo \>\> ${MOD}.lk >> cat.sh; \
	    sh < cat.sh; \
	    rm -f cat.sh
	cat ${MAKEFILE} | sed '/^include/d'>> ${LIB_DIR}/${MOD}.lk
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk ${LINKER};
ifdef DUAL_ENABLE
ifndef _USE_SIGNED_BL_SEE
	cp -f ${SEE_LSCR} ${LIB_DIR}
	cd ${LIB_DIR}; \
  	  ${MAKE} -f ${MOD}.lk ${SEE_LINKER}
endif  	  
endif

ifdef _LINUX_COMPILER_ENVIRONMENT_
	sh < post-rename-3711c.sh
endif

linkonly :
	cp -f ${LSCR} ${LIB_DIR}/lscript.ld

	cd ${LIB_DIR}; \
	  echo include ../src/path.def > ${MOD}.lk; \
	  echo include ../src/compiler.def >> ${MOD}.lk; \
	  echo NEED = \\ >> ${MOD}.lk; \
	    echo cat \\ > cat.sh; \
	    cat list.mod >> cat.sh; \
	    echo ${MOD}.lst \\ >> cat.sh; \
	    echo \>\> ${MOD}.lk >> cat.sh; \
	    sh < cat.sh; \
	    rm -f cat.sh
	cat ${MAKEFILE} >> ${LIB_DIR}/${MOD}.lk
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk ${LINKER};
ifdef DUAL_ENABLE
	cp -f ${SEE_LSCR} ${LIB_DIR}
	cd ${LIB_DIR}; \
  	  ${MAKE} -f ${MOD}.lk ${SEE_LINKER}
endif

${MOD}.mk : ${ASMS} ${SRCS}

	echo include ${ROOT}/src/path.def > $@
	echo include ${ROOT}/src/compiler.def >> $@
	cat ${MAKEFILE} >> $@
ifdef _WARNING_TO_ERROR
	echo "" >>$@
	echo "CFLAGS+=${EXTRA_FLAGS}" >>$@
endif
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@

clean :
	rm -f ${OBJS} ${MOD}.mk
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk cleanld; \
	  rm -f lscript.ld list.mod ${MOD}.lk; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst
ifndef _AUI_
	  cd copper_common; \
	  ${MAKE} clean
	  cd gos_ui; \
	  ${MAKE} clean
endif
	  cd platform; \
	   ${MAKE} clean
	  cd platform/chip; \
	   ${MAKE} clean
	  cd ${CHIP_CFG_PATH}; \
	   ${MAKE} clean
	  cd ${CHIP_AVDCFG_PATH}; \
	   ${MAKE} clean
	  cd ${BOARD_CFG_PATH}; \
	  ${MAKE} clean
ifndef _AUI_
ifdef _CAS9_VSC_RAP_ENABLE_
	  cd vsc_rap; \
	   ${MAKE} clean
endif

ifdef _CAS7_CA_ENABLE_	  
	  cd ${CONAX_AP_DIR}; \
	  ${MAKE} clean
endif
ifdef _CAS9_CA_ENABLE_	  
	  cd ${CA_AP_DIR}; \
	  ${MAKE} clean
endif
ifdef _GEN_CA_ENABLE_
	cd ${CA_AP_DIR}; \
  		${MAKE} clean
endif
ifdef _BC_CA_ENABLE_
	  cd ${CA_AP_DIR}; \
	   ${MAKE} clean
endif
ifdef _SMI_ENABLE_
	cd ${SMI_AP_DIR}; \
  		${MAKE} clean
endif
endif
ifdef _MAC_TEST
	cd ethernetmac; \
	  ${MAKE} clean
endif
ifdef _C1800A_LOADERCORE_ENABLE_
	cd ${CA_AP_DIR}; \
  		${MAKE} clean
endif
ifdef _C1800A_LOADERCORE_ENABLE_
	cd ${VD_DIR}; \
  		${MAKE} clean
endif
ifeq ($(SAT2IP_APP_DIR),sat2ip)
	cd sat2ip;\
	${MAKE} clean
endif
ifeq ($(FSC_APP_DIR), fsc_ap)
ifndef _BUILD_OTA_E_
	cd fsc_ap;\
	${MAKE} clean
endif
endif

new : clean
	cd ${ROOT}/src; \
	  ${MAKE} ${RMOD_}

rebuild: clean all

ifdef LOGISCOPE_CMD_LINE
# gather all project compile define
logiscope_def:=$(foreach item, $(DEFS), $(subst -,,$(item)))

# gather all SDK include path, except compiler's default include path which will add alone
logiscope_inc:=$(filter -I/cygdrive%,${CFLAGS})
logiscope_inc:=$(foreach item, ${logiscope_inc}, $(subst -I/cygdrive/,,${item}))
logiscope_inc:=$(foreach item, ${logiscope_inc}, $(firstword $(subst /, ,${item})):\\$(subst $(firstword $(subst /, ,${item}))/,,${item}))
logiscope_inc:=$(subst /,\\,${logiscope_inc})

logiscope:
	@echo -n  > ${ROOT}/src/logiscope.def
	@for k in $(logiscope_inc); \
		do echo I$$k>> ${ROOT}/src/logiscope.def;\
	done
#	# add compiler's default include path alone(position must after SDK's include path)
	@echo "IC:\Cygwin\mips-sde\lib\gcc\sde\3.4.4\include" >> ${ROOT}/src/logiscope.def
	@echo "IC:\Cygwin\mips-sde\sde\include" >> ${ROOT}/src/logiscope.def
	@echo "D__GNUC__=1" >> ${ROOT}/src/logiscope.def
	@echo "D__STDC__=1" >>${ROOT}/src/logiscope.def
#	# for log_cc can not reconize our config is little endian
	@echo "D__IEEE_LITTLE_ENDIAN=1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_DECA_REMOTE=1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_SND_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_VP_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_SUBT_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_CLOSE_CAPTION_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_TTX_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_SDEC_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_ISDBTCC_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_ISDBTCC_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_VBI_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DECA_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DECA_M33_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_SND_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_SND_M33_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_VP_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_VP_M33_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_SDEC_SW_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_VBI_M33_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DMX_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DSC_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_CRYPTO_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_TRNG_M36F_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_DSC_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_CRYPTO_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_HLD_DECV_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DECV_M36_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LLD_DECV_AVC_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_PE_ADV_VIDEO_ENGINE_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_PE_MUSIC_ENGINE_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@echo "D_LIB_PE_IMAGE_ENGINE_REMOTE = 1" >>${ROOT}/src/logiscope.def
	@for k in $(logiscope_def); \
		do echo $$k=1 >>${ROOT}/src/logiscope.def; \
	done
else
logiscope:
	@echo "Done!"
endif

SEEDIR?=see

sync: 
	cd ../${ROOT}/${SEEDIR}; pwd
	cp -f ../${ROOT}/${SEEDIR}/inc/modules.h ${ROOT}/inc/
	cp -f ../${ROOT}/${SEEDIR}/src/hld/mod/modules_cpu.c ${ROOT}/src/hld/mod/
#	cp -f ../${ROOT}/${SEEDIR}/src/see/m36f/compiler_cpu.def ./
	cp -f ../${ROOT}/${SEEDIR}/ddkt2/${SEELIB}*.a ${ROOT}/ddkt2;
	cp -f ../${ROOT}/${SEEDIR}/ddkt2/${BOOT_SEE}.o ${ROOT}/ddkt2/;
#	cp -f ../${ROOT}/${SEEDIR}/ddkt2/plugin/* ${ROOT}/ddkt2/plugin/
#*/

syncbl:
	@if [ -d ../${ROOT}/ALi_Boot ];then \
		ls ../${ROOT}/ALi_Boot/img/*.abs;\
		cp -vf ../${ROOT}/ALi_Boot/img/*.abs ${ROOT_PRJ}/ ;\
	else\
		if [ -d ../${ROOT}/aliboot ];then \
			ls ../${ROOT}/aliboot/img/*.abs;\
			cp -vf ../${ROOT}/aliboot/img/*.abs ${ROOT_PRJ}/ ;\
		else \
			echo "1 ";\
			echo "cannot find aliboot or ALi_Boot directory ";\
			echo "2 ";\
		fi \
	fi

ddk:
ifdef _CAS7_CA_ENABLE_
#	for compile error
	cp -f ${ROOT_PRJ}/${CONAX_AP_DIR}/libconax7.a ${DDK_DIR}/
endif

ifdef _GEN_CA_ENABLE_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libupg_usb*.a ${LIB_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libupg_usb*.a ${DDK_DIR}/
endif                               

ifdef _CAS9_CA_ENABLE_
#	for compile error
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libconax*.a ${DDK_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libupg_rs232*.a ${DDK_DIR}/
endif
ifdef _C0700A_VSC_ENABLE_
	cp -f ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/${CA_AP_DIR}/../c0700a_ap/lib_c0700a_vsc.a ${DDK_DIR}/
endif
ifdef _BC_CA_ENABLE_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/lib_bc*.a ${DDK_DIR}/
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/lib_bc_nsc*.a ${DDK_DIR}/
ifdef _BC_AUI_
	cp -f ${ROOT_PRJ}/${CA_AP_DIR}/libaui_bc.a ${LIB_DIR}/
endif
endif
	cd ${DDK_DIR}; \
	cp ${ALL_LIBS} ${SAT2IP_LIB} ${HTTPSERVER_LIB} ${WIFISTORAGE_LIB} ${LIB_DIR}/ ;\
	rm -f ${LIB_DIR}/${DDKLIB}.mak

bc :
ifndef _AUI_
	cd ${ROOT_PRJ}/bc_sec; \
	${MAKE} all
else
	cd ${ROOT_PRJ}/bc_sec; \
	${MAKE} all
	cd ${BASE_DIR}/src/lib/libcas/bc/src; \
	${MAKE} all
endif

bc_clean :
ifndef _AUI_
	cd ${ROOT_PRJ}/bc_sec; \
	${MAKE} clean
else
	cd ${ROOT_PRJ}/bc_sec; \
	${MAKE} clean
	cd ${BASE_DIR}/src/lib/libcas/bc/src; \
	${MAKE} clean
endif

sat2ip_lib:
ifdef _LIB_UPNP_ENABLE_
	cd ${ROOT}/src; \
	  ${MAKE} LIB_UPNP
endif
	cd ${ROOT}/src; \
	  ${MAKE} LIB_SATIP_SERVER
	cd ${ROOT}/src; \
	  ${MAKE} LIB_MEDIA_PROVIDER

stdby_lib:
	cd ${ROOT}/src; \
	  ${MAKE} LIB_STANDBY
 
nim:
	rm -f ${LIB_DIR}/${DDKLIB}.mak
	rm -f ${LIB_DIR}/list.nmt
	cd ${ROOT}/src; \
	  ${MAKE} ${SMOD}
	  
	echo "#" > ${LIB_DIR}/${DDKLIB}.mak
# ifdef _DVBS_ENABLE_
	 cd ${LIB_DIR}; \
	 echo NIM_S2_MOD_OBJ = \\ >> ${DDKLIB}.mak; \
	 cat ${NIM_S2_MOD}.lst >> ${DDKLIB}.mak; \
	 echo "#" >> ${LIB_DIR}/${DDKLIB}.mak
# endif
	  
# ifdef _DVBT_ENABLE_
	 cd ${LIB_DIR}; \
	 echo NIM_T_MOD_OBJ = \\ >> ${DDKLIB}.mak; \
	 cat ${NIM_T_MOD}.lst >> ${DDKLIB}.mak; \
	 echo "#" >> ${LIB_DIR}/${DDKLIB}.mak
# endif

# ifdef _DVBT2_ENABLE_
	 cd ${LIB_DIR}; \
	 echo NIM_S3821_MOD_OBJ = \\ >> ${DDKLIB}.mak; \
	 cat ${NIM_S3821_MOD}.lst >> ${DDKLIB}.mak; \
	 echo "#" >> ${LIB_DIR}/${DDKLIB}.mak
# endif
	  
#ifdef _DVBC_ENABLE_
	 cd ${LIB_DIR}; \
	 echo NIM_S3281_C_MOD_OBJ = \\ >> ${DDKLIB}.mak; \
	 cat ${NIM_S3281_C_MOD}.lst >> ${DDKLIB}.mak; \
	 echo "#" >> ${LIB_DIR}/${DDKLIB}.mak
#endif

	echo all: >> ${LIB_DIR}/${DDKLIB}.mak

# ifdef _DVBS_ENABLE_
	echo "	${AR} -r ${NIM_S2_LIB} \$${NIM_S2_MOD_OBJ}" >> ${LIB_DIR}/${DDKLIB}.mak
# endif
# ifdef _DVBT_ENABLE_
	echo "	${AR} -r ${NIM_T_LIB} \$${NIM_T_MOD_OBJ}" >> ${LIB_DIR}/${DDKLIB}.mak
# endif
# ifdef _DVBT2_ENABLE_
	echo "	${AR} -r ${NIM_S3821_LIB} \$${NIM_S3821_MOD_OBJ}" >> ${LIB_DIR}/${DDKLIB}.mak
# endif
# ifdef _DVBC_ENABLE_
	echo "	${AR} -r ${NIM_S3281_C_LIB} \$${NIM_S3281_C_MOD_OBJ}" >> ${LIB_DIR}/${DDKLIB}.mak
# endif

	cd ${LIB_DIR}; \
	  ${MAKE} -f ${DDKLIB}.mak
	  
# ifdef _DVBS_ENABLE_
	cp ${LIB_DIR}/${NIM_S2_LIB} ${DDK_DIR}
# endif
# ifdef _DVBT_ENABLE_
	cp ${LIB_DIR}/${NIM_T_LIB} ${DDK_DIR}
# endif
# ifdef _DVBT2_ENABLE_
	cp ${LIB_DIR}/${NIM_S3821_LIB} ${DDK_DIR}
# endif	
# ifdef _DVBC_ENABLE_
	cp ${LIB_DIR}/${NIM_S3281_C_LIB} ${DDK_DIR}
# endif

sdk_os:

ifdef _BUILD_USB_LOADER_RET2GAC 
	rm -f ${LIB_DIR}/list.ddk
	rm -f ${LIB_DIR}/list.nmt
	
	cd ${ROOT}/src; \
	  ${MAKE} ${OSMOD} ;\

	cd ${LIB_DIR}; \
	  echo DDK = \\ > ${DDKOSLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${DDKOSLIB}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${DDKOSLIB}.mak	
	echo all: >> ${LIB_DIR}/${DDKOSLIB}.mak
	echo "	${AR} -r ${DDKOSLIB}.a \$${DDK}" >> ${LIB_DIR}/${DDKOSLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${DDKOSLIB}.mak

	cp ${LIB_DIR}/${DDKOSLIB}.a ${DDK_DIR}/sec
	cp -f ${DDK_DIR}/boot.o ${DDK_DIR}/${BOOT_MOD}.o
	
	cp -f ${ROOT}/src/lld/sec/m36f/${SECLIB_MAIN}.a ${DDK_DIR}/sec/${SECLIB_MAIN}.a
	cd ${ROOT}/ddkt2/sec; \
		rm -f *.o; \
		${AR} -x ${SECLIB_MAIN}.a; \
		${AR} -r ${DDKOSLIB}.a *.o; \
		rm -f *.o
else
	rm -f ${LIB_DIR}/list.ddk
	rm -f ${LIB_DIR}/list.nmt
	
	${MAKE} nim; \

	cd ${ROOT}/src; \
	  ${MAKE} ${OSMOD} ;\

	cd ${LIB_DIR}; \
	  echo DDK = \\ > ${DDKOSLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${DDKOSLIB}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${DDKOSLIB}.mak	
	echo all: >> ${LIB_DIR}/${DDKOSLIB}.mak
	echo "	${AR} -r ${DDKOSLIB}.a \$${DDK}" >> ${LIB_DIR}/${DDKOSLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${DDKOSLIB}.mak

	cp ${LIB_DIR}/${DDKOSLIB}.a ${DDK_DIR}/sec
	cp -f ${DDK_DIR}/boot.o ${DDK_DIR}/${BOOT_MOD}.o
	
	cp -f ${ROOT}/src/lld/sec/m36f/${SECLIB_MAIN}.a ${DDK_DIR}/sec/${SECLIB_MAIN}.a
	cd ${ROOT}/ddkt2/sec; \
		rm -f *.o; \
		${AR} -x ${SECLIB_MAIN}.a; \
		${AR} -r ${DDKOSLIB}.a *.o; \
		rm -f *.o
endif

	cp -f ${DDK_DIR}/sec/${DDKOSLIB}.a ${DDK_DIR}/ 
	cp -f ${DDK_DIR}/sec/${DDKOSLIB}.a ${LIB_DIR}/ 
sdk:
	rm -f ${LIB_DIR}/list.ddk
	rm -f ${LIB_DIR}/list.nmt

	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD} ;\

	cd ${LIB_DIR}; \
	  echo DDK = \\ > ${DDKLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${DDKLIB}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${DDKLIB}.mak
	echo all: >> ${LIB_DIR}/${DDKLIB}.mak
	echo "	${AR} -r ${DDKLIB}.a \$${DDK}" >> ${LIB_DIR}/${DDKLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${DDKLIB}.mak

	cp ${LIB_DIR}/${DDKLIB}.a ${DDK_DIR}
#	cd ${DDK_DIR}; \
#	${AR} -r ${DDKLIB}.a ${DDKOSLIB}.a
	mkdir -p ${DDK_DIR}/temp
	cp ${LIB_DIR}/${DDKLIB}.a ${DDK_DIR}/temp
	cp ${DDK_DIR}/${DDKOSLIB}.a ${DDK_DIR}/temp
	cd ${DDK_DIR}/temp; \
	${AR} -x ${DDKOSLIB}.a;\
	${AR} -r ${DDKLIB}.a *.o;\
	${AR} -r ${DDKLIB}.a libmpg*.a;\
	cp ${DDKLIB}.a ${DDK_DIR}
	rm -rf ${DDK_DIR}/temp
ifndef _BUILD_OTA_E_
ifneq ($(LWIP_LIB),)
	cp ${LIB_DIR}/${LWIP_LIB} ${DDK_DIR}/
endif
endif

	cp -f ${DDK_DIR}/${DDKLIB}.a ${LIB_DIR}/${DDKLIB}.a
	rm -f ${LIB_DIR}/${DDKLIB}.mak
	rm -f ${DDK_DIR}/boot.o

pplus:
	cd ${ROOT}/src; \
	  ${MAKE} ${PPMOD}

plus: plus_open
	rm -f ${LIB_DIR}/list.ddk
	rm -f ${LIB_DIR}/list.nmt
	cd ${ROOT}/src; \
	  ${MAKE} ${PMOD}
	cd ${LIB_DIR}; \
	  echo NEED = \\ > ${PVR_PLUS}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${PVR_PLUS}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${PVR_PLUS}.mak
	 cd ${LIB_DIR}; \
	 echo NIM = \\ >> ${PVR_PLUS}.mak; \
	 echo cat \\ > cat.sh; \
	 cat list.nmt >> cat.sh; \
	 echo \>\> ${PVR_PLUS}.mak >> cat.sh; \
	 sh < cat.sh; \
	 rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${PVR_PLUS}.mak
	echo all: >> ${LIB_DIR}/${PVR_PLUS}.mak
	echo "	${AR} -r ${PVR_PLUS}.a \$${NEED}" >> ${LIB_DIR}/${PVR_PLUS}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${PVR_PLUS}.mak
	cp ${LIB_DIR}/${PVR_PLUS}.a ${DDK_DIR}/
	rm -f ${LIB_DIR}/${PVR_PLUS}.mak

	rm -f ${LIB_DIR}/list.ddk
	cd ${ROOT}/src; \
	  ${MAKE} ${IMGMOD}
	cd ${LIB_DIR}; \
	  echo NEED = \\ > ${IMG_ENG}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${IMG_ENG}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${IMG_ENG}.mak	  
	echo all: >> ${LIB_DIR}/${IMG_ENG}.mak
	echo "	${AR} -r ${IMG_ENG}.a \$${NEED}" >> ${LIB_DIR}/${IMG_ENG}.mak
	cd ${LIB_DIR}; \
	 ${MAKE} -f ${IMG_ENG}.mak
	cp ${LIB_DIR}/${IMG_ENG}.a ${DDK_DIR}/
	rm -f ${LIB_DIR}/${IMG_ENG}.mak
ddk_clean:

sdk_os_clean:
	cd ${LIB_DIR}; \
	  rm -f *.a *.o
	cd ${ROOT}/src; \
	  ${MAKE} ${OSMOD_};
	  
sdk_clean:
#	cd ${DDK_DIR}; \
#	  rm -f ${ALL_LIBS} *.o
	cd ${LIB_DIR}; \
	  rm -f *.a *.o
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD_} ${SMOD_};
sdk_lib_clean:
#	cd ${DDK_DIR}; \
#	  rm -f ${ALL_LIBS} *.o
	cd ${LIB_DIR}; \
	  rm -f *.a *.o
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD_};

plus_clean: plus_open_clean
	cd ${DDK_DIR}; \
	rm -f ${PVR_PLUS}.a
	cd ${LIB_DIR}; \
	rm -f ${PVR_PLUS}.a
	cd ${ROOT}/src; \
	  ${MAKE} ${PMOD_};
	cd ${DDK_DIR}; \
		rm -f ${IMG_ENG}.a
	cd ${LIB_DIR}; \
		rm -f ${IMG_ENG}.a
	cd ${ROOT}/src; \
	  ${MAKE} ${IMGMOD_};  
 
pplus_clean:
	cd ${PLUGINDDK_DIR} && \
		rm -f *.*;
	cd ${PLUGINEXT_DIR} && \
		rm -f *.*;
	cd ${ROOT}/src; \
	  ${MAKE} ${PPMOD_};
	
plus_open:
	rm -f ${LIB_DIR}/list.ddk
	cd ${ROOT}/src; \
	  ${MAKE} ${OPMOD}
	cd ${LIB_DIR}; \
	  echo NEED = \\ > ${PLUS_OPEN}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.ddk >> cat.sh; \
	  echo \>\> ${PLUS_OPEN}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh	
	echo "#" >> ${LIB_DIR}/${PLUS_OPEN}.mak
	echo all: >> ${LIB_DIR}/${PLUS_OPEN}.mak
	echo "	${AR} -r ${PLUS_OPEN}.a \$${NEED}" >> ${LIB_DIR}/${PLUS_OPEN}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${PLUS_OPEN}.mak
	cp ${LIB_DIR}/${PLUS_OPEN}.a ${DDK_DIR}/
	rm -f ${LIB_DIR}/${PLUS_OPEN}.mak
	
plus_merge:
	mkdir -p ${DDK_DIR}/temp
	cp ${DDK_DIR}/${PVR_PLUS}.a ${DDK_DIR}/temp
	cp ${DDK_DIR}/${PLUS_OPEN}.a ${DDK_DIR}/temp
	cd ${DDK_DIR}/temp; \
	${AR} -x ${PLUS_OPEN}.a;\
	${AR} -r -s ${PVR_PLUS}.a *.o;\
	cp ${PVR_PLUS}.a ${DDK_DIR}
	cp ${DDK_DIR}/${PVR_PLUS}.a ${LIB_DIR}/
	rm -rf ${DDK_DIR}/temp
	
plus_open_clean:	
	rm -f ${PLUS_OPEN}.a
	cd ${DDK_DIR}; \
	rm -f ${PLUS_OPEN}.a
	cd ${ROOT}/src; \
	  ${MAKE} ${OPMOD_}
	  
release :
	rm -f ${LIB_DIR}/list.sdk
	rm -f ${LIB_DIR}/list.mod
	cd ${ROOT}/src; \
	  ${MAKE} ${RMOD}
	cd ${LIB_DIR}; \
	  echo NEED = \\ > ${RELLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.mod >> cat.sh; \
	  echo \>\> ${RELLIB}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${RELLIB}.mak
	echo all: >> ${LIB_DIR}/${RELLIB}.mak
	echo "	${AR} -r ${RELLIB}.a \$${NEED}" >> ${LIB_DIR}/${RELLIB}.mak
#	sed -e "s/boot.o//" -e "s/glue.o//" ${LIB_DIR}/${RELLIB}.mak > ${LIB_DIR}/${RELLIB}.tmp
#	mv ${LIB_DIR}/${RELLIB}.tmp ${LIB_DIR}/${RELLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${RELLIB}.mak
#	cp ${LIB_DIR}/${RELLIB}.a ${LIB_DIR}/boot.o \
#	  ${LIB_DIR}/glue.o ${RELPROJ}/lib
	rm -f ${LIB_DIR}/${RELLIB}.mak

compress :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk compress

generate :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk generate

compress_7z :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk compress_7z

generate_7z :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk generate_7z
	  
${LIB_DIR}/makefile4bin: ${LIB_DIR}/makefile4bin.lk
	echo "Generate makefile4bin"
	cd ${LIB_DIR}; \
	chmod 777 ../* -R; \
	echo include ../src/path.def > makefile4bin; \
	echo include ../src/compiler.def >> makefile4bin; \
	cat makefile4bin.lk | sed '/^include/d' >> ${LIB_DIR}/makefile4bin


see_enc:
	cp $(DDK_DIR)/${SEELIB}${VSC_POSTFIX}${SCU_POSTFIX}.a ${CA_AP_DIR}/seelib_temp.a
	cd ${ROOT_PRJ}/${CA_AP_DIR}; \
	 ./encrypt_see.bat
	cp ${CA_AP_DIR}/seelib_temp2.bin $(DDK_DIR)/${SEELIB}${VSC_POSTFIX}${SCU_POSTFIX}_enc.a
	rm -f ${CA_AP_DIR}/seelib_temp2.bin ${CA_AP_DIR}/seelib_temp.a

label :
	./updatelabel.bat $(APPLABEL) > label_log.txt

label_core :
	./updatelabel.bat $(LIBCORELABEL) > label_log.txt

label_sec :
	./updatelabel_sec.bat Security_module > label_log.txt

# End of common description.

# Construct of sub-modules

# Deconstruct of sub-modules

ifdef _AUI_
DDK_PRJ_DIR=prj/app/demo/combo/aui_demo
else
DDK_PRJ_DIR=prj/app/demo/combo/sabbat_dual
endif
# This directory use to sync code to aui_demo when release
DDK_PRJ_DIR_AUI=prj/app/demo/combo/aui_demo

DDK_REL_DIR=${BASE_DIR}/../ddk_release
CAS9_REL_DIR=${BASE_DIR}/../cas9_release_combo
CAS7_REL_DIR=${BASE_DIR}/../cas7_release_combo
CIPLUS_REL_DIR=${BASE_DIR}/../ciplus_release_combo
PLUGIN_REL_DIR=${BASE_DIR}/../plugin_combo
SAT2IP_REL_DIR=${BASE_DIR}/../sat2ip_release_combo
CAS9_VSC_REL_DIR=${BASE_DIR}/../cas9_vsc_release_combo
CAS9_VSC_ENV_REL_DIR=${BASE_DIR}/../cas9_vsc_env_release_combo
GEN_CA_REL_DIR=${BASE_DIR}/../ACS_release_combo
C1800A_CA_REL_DIR=${BASE_DIR}/../c1800a_ca_release_combo
BC_REL_DIR=${BASE_DIR}/../bc_release_combo
MHEG5_REL_DIR=${BASE_DIR}/../mheg5_release_combo
PLAY_REL_DIR=${BASE_DIR}/../play_release_combo
ALICAST_REL_DIR=${BASE_DIR}/../alicast_release_combo
WIFIDIRECT_REL_DIR=${BASE_DIR}/../wifi_direct_release_combo
SAT2IP_REL_DIR=${BASE_DIR}/../Big2Small_release_combo
SAT2IPCLIENT_REL_DIR=${BASE_DIR}/../Big2SmallClient_release_combo
DVB2IP_REL_DIR=${BASE_DIR}/../dvb2ip_release_combo
WIFISTORAGE_REL_DIR=${BASE_DIR}/../wifistorage_release_combo
INDIAN_LANG_REL_DIR=${BASE_DIR}/../indian_language_release
INTEST_REL_DIR=${BASE_DIR}/../intest_release_combo
YT_REL_DIR=${BASE_DIR}/../youtube_release_combo


sdk_root:
	rm -rf ${DDK_REL_DIR}
	mkdir ${DDK_REL_DIR}
sdk_core_rel_3711c:
	rm -rf ../../../../../../ddk_core_release
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD__}
	mv ../../../../../../ddk_release ../../../../../../ddk_core_release
	
plus_open_rel_3711c:
	rm -rf ../../../../../../ddk_plus_release
	cd ${ROOT}/src; \
	  ${MAKE} ${OPMOD__}
	mv ../../../../../../ddk_release ../../../../../../ddk_plus_release

sdk_rel_3711c:
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} > dep.txt
	${DDK_DIR}/genlist dep.txt "${BASE_DIR}" "${DDK_REL_DIR}" "${DDK_REL_DIR}" "${DDK_REL_DIR}" 1>file.txt 2>dir.txt
	sh dir.txt
	sh file.txt
	rm -f dep.txt
	rm -f dir.txt
	rm -f file.txt
	mkdir -p ${DDK_REL_DIR}/obj
	mkdir -p ${DDK_REL_DIR}/ddkt2
	mkdir -p ${DDK_REL_DIR}/ddkt2/plugin
	mkdir -p ${DDK_REL_DIR}/ddkt2/plugin_ext
	#mkdir -p ${DDK_REL_DIR}/sdk
	mkdir -p ${DDK_REL_DIR}/doc
	mkdir -p ${DDK_REL_DIR}/aui
	mkdir -p ${DDK_REL_DIR}/supplement
	mkdir -p ${DDK_REL_DIR}/ddkt2/blsdk
	mkdir -p ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc
#	mkdir -p ${DDK_REL_DIR}/${DDK_PRJ_DIR}/loader3_AS_G2
	mkdir -p ${DDK_REL_DIR}/${DDK_PRJ_DIR}/DDR-Setting/C3711C
	mkdir -p ${DDK_REL_DIR}/src/app
	mkdir -p ${DDK_REL_DIR}/prj/tools
	mkdir -p ${DDK_REL_DIR}/prj/tools/tools_linux
ifdef _WIFI_ENABLE_
	mkdir -p ${DDK_REL_DIR}/ddkt2/wifi
	cp ${DDK_DIR}/wifi/lib_*_gusb_t2.a ${DDK_REL_DIR}/ddkt2/wifi/
	rm -rf 	${DDK_REL_DIR}/ddkt2/wifi/lib_*_p2p_gusb_t2.a
endif
ifdef _USB_ETHERNET_ENABLE_
	mkdir -p ${DDK_REL_DIR}/ddkt2/wifi
	cp ${DDK_DIR}/wifi/lib_usb_etnernet_t2.a ${DDK_REL_DIR}/ddkt2/wifi/
endif
	mkdir -p ${DDK_REL_DIR}/ddkt2/ali_3g
	cp ${DDK_DIR}/ali_3g/*.* ${DDK_REL_DIR}/ddkt2/ali_3g/

	mkdir -p ${DDK_REL_DIR}/src/lld/pmu/mcu
	cp -rf ${BASE_DIR}/src/lld/pmu/mcu ${DDK_REL_DIR}/src/lld/pmu
	-cp -rf ${BASE_DIR}/aui/* ${DDK_REL_DIR}/aui/
	cd ${DDK_REL_DIR}/aui/doc/doxygen;./build-doxygen.sh
ifdef _CAS9_CA_ENABLE_
	cp -f ${BASE_DIR}/alici/other/conax_sample/aui_test_stream_decrypt.c ${DDK_REL_DIR}/aui/samples/sample_src/
endif
ifdef _C1800A_CA_ENABLE_
	cp -f ${DDK_DIR}/libimg_eng_v40b*.a ${DDK_REL_DIR}/ddkt2/
endif
ifdef _NOVEL_ENABLE_
	cp -f ${BASE_DIR}/alici/other/nstv_sample/aui_test_stream_decrypt.c ${DDK_REL_DIR}/aui/samples/sample_src/
endif	
	rm -rf ${DDK_REL_DIR}/aui/nestor
	rm -rf ${DDK_REL_DIR}/aui/sfu_test
	rm -rf ${DDK_REL_DIR}/aui/src/linux
	mkdir -p ${DDK_REL_DIR}/prj/app/demo/combo/aui_demo/
	cp -rf ${BASE_DIR}/prj/app/demo/combo/aui_demo/* ${DDK_REL_DIR}/prj/app/demo/combo/aui_demo/

	cd ${DDK_DIR}; \
	cp ${ALL_LIBS} ${HTTPSERVER_LIB} ${DDK_REL_DIR}/ddkt2/ ;\
	cp lib7zip.a ${DDK_REL_DIR}/ddkt2/ ;\

	cp ${DDK_DIR}/libmpg2c_t2.a ${DDK_REL_DIR}/ddkt2/
ifdef DUAL_ENABLE
	cp ${DDK_DIR}/libsee_m3711*.a	${DDK_REL_DIR}/ddkt2/
endif
	cp ${DDK_DIR}/libcore_v40e_m3711*.a	${DDK_REL_DIR}/ddkt2/
	cp ${DDK_DIR}/libplus_v40b*.a	${DDK_REL_DIR}/ddkt2/
	cp ${DDK_DIR}/libmpg6c_t2.a ${DDK_REL_DIR}/ddkt2/
#	cp ${DDK_DIR}/plugin/lib_ogg.o ${DDK_REL_DIR}/ddkt2/plugin/
	cp ${DDK_DIR}/plugin/*.* ${DDK_REL_DIR}/ddkt2/plugin/
	cp ${DDK_DIR}/plugin_ext/*.* ${DDK_REL_DIR}/ddkt2/plugin_ext/

	cp ${DDK_DIR}/${BOOT_MOD}.o ${DDK_REL_DIR}/ddkt2/
	cp ${DDK_DIR}/libHDMI_t2.a ${DDK_REL_DIR}/ddkt2/
	cp -f ${DDK_DIR}/${C_STDBY}.a ${DDK_REL_DIR}/ddkt2/
	
ifndef _BUILD_OTA_E_	
	cp ${DDK_DIR}/libhttpmgserver.a ${DDK_REL_DIR}/ddkt2/
	#release live555 lib
	cp -f ${DDK_DIR}/liblive555*.a ${DDK_REL_DIR}/ddkt2/
	#cp -f ${DDK_DIR}/libsupc++.a ${DDK_REL_DIR}/ddkt2/
	cp -f ${DDK_DIR}/libjson.a	${DDK_REL_DIR}/ddkt2/
	#release youtube
#	mkdir -p ${DDK_REL_DIR}/inc/api/libyoutube
	mkdir -p ${DDK_REL_DIR}/inc/openssl/
#	mkdir -p ${DDK_REL_DIR}/inc/api/libyoutube/
#	cp -f ${DDK_DIR}/libyoutube.a ${DDK_REL_DIR}/ddkt2/
#	cp -f ${BASE_DIR}/inc/api/libyoutube/*.h ${DDK_REL_DIR}/inc/api/libyoutube/
	cp -rf ${BASE_DIR}/inc/api/libjson	${DDK_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/inc/openssl/*.h	${DDK_REL_DIR}/inc/openssl/	
#	cp -f ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/win_youtube_play.c ${DDK_REL_DIR}/prj/app/demo/combo/sabbat_dual/
#	cp -f ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/win_youtube_keyword_input.c ${DDK_REL_DIR}/prj/app/demo/combo/sabbat_dual/
#	cp -f ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/win_youtube_category.c ${DDK_REL_DIR}/prj/app/demo/combo/sabbat_dual/
ifdef _WIFI_ENABLE_
	mkdir -p ${DDK_REL_DIR}/ddkt2/wifi
	cp ${DDK_DIR}/wifi/lib_*_gusb_t2.a ${DDK_REL_DIR}/ddkt2/wifi/
	cp ${DDK_DIR}/wifi/*.o ${DDK_REL_DIR}/ddkt2/wifi/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/win_wifi* ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
endif
endif

ifdef _DVBT_ENABLE_
	cp 	${DDK_DIR}/libnim_t.a ${DDK_REL_DIR}/ddkt2/libnim_t.a
endif
#   3503 bootloader releate *.a , need to add here
#	cd ${DDK_DIR}; \
#	cd blsdk ;\
#	cp -f ${BOOT_LIBS_3503} ${DDK_REL_DIR}/ddkt2/blsdk/

	cd ${BASE_DIR}/obj; \
	cp ./s3711c*.ini ${DDK_REL_DIR}/obj/

	cd ${BASE_DIR}/prj/tools; \
	cp ${ALL_TOOLS} ${DDK_REL_DIR}/prj/tools ;\
	
	cd ${BASE_DIR}/prj/tools/tools_linux; \
	cp ./* ${DDK_REL_DIR}/prj/tools/tools_linux ;\

#	cp -r loader3_AS_G2 ${DDK_REL_DIR}/${DDK_PRJ_DIR}/ ;echo
	cp -r ./rsc 	${DDK_REL_DIR}/${DDK_PRJ_DIR}
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit/Font_devanagari.h
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit/Font_telugu.h
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit/Font_bengali.h
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/font/font_indian.rar
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str/string_indian.xls
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/str/str_Hindi.h
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/str/str_Telugu.h
	-rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/rsc/str/str_Bengali.h

	cp -f ${BASE_DIR}/inc/unistd.h ${DDK_REL_DIR}/inc/unistd.h
	cp -f ${BASE_DIR}/inc/string.h ${DDK_REL_DIR}/inc/string.h
	cp -f ${BASE_DIR}/inc/stdlib.h ${DDK_REL_DIR}/inc/stdlib.h
	cp -f ${BASE_DIR}/inc/stdio.h ${DDK_REL_DIR}/inc/stdio.h

	cp -f ${BASE_DIR}/ddkt2/genlist.exe ${DDK_REL_DIR}/ddkt2/genlist.exe
	#cp -f ${BASE_DIR}/ddkt2/mod_makefile.exe ${DDK_REL_DIR}/ddkt2/mod_makefile.exe
	cp -f ${BASE_DIR}/src/path.def ${DDK_REL_DIR}/src/
	cp -f ${BASE_DIR}/src/makefile ${DDK_REL_DIR}/src/
	cp -f ${BASE_DIR}/src/compiler.def ${DDK_REL_DIR}/src/
	cp -f ./tve_hd_setting_half_mode_M3603.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./ldscript.ld ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./ldscript_3711*.ld ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./see_ldscript_m3711*.ld ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./compiler_3711c.def ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./sys_config_3711c.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./sys_memmap_3711*.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./*.m2v ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./bloader3_37*.abs ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./blocks_script_7z_*.txt ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./ota_parameter.bin ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./usb_parameter.bin ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./factory_data_*.bin ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./empty_hdcp.key ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./getpath_t2.sh	 ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./build_ota*.sh	 ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./universal_key_encrypt_M3711C.sh ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./ota_link.c ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./how_to_compile_3711*.txt ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./tve_hd_setting_full_mode.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./tve_hd_setting_half_mode.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./tve_hd_setting_half_mode_M3603.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./*.ddf ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -rf ./platform/* ${DDK_REL_DIR}/${DDK_PRJ_DIR}/platform
	cp -f ./memcfg_37*.abs ${DDK_REL_DIR}/${DDK_PRJ_DIR}/ 
	cp -f ./layout.zip ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./win_ci_*.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./win_parental* ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./sfu_test.h ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	#cp -f ./eromclient_M3527*.abs ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./post-rename*.sh ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./ad_pic*.* ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -rf ${BASE_DIR}/doc/* ${DDK_REL_DIR}/doc/
	cp -f ${BASE_DIR}/inc/api/libmp/imgdec_common.h ${DDK_REL_DIR}/inc/api/libmp/
	cp -f ./3711*ouc_loader.bin ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./reserve.bin ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./sw_init*.abs ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	#cp -rf ./mpclient ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./plugin_m*.sh ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	#cp -f ./3711c_ret_bl_gen.sh	${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -rf ./bloader_cmd		${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./root*.c ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	cp -rf ./DDR-Setting/C3711C/* ${DDK_REL_DIR}/${DDK_PRJ_DIR}/DDR-Setting/C3711C/
	
	#cp -rf ./gen_ap ${DDK_REL_DIR}/${DDK_PRJ_DIR}/
	#remove keys in gen_ap in order to force customer to modify it
	#rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/decrypt_key.bin
	#rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/decrypt_key_see.bin
	#rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/sys_sw_pair.key
	#rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/orig_bl_key.bin
	#rm -f ${DDK_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/see_test_pair.key
	#remove unused
ifndef _CAS9_VSC_ENABLE_	
	rm -f ${DDK_REL_DIR}/lib*vsc*.a
endif

	cd ${ROOT}/src; \
	  ${MAKE} ${DMOD__}; \
	  ${MAKE} ${RMOD__};\
	  ${MAKE} ${BL_RMOD__}
	cd copper_common; \
	  ${MAKE} ddk_rel
	cd gos_ui; \
	  ${MAKE} ddk_rel
ifeq ($(FSC_APP_DIR), fsc_ap)
	cd fsc_ap;\
	${MAKE} ddk_rel
endif 
  # for temp cxd2856 rel
	mkdir -p ${DDK_REL_DIR}/src/lld/nim/combo/cxd2856
	cp -rf ${BASE_DIR}/src/lld/nim/combo/cxd2856/*  ${DDK_REL_DIR}/src/lld/nim/combo/cxd2856/
	
	rm -f ${DDK_REL_DIR}/ddkt2/libsee*.a
	rm -f ${DDK_REL_DIR}/ddkt2/plugin/*.o
	rm -f ${DDK_REL_DIR}/ddkt2/plugin/*.bak
	
	rm -f ${DDK_REL_DIR}/ddkt2/lib*m32.a
	# Sync files AUI projects need to aui_demo
	#cd ${DDK_REL_DIR}/${DDK_PRJ_DIR_AUI};\
	#	${MAKE} path_t2  aui_sync_3711c

	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/consonant_str.h
	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/Devanagari.c
	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/telugu_combination.h
	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/Telugu.c
	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/bengali_combination.h
	-rm -f ${DDK_REL_DIR}/src/lib/libcommon/libchar/Bengali.c

	-rm -f ${DDK_REL_DIR}/ddkt2/libplus_*open.a

indian_lang_rel:
	mkdir -p ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}
	mkdir -p ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit
	mkdir -p ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/font
	mkdir -p ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str
	mkdir -p ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/str
	mkdir -p ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	
	cp -f ./rsc/font_4bit/Font_devanagari.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit
	cp -f ./rsc/font_4bit/Font_telugu.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit
	cp -f ./rsc/font_4bit/Font_bengali.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/font_4bit
	cp -f ./rsc/package/font/font_indian.rar ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/font
	cp -f ./rsc/package/str/osd_rsd_def.txt ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str
	cp -f ./rsc/package/str/string_bin_config.txt ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str
	cp -f ./rsc/package/str/string_indian.xls ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str
	cp -f ./rsc/package/str/string.txt ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/package/str
	cp -f ./rsc/str/str_Hindi.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/str
	cp -f ./rsc/str/str_Telugu.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/str
	cp -f ./rsc/str/str_Bengali.h ${INDIAN_LANG_REL_DIR}/${DDK_PRJ_DIR}/rsc/str
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/consonant_str.h ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/Devanagari.c ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/telugu_combination.h ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/Telugu.c ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/bengali_combination.h ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar
	cp -f ${BASE_DIR}/src/lib/libcommon/libchar/Bengali.c ${INDIAN_LANG_REL_DIR}/src/lib/libcommon/libchar

cas7_rel:
	mkdir -p ${CAS7_REL_DIR}/ddkt2
	mkdir -p ${CAS7_REL_DIR}/inc
	mkdir -p ${CAS7_REL_DIR}/inc/api
	mkdir -p ${CAS7_REL_DIR}/inc/api/libtsi
	mkdir -p ${CAS7_REL_DIR}/inc/api/libcas
	mkdir -p ${CAS7_REL_DIR}/${DDK_PRJ_DIR}

	cp -f ${BASE_DIR}/inc/api/libtsi/db_3l_T.h ${CAS7_REL_DIR}/inc/api/libtsi/
	cp -r ${BASE_DIR}/inc/freetype2 ${CAS7_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/api/libcas/conax_mmi.h ${CAS7_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas_ca_info.h ${CAS7_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas.h ${CAS7_REL_DIR}/inc/api/libcas/
	cp -r ${BASE_DIR}/inc/api/libge ${CAS7_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/ddkt2/libfreetype_m32.a ${CAS7_REL_DIR}/ddkt2/
	cp -r conax_ap7 ${CAS7_REL_DIR}/${DDK_PRJ_DIR}/
	# Copy release code to aui_demo
	cp -rf ${CAS7_REL_DIR}/${DDK_PRJ_DIR} ${CAS7_REL_DIR}/${DDK_PRJ_DIR_AUI}
	
cas9_rel:
	mkdir -p ${CAS9_REL_DIR}/ddkt2
	mkdir -p ${CAS9_REL_DIR}/ddkt2/blsdk
	mkdir -p ${CAS9_REL_DIR}/inc
	mkdir -p ${CAS9_REL_DIR}/inc/api
	mkdir -p ${CAS9_REL_DIR}/inc/api/libtsi
	mkdir -p ${CAS9_REL_DIR}/inc/api/libcas
	mkdir -p ${CAS9_REL_DIR}/inc/api/libcas/cas9
	mkdir -p ${CAS9_REL_DIR}/${DDK_PRJ_DIR}

#	cp -f ${DDK_DIR}/blsdk/boot35.o ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/boot_see*.o ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libboot_m35.a ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libslot36f.a ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libslot3602f.a ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libslothd_ct1628.a ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libslothd_fd650.a ${CAS9_REL_DIR}/ddkt2/blsdk
#	cp -f ${DDK_DIR}/blsdk/libblsee*.a ${CAS9_REL_DIR}/ddkt2/blsdk
	cp -f ${BASE_DIR}/inc/api/libtsi/db_3l_T.h ${CAS9_REL_DIR}/inc/api/libtsi/
	cp -r ${BASE_DIR}/inc/freetype2 ${CAS9_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/api/libcas/cas9/cas9.h ${CAS9_REL_DIR}/inc/api/libcas/cas9/
	cp -f ${BASE_DIR}/inc/api/libcas/conax_mmi.h ${CAS9_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas_ca_info.h ${CAS9_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas.h ${CAS9_REL_DIR}/inc/api/libcas/
	cp -r ${BASE_DIR}/inc/api/libge ${CAS9_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/ddkt2/libfreetype_m32.a ${CAS9_REL_DIR}/ddkt2/
	cp -r conax_ap ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/
#	cp -r loader3_hw_security ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/
	#remove keys in release DIR in order to force customer to modify it
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/decrypt_key.bin
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/sys_sw_pair.key
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/orig_bl_key.bin
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/see_test_pair.key
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/see_sw_pair.key
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/see_sw_uk.bin
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/see_rsa_key_pair.key
	#remove unused lib 
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/libconax_m32*
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/libupg_rs232_m32*	
	#remove vsc related
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/signed_vsc_see_se_*
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/libconax_vsc*.a
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/libconax_scu*.a
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/libupg_rs232_*vsc*.a
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/vsc_see_se*
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/*_vsc*.sh
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/*_vsc*.bat
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/*_vsc*.ini
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/2nd_level_public_key/public_key*.vsc	
	rm -rf ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/conax_vsc
	rm -f ${CAS9_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/ALi_SEE*
	# Copy release code to aui_demo
	cp -rf ${CAS9_REL_DIR}/${DDK_PRJ_DIR} ${CAS9_REL_DIR}/${DDK_PRJ_DIR_AUI}

cas9_vsc_rel:
	mkdir -p ${CAS9_VSC_REL_DIR}/ddkt2
	mkdir -p ${CAS9_VSC_REL_DIR}/ddkt2/blsdk
	mkdir -p ${CAS9_VSC_REL_DIR}/inc
	mkdir -p ${CAS9_VSC_REL_DIR}/inc/api
	mkdir -p ${CAS9_VSC_REL_DIR}/inc/api/libtsi
	mkdir -p ${CAS9_VSC_REL_DIR}/inc/api/libcas
	mkdir -p ${CAS9_VSC_REL_DIR}/inc/api/libcas/cas9
	mkdir -p ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}
	mkdir -p ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap
	mkdir -p ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/2nd_level_public_key

	cp -f ${BASE_DIR}/inc/api/libtsi/db_3l_T.h ${CAS9_VSC_REL_DIR}/inc/api/libtsi/
	cp -r ${BASE_DIR}/inc/freetype2 ${CAS9_VSC_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/api/libcas/cas9/cas9.h ${CAS9_VSC_REL_DIR}/inc/api/libcas/cas9/
	cp -f ${BASE_DIR}/inc/api/libcas/conax_mmi.h ${CAS9_VSC_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas_ca_info.h ${CAS9_VSC_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas.h ${CAS9_VSC_REL_DIR}/inc/api/libcas/
	cp -r ${BASE_DIR}/inc/api/libge ${CAS9_VSC_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/ddkt2/libfreetype_m32.a ${CAS9_VSC_REL_DIR}/ddkt2/
	#cp -r conax_ap ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/
	cp -r vsc_rap ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ./see_ldscript_M3505_VSC.ld ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/
	#remove keys in release DIR in order to force customer to modify it
	#rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/decrypt_key.bin
	#rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/decrypt_key_see.bin
	#rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/sys_sw_pair.key
	#rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/orig_bl_key.bin
	#rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/see_test_pair.key
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/2nd_level_public_key/public_key_m3527.der ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/2nd_level_public_key/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/libconax_vsc.a ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/libconax_scu.a ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/libupg_rs232_*vsc.a ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.c ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.h ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.sh ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.bat ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.ini ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.exe ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.cmd ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/*.cfg ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/Makefile ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/ca_data.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/ejtag.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/key_store.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/signed_vsc_see_se_M3715_fake.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/stb_info*.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/see_bl_origin*.abs ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/vsc_test_pair.pub ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/C3711C_*.txt ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/	
	#cp -rf ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/conax_vsc ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/AUX_Code_*3527*.abs ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/root_key_pair* ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/ext_key_pair* ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/public*.key ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/BL_Encrypted* ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/	
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/vsc_see_se_fake.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/to_besigned_boot_see_bl*.abs ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/signed_ota_app_see*.abs ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/padding_header.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/conax_ap/static_data_hdr.bin ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/

	rm -f ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR}/conax_ap/ALi_SEE*
	# Copy release code to aui_demo
	cp -rf ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR} ${CAS9_VSC_REL_DIR}/${DDK_PRJ_DIR_AUI}

cas9_vsc_env_rel:
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/src/vsc
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/obj
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/inc
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/inc/api
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/inc/bus
	mkdir -p ${CAS9_VSC_ENV_REL_DIR}/inc/hld
	
	cp -rf ${BASE_DIR}/src/vsc ${CAS9_VSC_ENV_REL_DIR}/src/
	cp -rf ${BASE_DIR}/src/makefile ${CAS9_VSC_ENV_REL_DIR}/src/
	cp -rf ${BASE_DIR}/inc/vsc ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/hal ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/osal ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/hld/crypto ${CAS9_VSC_ENV_REL_DIR}/inc/hld/
	cp -rf ${BASE_DIR}/inc/hld/dsc ${CAS9_VSC_ENV_REL_DIR}/inc/hld/
	cp -rf ${BASE_DIR}/inc/hld/trng ${CAS9_VSC_ENV_REL_DIR}/inc/hld/
	cp -rf ${BASE_DIR}/inc/vsc ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/hld/hld_dev.h ${CAS9_VSC_ENV_REL_DIR}/inc/hld/
	cp -rf ${BASE_DIR}/inc/bus/otp ${CAS9_VSC_ENV_REL_DIR}/inc/bus/
	cp -rf ${BASE_DIR}/inc/api/libc ${CAS9_VSC_ENV_REL_DIR}/inc/api/
	cp -rf ${BASE_DIR}/inc/sys_define.h ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/basic_types.h ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/retcode.h ${CAS9_VSC_ENV_REL_DIR}/inc/
	cp -rf ${BASE_DIR}/inc/types.h ${CAS9_VSC_ENV_REL_DIR}/inc/

c1800a_ca_rel:    
	mkdir -p ${C1800A_CA_REL_DIR}/ddkt2
	mkdir -p ${C1800A_CA_REL_DIR}/ddkt2/blsdk
	mkdir -p ${C1800A_CA_REL_DIR}/inc
	mkdir -p ${C1800A_CA_REL_DIR}/inc/api
	mkdir -p ${C1800A_CA_REL_DIR}/inc/api/libtsi
	mkdir -p ${C1800A_CA_REL_DIR}/inc/api/libcas
	mkdir -p ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}
	mkdir -p ${C1800A_CA_REL_DIR}/aui/samples/sample_src/
	
	cp -f ${BASE_DIR}/alici/other/nstv_sample/aui_test_stream_decrypt.c ${C1800A_CA_REL_DIR}/aui/samples/sample_src/
	cp -f ${BASE_DIR}/inc/api/libtsi/db_3l_T.h ${C1800A_CA_REL_DIR}/inc/api/libtsi/
	cp -r ${BASE_DIR}/inc/freetype2 ${C1800A_CA_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas_ca_info.h ${C1800A_CA_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas.h ${C1800A_CA_REL_DIR}/inc/api/libcas/
	cp -r ${BASE_DIR}/inc/api/libge ${C1800A_CA_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/ddkt2/libfreetype_m32.a ${C1800A_CA_REL_DIR}/ddkt2/
	cp -r ird_ap ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/DVB_SUIT/boot_temp/dvtsuit/USER/*.cfg ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap
	#remove keys in release DIR in order to force customer to modify it
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/decrypt_key.bin
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/decrypt_key_see.bin
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/sys_sw_pair.key
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/orig_bl_key.bin
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/see_test_pair.key
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/see_sw_pair.key
	rm -rf ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/temporary
	rm -rf ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/conax_ap
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/common/see_bl.abs
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/common/cryptoclient*
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/common/see_bin.abs
	rm -rf ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/teeaux
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/tool/cryptoclient*
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/tool_linux/cryptoclient*
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/keys/abl*
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/tee_auxcode.abs
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/ifcp_root_rsa.key
	rm -f ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/ird_ap/generate_boot_total_abs/see_sw_key_pair.der.pub
	
	cp -f compiler_3711c.def ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/
	#enable gen macro
	sed -i "s/_CAS9_CA_ENABLE_ = 1/#_CAS9_CA_ENABLE_ = 1/g" 	${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	sed -i "s/#_GEN_CA_ENABLE_ = 1/_GEN_CA_ENABLE_ = 1/g" 	${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	sed -i "s/#_USE_SIGNED_BL_SEE = 1/_USE_SIGNED_BL_SEE = 1/g" 	${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	
	# Copy release code to aui_demo
	cp -rf ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR} ${C1800A_CA_REL_DIR}/${DDK_PRJ_DIR_AUI}	
	
gen_ca_rel:
	mkdir -p ${GEN_CA_REL_DIR}/ddkt2
	mkdir -p ${GEN_CA_REL_DIR}/ddkt2/blsdk
	mkdir -p ${GEN_CA_REL_DIR}/inc
	mkdir -p ${GEN_CA_REL_DIR}/inc/api
	mkdir -p ${GEN_CA_REL_DIR}/inc/api/libtsi
	mkdir -p ${GEN_CA_REL_DIR}/inc/api/libcas
	mkdir -p ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}
	mkdir -p ${GEN_CA_REL_DIR}/aui/samples/sample_src/
	
	cp -f ${BASE_DIR}/alici/other/nstv_sample/aui_test_stream_decrypt.c ${GEN_CA_REL_DIR}/aui/samples/sample_src/
	cp -f ${BASE_DIR}/inc/api/libtsi/db_3l_T.h ${GEN_CA_REL_DIR}/inc/api/libtsi/
	cp -r ${BASE_DIR}/inc/freetype2 ${GEN_CA_REL_DIR}/inc/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas_ca_info.h ${GEN_CA_REL_DIR}/inc/api/libcas/
	cp -f ${BASE_DIR}/inc/api/libcas/mcas.h ${GEN_CA_REL_DIR}/inc/api/libcas/
	cp -r ${BASE_DIR}/inc/api/libge ${GEN_CA_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/ddkt2/libfreetype_m32.a ${GEN_CA_REL_DIR}/ddkt2/
	#cp -f ./universal_key_encrypt_M3515_gen.sh ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}
	cp -r gen_ap ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/
	#remove keys in release DIR in order to force customer to modify it
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/decrypt_key.bin
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/decrypt_key_see.bin
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/sys_sw_pair.key
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/sys_sw_pair.der
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/orig_bl_key.bin
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/see_test_pair.key
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/see_sw_pair.key
	rm -rf ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/temporary
	rm -rf ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/conax_ap
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/common/see_bl.abs
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/common/cryptoclient*
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/common/see_bin.abs
	rm -rf ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/teeaux
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/tool/cryptoclient*
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/tool_linux/cryptoclient*
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/keys/abl*
	rm -f ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/gen_ap/generate_boot_total_abs/tee_auxcode.abs
	
	cp -f compiler_3711c.def ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/
	#enable gen macro
	sed -i "s/^_CAS9_CA_ENABLE_ = 1/#_CAS9_CA_ENABLE_ = 1/g" 	${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	sed -i "s/^#_GEN_CA_ENABLE_ = 1/_GEN_CA_ENABLE_ = 1/g" 	${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	sed -i "s/^#_USE_SIGNED_BL_SEE = 1/_USE_SIGNED_BL_SEE = 1/g" 	${GEN_CA_REL_DIR}/${DDK_PRJ_DIR}/compiler_3711c.def
	
	# Copy release code to aui_demo
	cp -rf ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR} ${GEN_CA_REL_DIR}/${DDK_PRJ_DIR_AUI}	

plugin_rel:
	mkdir -p ${PLUGIN_REL_DIR}/ddkt2
	mkdir -p ${PLUGIN_REL_DIR}/ddkt2/plugin
	mkdir -p ${PLUGIN_REL_DIR}/ddkt2/plugin/t2
	mkdir -p ${PLUGIN_REL_DIR}/ddkt2/plugin_ext

	cp -f ${DDK_DIR}/plugin/t2/*.* ${PLUGIN_REL_DIR}/ddkt2/plugin/t2
	cp -f ${DDK_DIR}/plugin_ext/*.* ${PLUGIN_REL_DIR}/ddkt2/plugin_ext/
ifdef _WIFI_ENABLE_
	mkdir -p ${DDK_REL_DIR}/ddkt2/wifi
	cp ${DDK_DIR}/wifi/lib_*_gusb.a ${DDK_REL_DIR}/ddkt2/wifi/
	cp ${DDK_DIR}/wifi/*.o ${DDK_REL_DIR}/ddkt2/wifi/
endif

bakcp:
	cp -f ${DDK_DIR}/plugin/lib_flv.o.bak ${DDK_DIR}/plugin/lib_flv.o
	cp -f ${DDK_DIR}/plugin/lib_con_amr.o.bak ${DDK_DIR}/plugin/lib_con_amr.o
ifdef _RD_DEBUG_
	cp -f ${DDK_DIR}/plugin/plugin_eac3.o.bak ${DDK_DIR}/plugin/plugin_eac3.o
	rm -f ${DDK_DIR}/plugin/plugin_ac32c.o
endif

dvb2ip_rel:
	mkdir -p ${DVB2IP_REL_DIR}/ddkt2/	
	mkdir -p ${DVB2IP_REL_DIR}/inc
	mkdir -p ${DVB2IP_REL_DIR}/inc/api
	mkdir -p ${DVB2IP_REL_DIR}/inc/api/libsat2ip
	mkdir -p ${DVB2IP_REL_DIR}/inc/api/libupnp
	mkdir -p ${DVB2IP_REL_DIR}/inc/udi/pthread
	mkdir -p ${DVB2IP_REL_DIR}/src/udi/pthread
	mkdir -p ${DVB2IP_REL_DIR}/${DDK_PRJ_DIR}/	
	
	cp -r ${BASE_DIR}/ddkt2/libupnp.a   ${DVB2IP_REL_DIR}/ddkt2/
	
	cp -r ${BASE_DIR}/inc/api/libupnp ${DVB2IP_REL_DIR}/inc/api/
	cp -r ${BASE_DIR}/inc/udi/pthread ${DVB2IP_REL_DIR}/inc/udi/	
	
	cp -r ${BASE_DIR}/src/udi/pthread/*.c ${DVB2IP_REL_DIR}/src/udi/pthread/
	cp -r ${BASE_DIR}/src/udi/pthread/*.h ${DVB2IP_REL_DIR}/src/udi/pthread/
	cp -r ${BASE_DIR}/src/udi/pthread/makefile ${DVB2IP_REL_DIR}/src/udi/pthread/	

	cp -f ${BASE_DIR}/inc/api/libsat2ip/*.h ${DVB2IP_REL_DIR}/inc/api/libsat2ip/
	cp -f ${BASE_DIR}/ddkt2/libprovider.a ${DVB2IP_REL_DIR}/ddkt2/	
	cp -f ${BASE_DIR}/ddkt2/libsat2ipserver.a ${DVB2IP_REL_DIR}/ddkt2/

	rm -f ./sat2ip/*.o
	rm -f ./sat2ip/*.mk
	cp -r sat2ip ${DVB2IP_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f how_to_compile_sat2ip.txt ${DVB2IP_REL_DIR}/${DDK_PRJ_DIR}/how_to_compile_sat2ip.txt
	
sat2ip_rel:
	mkdir -p ${SAT2IP_REL_DIR}/ddkt2
	mkdir -p ${SAT2IP_REL_DIR}/inc
	mkdir -p ${SAT2IP_REL_DIR}/inc/api
	mkdir -p ${SAT2IP_REL_DIR}/inc/api/libsat2ip
	mkdir -p ${SAT2IP_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ${BASE_DIR}/inc/api/libsat2ip/*.h ${SAT2IP_REL_DIR}/inc/api/libsat2ip/
	cp -f ${BASE_DIR}/ddkt2/libprovider.a ${SAT2IP_REL_DIR}/ddkt2/	
	cp -f ${BASE_DIR}/ddkt2/libsat2ipserver.a ${SAT2IP_REL_DIR}/ddkt2/
	rm -f ./sat2ip/*.o
	rm -f ./sat2ip/*.mk
	cp -r sat2ip ${SAT2IP_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f how_to_compile_sat2ip.txt ${SAT2IP_REL_DIR}/${DDK_PRJ_DIR}/how_to_compile_sat2ip.txt

sat2ip_client_rel:
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/ddkt2
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/inc
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/inc/api
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/inc/api/libsat2ip
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/src
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/src/lib
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet/libsat2ip
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet/libsat2ip/client
	mkdir -p ${SAT2IPCLIENT_REL_DIR}/${DDK_PRJ_DIR}/

	cp -f ${BASE_DIR}/inc/api/libsat2ip/*.h ${SAT2IPCLIENT_REL_DIR}/inc/api/libsat2ip
	cp -f ${BASE_DIR}/ddkt2/libsat2ipclient.a ${SAT2IPCLIENT_REL_DIR}/ddkt2/
	cp -f ${BASE_DIR}/src/lib/libapplet/libsat2ip/client/*.h ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet/libsat2ip/client
	cp -f ${BASE_DIR}/src/lib/libapplet/libsat2ip/client/*.c ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet/libsat2ip/client
	cp -f ${BASE_DIR}/src/lib/libapplet/libsat2ip/client/makefile ${SAT2IPCLIENT_REL_DIR}/src/lib/libapplet/libsat2ip/client
	rm -f ./sat2ip/*.o
	rm -f ./sat2ip/*.mk
	cp -r sat2ip ${SAT2IPCLIENT_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f how_to_compile_sat2ip.txt ${SAT2IPCLIENT_REL_DIR}/${DDK_PRJ_DIR}/how_to_compile_sat2ip.txt

play_rel:
	mkdir -p ${PLAY_REL_DIR}/${DDK_PRJ_DIR}
	
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/win_play.c ${PLAY_REL_DIR}/${DDK_PRJ_DIR}/

intest_rel:
	mkdir -p ${INTEST_REL_DIR}/${DDK_PRJ_DIR}
	
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/win_internal_test.c ${INTEST_REL_DIR}/${DDK_PRJ_DIR}/

wifistorage_rel:
	mkdir -p ${WIFISTORAGE_REL_DIR}/ddkt2
	mkdir -p ${WIFISTORAGE_REL_DIR}/inc
	mkdir -p ${WIFISTORAGE_REL_DIR}/inc/api
	mkdir -p ${WIFISTORAGE_REL_DIR}/inc/api/libwifistorage

	cp -f ${BASE_DIR}/inc/api/libwifistorage/*.h ${WIFISTORAGE_REL_DIR}/inc/api/libwifistorage/	
#	cp -f ${BASE_DIR}/ddkt2/libhttpmgserver.a ${WIFISTORAGE_REL_DIR}/ddkt2/
	cp -f ${BASE_DIR}/ddkt2/libwifistorage.a ${WIFISTORAGE_REL_DIR}/ddkt2/
	cp -f ${BASE_DIR}/ddkt2/libjson.a ${WIFISTORAGE_REL_DIR}/ddkt2/

alicast_rel:
	mkdir -p ${ALICAST_REL_DIR}/ddkt2
	mkdir -p ${ALICAST_REL_DIR}/ddkt2/wfd
	mkdir -p ${ALICAST_REL_DIR}/inc
	mkdir -p ${ALICAST_REL_DIR}/inc/api
	mkdir -p ${ALICAST_REL_DIR}/inc/api/libwfd
	mkdir -p ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}	

	cp -f ${BASE_DIR}/inc/api/libwfd/*.h ${ALICAST_REL_DIR}/inc/api/libwfd/	
#	cp -f ${BASE_DIR}/ddkt2/wfd/lib_ali_pthread.a ${ALICAST_REL_DIR}/ddkt2/wfd/
	cp -f ${BASE_DIR}/ddkt2/wfd/libhdcp2.a ${ALICAST_REL_DIR}/ddkt2/wfd/
	cp -f ${BASE_DIR}/ddkt2/wfd/libnmpwfd.a ${ALICAST_REL_DIR}/ddkt2/wfd/
	cp -f ${BASE_DIR}/ddkt2/wfd/libopenssl.a ${ALICAST_REL_DIR}/ddkt2/wfd/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/win_alicast.c ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/win_wifi_direct_setting.c ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/blocks_script_7z_sabbat_*alicast.txt ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}/
	cp -r ${BASE_DIR}/${DDK_PRJ_DIR}/empty_hdcp20.key ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}/
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/how_to_compile_alicast.txt ${ALICAST_REL_DIR}/${DDK_PRJ_DIR}/how_to_compile_alicast.txt

wifi_direct_rel:
	mkdir -p ${WIFIDIRECT_REL_DIR}/ddkt2
	mkdir -p ${WIFIDIRECT_REL_DIR}/ddkt2/wifi
	mkdir -p ${WIFIDIRECT_REL_DIR}/${DDK_PRJ_DIR}	

	cp -f ${BASE_DIR}/ddkt2/wifi/lib_mt7601_p2p_gusb.a ${WIFIDIRECT_REL_DIR}/ddkt2/wifi/
	cp -f win_wifi_direct_setting.c ${WIFIDIRECT_REL_DIR}/${DDK_PRJ_DIR}/win_wifi_direct_setting.c

# ALiPlay 2.0
ifdef _ALI_PLAY_VERSION_
SRCS += aliplay_service.c win_aliplay_pairing.c
#RMOD += LIB_ALI_PLAY
#LIBS += ${BASE_DIR}/src/lib/libapplet/libaliplay/libaliplay.a
#LIBS += ${BASE_DIR}/src/lib/libapplet/libaliplay/json-c-0.9/OUTPUT/lib/libjson.a
endif


youtube_rel:
	#release youtube
	mkdir -p ${YT_REL_DIR}/ddkt2/
	mkdir -p ${YT_REL_DIR}/inc/api/libyoutube
	mkdir -p ${YT_REL_DIR}/inc/openssl/
	mkdir -p ${YT_REL_DIR}/inc/api/libyoutube/
	mkdir -p ${YT_REL_DIR}/${DDK_PRJ_DIR}/
	
	cp -f ${DDK_DIR}/libyoutube.a ${YT_REL_DIR}/ddkt2/
	cp -f ${BASE_DIR}/inc/api/libyoutube/*.h ${YT_REL_DIR}/inc/api/libyoutube/
	cp -rf ${BASE_DIR}/inc/api/libjson	${YT_REL_DIR}/inc/api/
	cp -f ${BASE_DIR}/inc/openssl/*.h	${YT_REL_DIR}/inc/openssl/	
	cp -f ${BASE_DIR}/${DDK_PRJ_DIR}/win_youtube* ${YT_REL_DIR}/${DDK_PRJ_DIR}/

pre_boot_code:
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk pre_boot_code_c0200a

pre_boot_total_area:
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk pre_boot_total_area_c0200a
	  
special_config:
	cp -f sys_memmap_3711c.h ${ROOT}/inc/sys_memmap.h

aui :
	cd ${ROOT}/aui; \
		${MAKE} aui_clean aui_test_clean aui aui_test
	
alici :
ifdef _TEST_OLD_CI_
	cd ${ROOT}/aui; \
		${MAKE} aui_nestor_clean aui_nestor
else
	cd ${ROOT}/alici; \
		${MAKE} clean all
endif

cas9:
	cd ${BASE_DIR}/src/lib/libcas/cas9/multi_secure_tds;\
	${MAKE}
ifdef _CAS9_SC_UPG_
	cp -f ${BASE_DIR}/src/lib/libcas/cas9/multi_secure_tds/libconax_scu.a ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/conax_ap/
else
ifdef _CAS9_VSC_ENABLE_
	cp -f ${BASE_DIR}/src/lib/libcas/cas9/multi_secure_tds/libconax_vsc.a ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/conax_ap/
else
	cp -f ${BASE_DIR}/src/lib/libcas/cas9/multi_secure_tds/libconax.a ${BASE_DIR}/prj/app/demo/combo/sabbat_dual/conax_ap/
endif
endif

