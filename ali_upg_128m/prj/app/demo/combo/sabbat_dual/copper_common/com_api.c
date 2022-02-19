/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_api.c
*
*    Description: The common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>

#include <bus/rtc/rtc.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/osd/osddrv.h>
#include <hld/sto/sto_dev.h>
#include <hld/scart/scart.h>
#include <hld/pmu/pmu.h>
#if (TTX_ON == 1)
#include <hld/vbi/vbi.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#endif
#if (SUBTITLE_ON == 1)
#include <hld/sdec/sdec.h>
#include <api/libsubt/lib_subt.h>
#endif

#if (ISDBT_CC==1)
#include <hld/sdec/sdec.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

#include <math.h>

#include "dev_handle.h"
#include "system_data.h"
#include "com_api.h"
#include <api/libdiseqc/lib_diseqc.h>
#include <api/libosd/osd_common.h>
#include <api/libchunk/chunk.h>
#include <api/libdb/db_interface.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_hde.h>
#endif
//modify for TTX and EPG share buffer 2011 10 10
#ifdef TTX_EPG_SHARE_MEM
#include <api/libsi/si_eit.h>
#endif
//modify end
#include <hld/dmx/dmx.h>
#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <api/libsi/psi_pmt.h>
#endif
#if (defined(_MHEG5_V20_ENABLE_) && !defined(_BUILD_OTA_E_))
#include <mh5_api/mh5_osd.h>
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
#include "../conax_ap/win_ca_mmi.h"
#include "../conax_ap/win_finger_popup.h"
#include "../conax_ap/cas_fing_msg.h"
#endif
#include <api/libclosecaption/lib_closecaption.h>

#include "../platform/board.h"
#include "system_data.h"

#include "com_api_avctl.h"

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
extern OSAL_ID bc_osd_sema_id;
extern BOOL api_bc_osd_is_displaying();
#include "../bc_ap/win_bc_osd_common.h"
#endif

#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_mmap.h>
#endif

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#if 0
#define BC_API_PRINTF        libc_printf
#else
#define BC_API_PRINTF(...)    do{}while(0)
#endif
#endif

#define INVALID_SAT_ID  0xFFFF

#ifndef _BUILD_OTA_E_
static UINT8 current_sub_mode __MAYBE_UNUSED__= 0xFF;
static UINT8 pre_sel_lang __MAYBE_UNUSED__= 0xFF;
#endif

#if (CC_ON == 1 )
static BOOL api_get_atsccc_onoff(void);
#endif

extern INT32 chchg_pause_video(struct vdec_device *vdec);
extern INT32 chchg_resume_video(struct vdec_device *vdec);
extern UINT8 ap_get_main_dmx(void);
extern INT32 chchg_show_logo(struct cc_logo_info *plogo, struct cc_device_list *dev_list);
extern UINT32 dev_get_device_sub_type(void *dev, UINT32 hld_type);

static UINT8 __MAYBE_UNUSED__ b_ttx_lang_ex = 0 ;
void osal_delay_ms(UINT32 ms)
{
    UINT32 i = 0;

    for (i = 0; i < ms; i++)
    {
        osal_delay(1000);
    }
}

void api_subt_show_onoff(BOOL onoff);

#if (ISDBT_CC==1)
void api_isdbtcc_show_onoff(BOOL onoff);
BOOL api_get_isdbtcc_onoff(void);
#endif

static void api_enable_deo_subtitle(BOOL benable)
{
#if defined  SUPPORT_CAS9 || defined  SUPPORT_CAS7
    osal_semaphore_capture(finger_osd_sema_id, TMO_FEVR);
    if(!is_fp_displaying())
    {
        osddrv_io_ctl((HANDLE)g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, benable);
    }
    osal_semaphore_release(finger_osd_sema_id);
#else
    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    BC_API_PRINTF("%s-Line-%d\n",__FUNCTION__,__LINE__);
    osal_semaphore_capture(bc_osd_sema_id, TMO_FEVR);
    BC_API_PRINTF("%s-Line-%d\n",__FUNCTION__,__LINE__);
    if(!api_bc_osd_is_displaying())
    {
        osddrv_io_ctl((HANDLE)g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, benable);
    }
    osal_semaphore_release(bc_osd_sema_id);
    #else
    osddrv_io_ctl((HANDLE)g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, benable);
#endif
#endif
}

#if (ISDBT_CC == 1)
void api_enable_deo_isdbtcc(BOOL benable)
{
    osddrv_io_ctl((HANDLE)g_osd_dev2, OSD_IO_SET_DEO_AUTO_SWITCH, benable);
}
#endif

static UINT32 m_win_count = 0;

UINT32 api_inc_wnd_count(void)
{
    m_win_count++;
    api_enable_deo_subtitle(FALSE);
#if (ISDBT_CC == 1)
    api_enable_deo_isdbtcc(FALSE);
#endif

    api_set_deo_layer(0);

    return m_win_count;
}

UINT32 api_dec_wnd_count(void)
{
    if (m_win_count)
    {
        m_win_count--;
    }
    if (0 == m_win_count)
    {
#if (SUBTITLE_ON == 1 )
        api_enable_deo_subtitle(TRUE);
#if (CC_ON == 1 )
        if (api_get_subt_onoff()|| api_get_atsccc_onoff() )    //vicky20110124
        {
            api_set_deo_layer(1);
        }
#else
        if (api_get_subt_onoff())
        {
            api_set_deo_layer(1);
        }
#endif
#endif

#if (ISDBT_CC == 1)
        api_enable_deo_isdbtcc(TRUE);
        if (api_get_isdbtcc_onoff())
        {
            api_set_deo_layer(1);
        }
#endif

    }

    return m_win_count;
}

#ifdef PARENTAL_SUPPORT
void api_lock_channel(UINT32 ch_id)
{
    //disable_process_cc(TRUE);//DON'T SHOW CC and clear
#if (SUBTITLE_ON == 1)
    api_subt_show_onoff(FALSE);
#endif
    #if (ISDBT_CC == 1)
    api_isdbtcc_show_onoff(FALSE);
    #endif
}
#endif



UINT8 api_ttxsub_getlang(struct t_ttx_lang *lang_ttx_list,struct t_subt_lang *lang_sub_list,UINT8 select,UINT8 lang_num)
{
    UINT8 i = 0;
    UINT8 stream_lang_num= 0;
    UINT8 stream_langmucode_num= 0;
    struct t_ttx_lang     *lang_txt = NULL;
    struct t_subt_lang    *lang_sub = NULL;
    const UINT8 lan_len = 3;
    UINT8 lang_code[3] = {0};
    UINT8 lang_codem[3] = {0};
    UINT8 *code = NULL;

    stream_lang_num = get_stream_lang_cnt();
    stream_langmucode_num = get_stream_langm_cnt();

    if(0 == lang_num)
    {
        return 0xFF;
    }
    if((NULL == lang_ttx_list) && (NULL == lang_sub_list))
    {
        return 0xFF;
    }

    if(1 == lang_num)
    {
        return 0;
    }
    if(select < stream_lang_num)
    {
        MEMCPY(lang_code,stream_iso_639lang_abbr[select],3);
        MEMCPY(lang_codem,lang_code,3);
        for(i=0;i<stream_langmucode_num;i++)
        {
            if(0 == MEMCMP(iso_639lang_multicode[i][0],lang_code,3))
            {
                MEMCPY(lang_codem,iso_639lang_multicode[i][1],3);
                break;
            }
        }
    }
    else
    {
        return 0;
    }

    for(i=0;i<lang_num;i++)
    {
        if(lang_ttx_list != NULL)
        {
            lang_txt = &lang_ttx_list[i];
            code = lang_txt->lang;
        }
        else
        {
            lang_sub = &lang_sub_list[i];
            code = lang_sub->lang;
        }
#ifdef SUPPORT_FRANCE_HD
        if(MEMCMP("qaa",code,3) == 0)
        {
            return i;
        }
#endif
        if((0 == MEMCMP(lang_code,code,lan_len)) || (0 == MEMCMP(lang_codem,code,lan_len)))
        {
            return i;
        }
    }
    return 0;
}


#if (TTX_ON ==1)
static BOOL m_ttx_onoff=FALSE;
#if (!defined(SUPPORT_CAS9) && !defined(SUPPORT_CAS7))
static BOOL api_get_ttx_onoff(void)
{
    return m_ttx_onoff;
}
#endif
void api_ttx_show_onoff(BOOL onoff)
{
    if (!onoff)
    {
        api_set_deo_layer(0);
    }
    ttxeng_show_on_off(onoff);

    if (onoff)
    {
        api_set_deo_layer(1);
    }
    m_ttx_onoff=onoff;
}
#endif

#if (SUBTITLE_ON == 1 )
static BOOL m_subtilte_onoff = FALSE;
BOOL api_get_subt_onoff(void)
{
    return m_subtilte_onoff;
}

void api_subt_show_onoff(BOOL onoff)
{
    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if(api_bc_osd_is_displaying())
    {
        BC_API_PRINTF("%s force skip\n",__FUNCTION__);
        return;
    }
    #endif

    if (!onoff)
    {
        api_set_deo_layer(0);
    }
    subt_show_onoff(onoff);

    if (onoff)
    {
        api_set_deo_layer(1);
    }
    m_subtilte_onoff = onoff;
}
#endif

#if (CC_ON == 1 )
static BOOL m_atsccc_onoff=FALSE; //vicky20110124

static BOOL api_get_atsccc_onoff(void) //vicky20110124
{
    return m_atsccc_onoff;
}

static void api_atsccc_show_onoff(BOOL onoff)
{
    if (!onoff)
    {
        api_set_deo_layer(0);
    }
    if(!((onoff ==TRUE) &&(sys_data_get_cc_control() ==0)&&
        (sys_data_get_dtvcc_service()==0))) //fix bug 02565
    {
        atsc_cc_show_onoff(onoff);
    }
    if (onoff)
    {
        api_set_deo_layer(1);
    }
    m_atsccc_onoff = onoff;
}

#endif

#if (ISDBT_CC==1)
    static BOOL m_isdbtcc_onoff=FALSE;
    BOOL api_get_isdbtcc_onoff(void)
    {
        return m_isdbtcc_onoff;
    }
    void api_isdbtcc_show_onoff(BOOL onoff)
    {
        if (!onoff)
        {
            api_set_deo_layer(0);
        }
        isdbtcc_show_onoff(onoff);
        if (onoff)
        {
            api_set_deo_layer(1);
        }

        m_isdbtcc_onoff = onoff;
    }
#endif

#if defined  SUPPORT_CAS9 || defined  SUPPORT_CAS7

static void osd_mode_change_to_popup(enum OSD_MODE pre_mode)
{
    UINT8 *pal_data = NULL;
    struct osdpara  topenpara;
    struct osdrect topenrect;
    struct osdrect r;
    enum tvsystem tvsys = PAL;
    struct osd_device *osd_dev = NULL;
    UINT8 pallete = 0;
	INT32 ret=ERR_FAILUE;

#if (SUBTITLE_ON == 1 )
    if((OSD_SUBTITLE == pre_mode) && (0 == current_sub_mode))
    {
    	ret=terminate_atsc_tasks();
			
		if(SUCCESS==ret)
			ret=terminate_sdec_task();
		if(SUCCESS==ret)
        api_subt_show_onoff(FALSE);
    }
#endif
#if (TTX_ON ==1)
    else if(((OSD_SUBTITLE == pre_mode) && (1 == current_sub_mode)) || (OSD_TELETEXT == pre_mode))
    {
    #ifdef TTX_BY_OSD
        ttxeng_set_init_lang(0xFF);//for exit TTXEng
        api_ttx_show_onoff(FALSE);
	#endif
        //modify for TTX and EPG share buffer 2011 10 10
    #ifdef TTX_EPG_SHARE_MEM
        extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);
        ttx_enable(FALSE);
        epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START/*buffer*/, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
    #endif//modify end
    }
#endif
#if (CC_ON == 1 )
    if(OSD_SUBTITLE == pre_mode)
    {
        if(TRUE==api_get_atsccc_onoff())    //vicky20110216
        {
            api_atsccc_show_onoff(FALSE);
        }
        #ifdef CC_BY_VBI
            cc_vbi_show_on(FALSE);
        #endif
    }
#endif

#if (ISDBT_CC == 1)
    if(pre_mode == OSD_SUBTITLE )
    {
        api_isdbtcc_show_onoff(FALSE);
    }
#endif

    vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));

#if (COLOR_N==4)
    topenpara.e_mode = OSD_4_COLOR;
#elif (COLOR_N==16)
    topenpara.e_mode = OSD_16_COLOR;
#elif (COLOR_N==256)
    topenpara.e_mode = OSD_256_COLOR;
#endif
    topenpara.u_galpha_enable = 0;
    topenpara.u_galpha = 0x0f;

    topenrect.u_left = (720 - OSD_MAX_WIDTH)>>1;
    topenrect.u_width = OSD_MAX_WIDTH;
    topenrect.u_top = OSD_STARTROW_P;
    topenrect.u_height = OSD_MAX_HEIGHT;

    r.u_left = 0;
    r.u_top = 0;
    r.u_width = OSD_MAX_WIDTH;
    r.u_height = OSD_MAX_HEIGHT;

    osd_dev = g_osd_dev2;
    osddrv_show_on_off((HANDLE)osd_dev,OSDDRV_OFF);
    osddrv_close((HANDLE)osd_dev);
    osddrv_open((HANDLE)osd_dev, &topenpara);
    osal_task_sleep(20);
    osddrv_create_region((HANDLE)osd_dev, 0, &(topenrect), NULL);
    osddrv_region_fill((HANDLE)osd_dev,0,&r, OSD_TRANSPARENT_COLOR);

    set_osd2_pos_for_tv_system(tvsys);

    pallete = sys_data_get_osd_pallete();
    /*Set pallette,for the second layer osd,it no need transparent*/
    //pal_data = (UINT8*)OSD_GetRscPallette(0x4080 | system_config.osd_set.pallete);
    pal_data = (UINT8*)osd_get_rsc_pallette(0x4080 | pallete);
    osddrv_set_pallette((HANDLE)osd_dev, pal_data, COLOR_N, OSDDRV_YCBCR);
    g_second_osd_mode = OSD_MSG_POPUP;
}

static void osd_mode_change_to_noshow(enum OSD_MODE pre_mode)
{
	INT32 ret=ERR_FAILUE;
#if (CC_ON == 1 )
    if((OSD_SUBTITLE == pre_mode) && (TRUE==api_get_atsccc_onoff()))
    {
        api_atsccc_show_onoff(FALSE);
    }
    #ifdef CC_BY_VBI
    if(OSD_SUBTITLE == pre_mode)
    {
        cc_vbi_show_on(FALSE);
    }
    #endif
#endif
#if (SUBTITLE_ON == 1 )
    if((OSD_SUBTITLE == pre_mode) && (0 == current_sub_mode))
    {
    	ret=terminate_atsc_tasks();
			
		if(SUCCESS==ret)
			ret=terminate_sdec_task();
		if(SUCCESS==ret)
        api_subt_show_onoff(FALSE);
    }
#endif
#if (TTX_ON ==1)
    else if(((OSD_SUBTITLE == pre_mode) && (1 == current_sub_mode)) || (OSD_TELETEXT == pre_mode))
    {
    #ifdef TTX_BY_OSD
        ttxeng_set_init_lang(0xFF);//for exit TTXEng
        api_ttx_show_onoff(FALSE);
	#endif
    }
//    else
#endif
    if(OSD_MSG_POPUP == pre_mode)
    {
        osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
    }
#if (ISDBT_CC == 1)
    if(OSD_SUBTITLE == pre_mode)
    {
        api_isdbtcc_show_onoff(FALSE);
    }
    else if(OSD_MSG_POPUP == pre_mode)
    {
        osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
    }
#endif
    g_second_osd_mode = OSD_NO_SHOW;
}

static void osd_mode_change_to_subtitle(UINT8 sel_lang)
{
    SYSTEM_DATA *sys_data = NULL;
    struct t_subt_lang    *sublang_list = NULL;
    UINT8 lang_num = 0;
    struct t_ttx_lang *ttxlang_list = NULL;
    UINT8 lang_numttx = 0;
    UINT8 sub_mode = 0;
	INT32 ret=ERR_FAILUE;

#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang *cclanglst;
    UINT8 b_lang_num,b_cc_lang;
#endif

    sys_data = sys_data_get();

#if (ISDBT_CC == 1)
    isdbtcc_get_language(&cclanglst, &b_lang_num);
    b_cc_lang=sys_data->osd_set.subtitle_lang;
#endif

#if(CC_ON==1) //vicky20110216
    #ifdef CC_BY_OSD
    if(1 == sys_data->osd_set.cc_display)
    {
        //if(get_dtv_cs_number()||get_cc_control_byte())
        api_atsccc_show_onoff(TRUE);
    }
    else
    {
        if(TRUE==api_get_atsccc_onoff())    //vicky20110216
        {
            api_atsccc_show_onoff(FALSE);
        }
    }
    #endif

    #ifdef CC_BY_VBI
    cc_vbi_show_on(TRUE);
    #endif

#endif

    if(1 == sys_data->osd_set.subtitle_display)
    {
        ttxlang_list = NULL;
        lang_numttx = 0;
        sub_mode = 0;


#if (TTX_ON ==1)
        if((0xFF == sel_lang)||(1 == b_ttx_lang_ex))
        {
            sub_mode = 1;
            ttxeng_get_subt_lang(&ttxlang_list, &lang_num);
            sel_lang = api_ttxsub_getlang(ttxlang_list,NULL,sys_data->lang.sub_lang,lang_num);
        }
#endif
        if(sys_data->osd_set.subtitle_lang != SUBTITLE_LANGUAGE_INVALID)
        {
            subt_get_language(&sublang_list,&lang_num);
            ttxeng_get_subt_lang(&ttxlang_list, &lang_numttx);

            if(((lang_num + lang_numttx) > 0) && (sys_data->osd_set.subtitle_lang < (lang_num + lang_numttx)))
            {
                if((lang_numttx>0) && (sys_data->osd_set.subtitle_lang <lang_numttx))
                {
                    sub_mode = 1;/*ttx-subtitle*/
                    sel_lang = sys_data->osd_set.subtitle_lang;
                }
                else if((lang_num>0) && (sys_data->osd_set.subtitle_lang <(lang_num + lang_numttx)))
                {
                    sub_mode = 0;/*normal-subtitle*/
                    sel_lang = (sys_data->osd_set.subtitle_lang - lang_numttx);
                }
            }
            else
            {
                PRINTF("NO SUBTITLE DATA.\n");
            }

        }

        if(sel_lang != 0XFF)/*subt languages exist*/
        {
            if(0 == sub_mode)
            {
        #if (CC_ON ==1)
            #ifdef CC_BY_OSD
                api_atsccc_show_onoff(FALSE);   //vicky20110216
            #endif
        #endif

        #if (TTX_ON ==1)
                api_ttx_show_onoff(FALSE);
        #endif

        #if (SUBTITLE_ON == 1)
				if(TRUE == subt_is_available())
				{
                	api_subt_show_onoff(TRUE);
                	//subt_set_language(sel_lang);
                	ret=start_atsc_tasks(sel_lang);
					if(ret==ERR_FAILUE)
						start_sdec_task(sel_lang);
				}
        #endif
            }
            else
            {
        #if (CC_ON ==1)
            #ifdef CC_BY_OSD
                api_atsccc_show_onoff(FALSE);   //vicky20110216
            #endif
        #endif

        #if (SUBTITLE_ON == 1 )
				ret=terminate_atsc_tasks();
			
				if(SUCCESS==ret)
					ret=terminate_sdec_task();
				if(SUCCESS==ret)
                api_subt_show_onoff(FALSE);
        #endif

        #if (TTX_ON ==1)
        //modify for TTX and EPG share buffer 2011 10 10
            #ifdef TTX_EPG_SHARE_MEM
                epg_off();
                epg_release();
                ttx_enable(TRUE);
            #endif//modify end
                ttx_enable(TRUE);
                api_ttx_show_onoff(TRUE);
                ttxeng_set_subt_lang(sel_lang);
        #endif
            }
        }

#if (ISDBT_CC == 1)
        //libc_printf("\nOsd mode change and lang num(%d)",bLangNum);
      	if(TRUE == isdbtcc_is_available())
		{
        	api_isdbtcc_show_onoff(TRUE);
        	if( (SUBTITLE_LANGUAGE_INVALID==b_cc_lang)  || (0>=b_lang_num) || (b_lang_num<=b_cc_lang))
        	{
            //api_isdbtcc_show_onoff(FALSE);
        	}
        	else
        	{
            	isdbtcc_set_language(b_cc_lang);
        	}
        }
#endif
        current_sub_mode = sub_mode;
    }
    else
    {
        /*Even subt off,we should still close ttx*/
    #if (TTX_ON ==1)
        api_ttx_show_onoff(FALSE);
    #endif
    }
    g_second_osd_mode = OSD_SUBTITLE;
}

static void osd_mode_change_to_teletext(void)
{
	INT32 ret=ERR_FAILUE;
#if (TTX_ON ==1)
    PRINTF("TELETEXT\n");
#if (SUBTITLE_ON == 1 )
	ret=terminate_atsc_tasks();
			
	if(SUCCESS==ret)
		ret=terminate_sdec_task();
	if(SUCCESS==ret)
    api_subt_show_onoff(FALSE);

#endif
#if (CC_ON ==1)
    #ifdef CC_BY_OSD
    api_atsccc_show_onoff(FALSE);
    #endif
#endif
    api_ttx_show_onoff(TRUE);
    g_second_osd_mode = OSD_TELETEXT;
#endif
}

enum OSD_MODE api_osd_mode_change_ex(enum OSD_MODE mode)
{/*SUBTITLE_ON,TTX_ON macro total a shit,with all project open,failure if closed*/
    enum OSD_MODE pre_mode = OSD_NO_SHOW;
    SYSTEM_DATA *sys_data = NULL;
    UINT8 sel_lang = 0;
    struct t_ttx_lang *ttxlang_list=NULL;
#if (SUBTITLE_ON == 1)
    struct t_subt_lang *sublang_list = NULL;
    UINT8 lang_num = 0;
#endif
    b_ttx_lang_ex = 0 ;

    pre_mode = g_second_osd_mode;
    sys_data = sys_data_get();
    sel_lang = 0xFF;
#if (SUBTITLE_ON == 1)
    subt_get_language(&sublang_list,&lang_num);
    sel_lang = api_ttxsub_getlang(NULL,sublang_list,sys_data->lang.sub_lang,lang_num);
    b_ttx_lang_ex = 0;
#endif
#if (TTX_ON ==1)
    if(sel_lang == 0xFF)
    {
        ttxeng_get_subt_lang(&ttxlang_list, &lang_num);
        sel_lang = api_ttxsub_getlang(ttxlang_list,NULL,sys_data->lang.sub_lang,lang_num);
        b_ttx_lang_ex= 1;
    }
#endif

    if((mode == pre_mode) && (sel_lang == pre_sel_lang))//if find subt lang change, redo osd mode change
    {
        return pre_mode;
    }
    pre_sel_lang = sel_lang;

    if(OSD_MSG_POPUP == mode)
    {
        osd_mode_change_to_popup(pre_mode);
    }
    else if(OSD_NO_SHOW == mode)
    {
        osd_mode_change_to_noshow(pre_mode);
    }
    else if(OSD_SUBTITLE == mode)
    {
        osd_mode_change_to_subtitle(sel_lang);
    }
    else if(OSD_TELETEXT == mode)
    {
        osd_mode_change_to_teletext();
    }
    else if(OSD_INVALID_MODE == mode)
    {
        g_second_osd_mode = OSD_INVALID_MODE;
    }

    return pre_mode;
}

enum OSD_MODE api_osd_mode_change(enum OSD_MODE mode)
{
    enum OSD_MODE ret_mode = OSD_NO_SHOW;
    SYSTEM_DATA *sys_data = NULL;

	if(NULL == sys_data)
	{
		;
	}
    sys_data = sys_data_get();
    ret_mode = g_second_osd_mode;

    osal_semaphore_capture(finger_osd_sema_id, TMO_FEVR);
    if(!is_fp_displaying())
    {
        ret_mode = api_osd_mode_change_ex(mode);
    }
    osal_semaphore_release(finger_osd_sema_id);
    return ret_mode;
}
#else
enum OSD_MODE api_osd_mode_change(enum OSD_MODE mode)
{/*SUBTITLE_ON,TTX_ON macro total a shit,with all project open,failure if closed*/
    enum OSD_MODE pre_mode = OSD_NO_SHOW;
    SYSTEM_DATA *sys_data = NULL;    
    UINT8 sel_lang = 0;
    static UINT8 pre_sel_lang = 0xFF;
    UINT8 pallete = 0;
    struct t_ttx_lang * __MAYBE_UNUSED__ ttxlang_list = NULL;
    UINT8 __MAYBE_UNUSED__ b_ttx_lang = 0 ;
#if (SUBTITLE_ON == 1 )    
	INT32 ret=ERR_FAILUE;
#endif
#ifndef _BUILD_OTA_E_
    struct t_subt_lang    *sublang_list = NULL;
    UINT8 lang_num = 0;
    //struct t_ttx_lang *ttxlang_list __MAYBE_UNUSED__= NULL;
    UINT8 lang_numttx __MAYBE_UNUSED__= 0;
    UINT8 sub_mode __MAYBE_UNUSED__= 0;
#endif

#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang*cclanglst;
    UINT8 b_lang_num,b_cc_lang;
#endif

    sys_data = sys_data_get();
    pre_mode = g_second_osd_mode;

#if ((SUBTITLE_ON == 1 )||(TTX_ON ==1))
   #if(defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
    if(is_fp_displaying() == TRUE)
        return pre_mode;
   #endif
#endif

    sel_lang = 0xFF;
#if (SUBTITLE_ON == 1)
    subt_get_language(&sublang_list,&lang_num);
    sel_lang = api_ttxsub_getlang(NULL,sublang_list,sys_data->lang.sub_lang,lang_num);
    b_ttx_lang = 0;
#endif
#if (TTX_ON ==1)
    if(sel_lang == 0xFF)
    {
        ttxeng_get_subt_lang(&ttxlang_list, &lang_num);
        sel_lang = api_ttxsub_getlang(ttxlang_list,NULL,sys_data->lang.sub_lang,lang_num);
        b_ttx_lang = 1;
    }
#endif

#if (ISDBT_CC == 1)
        isdbtcc_get_language(&cclanglst, &b_lang_num);
        b_cc_lang=sys_data->osd_set.subtitle_lang;
#endif

    if((mode == pre_mode) && (sel_lang == pre_sel_lang))//if find subt lang change, redo osd mode change
    {
        return pre_mode;
    }

    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    {
        if(api_bc_osd_is_displaying())
        {
            return pre_mode;
        }
        BC_API_PRINTF("%s-Line-%d\n",__FUNCTION__,__LINE__);
        osal_semaphore_capture(bc_osd_sema_id, TMO_FEVR);
        BC_API_PRINTF("%s-Line-%d\n",__FUNCTION__,__LINE__);
    }
    #endif

    pre_sel_lang = sel_lang;
    if(OSD_MSG_POPUP == mode)
    {
        UINT8* pal_data = NULL;
        struct osdpara    t_open_para;
        struct osdrect t_open_rect,r;
        enum tvsystem tvsys;
        struct osd_device *osd_dev = NULL;

        MEMSET(&t_open_para, 0x0, sizeof (t_open_para));
        MEMSET(&t_open_rect, 0x0, sizeof (t_open_rect));
        MEMSET(&r, 0x0, sizeof (r));
        MEMSET(&tvsys, 0x0, sizeof (tvsys));

#if (SUBTITLE_ON == 1 )
        if( (pre_mode == OSD_SUBTITLE) && (current_sub_mode == 0))
        {
   			ret=terminate_atsc_tasks();
			
			if(SUCCESS==ret)
				ret=terminate_sdec_task();
			if(SUCCESS==ret)
            api_subt_show_onoff(FALSE);
        }
#endif
#if (TTX_ON ==1)
        else if((pre_mode == OSD_SUBTITLE && current_sub_mode == 1) || pre_mode == OSD_TELETEXT)
        {
       #ifdef TTX_BY_OSD
            ttxeng_set_init_lang(0xFF);//for exit ttxeng
            api_ttx_show_onoff(FALSE);
	   #endif
        }
#endif
#if (CC_ON == 1 )
        if(pre_mode == OSD_SUBTITLE )
        {
            if(TRUE==api_get_atsccc_onoff())
                api_atsccc_show_onoff(FALSE);
            #ifdef CC_BY_VBI
                cc_vbi_show_on(FALSE);
            #endif
        }
#endif

#if (ISDBT_CC == 1)
        if(pre_mode == OSD_SUBTITLE )
        {
            api_isdbtcc_show_onoff(FALSE);
        }
#endif
        vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));

#if (COLOR_N==4)
        t_open_para.e_mode = OSD_4_COLOR;
#elif (COLOR_N==16)
        t_open_para.e_mode = OSD_16_COLOR;
#elif (COLOR_N==256)
        t_open_para.e_mode = OSD_256_COLOR;
#endif
        t_open_para.u_galpha_enable = 0;
        t_open_para.u_galpha = 0x0f;

        t_open_rect.u_left = (720 - OSD_MAX_WIDTH)>>1;
        t_open_rect.u_width = OSD_MAX_WIDTH;
        t_open_rect.u_top = OSD_STARTROW_P;
        t_open_rect.u_height = OSD_MAX_HEIGHT;

        r.u_left = 0;
        r.u_top = 0;
        r.u_width = OSD_MAX_WIDTH;
        r.u_height = OSD_MAX_HEIGHT;

        osd_dev = g_osd_dev2;
        osddrv_show_on_off((HANDLE)osd_dev,OSDDRV_OFF);
        osddrv_close((HANDLE)osd_dev);
        osddrv_open((HANDLE)osd_dev, &t_open_para);
        osal_task_sleep(20);
        osddrv_create_region((HANDLE)osd_dev, 0, &(t_open_rect), NULL);
        osddrv_region_fill((HANDLE)osd_dev,0,&r, OSD_TRANSPARENT_COLOR);

        set_osd2_pos_for_tv_system(tvsys);
        pallete = sys_data_get_osd_pallete();
        /*Set pallette,for the second layer osd,it no need transparent*/
        pal_data = (UINT8*)osd_get_rsc_pallette(0x4080 | pallete);
        osddrv_set_pallette((HANDLE)osd_dev, pal_data, COLOR_N, OSDDRV_YCBCR);
        g_second_osd_mode = OSD_MSG_POPUP;
    }
    else if(mode == OSD_NO_SHOW)
    {
#if (CC_ON == 1 )
        if((pre_mode==OSD_SUBTITLE) && (TRUE==api_get_atsccc_onoff()))
        {
            api_atsccc_show_onoff(FALSE);
        }
        #ifdef CC_BY_VBI
            if(pre_mode==OSD_SUBTITLE)
            {
                cc_vbi_show_on(FALSE);
            }
        #endif
#endif
#if (SUBTITLE_ON == 1)
        if((TRUE == api_get_subt_onoff()) && (OSD_SUBTITLE == pre_mode) && (0 == current_sub_mode))
        {
		    ret=terminate_atsc_tasks();
			
			if(SUCCESS==ret)
				ret=terminate_sdec_task();
			if(SUCCESS==ret)
            api_subt_show_onoff(FALSE);
        }
        else if(((TRUE == api_get_subt_onoff() ||TRUE == api_get_ttx_onoff())&&
          (pre_mode == OSD_SUBTITLE && current_sub_mode == 1)) 
          || (pre_mode == OSD_TELETEXT))
        {
        #ifdef TTX_BY_OSD
            ttxeng_set_init_lang(0xFF);//for exit ttxeng
            api_ttx_show_onoff(FALSE);
		#endif
            //modify for TTX and EPG share buffer 2011 10 10
            #ifdef TTX_EPG_SHARE_MEM
            extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);
            ttx_enable(FALSE);
            epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
            #endif
            //modify end
        }
        else if(TRUE == api_get_subt_onoff() && pre_mode == OSD_MSG_POPUP)
        {
            osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
        }
#endif
#if (ISDBT_CC == 1)
        if(TRUE == api_get_isdbtcc_onoff()&& pre_mode == OSD_SUBTITLE )
        {
            api_isdbtcc_show_onoff(FALSE);
        }
        else if(TRUE == isdbtcc_is_available() && pre_mode == OSD_MSG_POPUP)
        {
            osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
        }
#endif
        #ifndef _BUILD_OTA_E_
        current_sub_mode = 0xFF;
        #endif
        g_second_osd_mode = OSD_NO_SHOW;
    }
    else if(mode == OSD_SUBTITLE)
    {
        #if(CC_ON==1)
        #ifdef CC_BY_OSD
        if(1 == sys_data->osd_set.cc_display)
        {
            //if(get_dtv_cs_number()||get_cc_control_byte())
                api_atsccc_show_onoff(TRUE);
        }
        else
        {
            if(TRUE==api_get_atsccc_onoff())
                api_atsccc_show_onoff(FALSE);
        }
        #endif
        #ifdef CC_BY_VBI
            cc_vbi_show_on(TRUE);
        #endif

        #endif
        if(1 == sys_data->osd_set.subtitle_display)
        {
            //struct t_ttx_lang *ttxlang_list = NULL;
            UINT8 __MAYBE_UNUSED__ lang_numttx = 0;
            UINT8 __MAYBE_UNUSED__ sub_mode = 0;

            //ttxlang_list = NULL;
            lang_numttx = 0;
            sub_mode = 0;

#if (TTX_ON ==1)            
            if((0xFF == sel_lang)||(1==b_ttx_lang))
            {
                sub_mode = 1;
                ttxeng_get_subt_lang(&ttxlang_list, &lang_num);
                sel_lang = api_ttxsub_getlang(ttxlang_list,NULL,sys_data->lang.sub_lang,lang_num);
            }
#endif

#if (SUBTITLE_ON == 1)
            if(sys_data->osd_set.subtitle_lang != SUBTITLE_LANGUAGE_INVALID)
            {
                subt_get_language(&sublang_list,&lang_num);
                ttxeng_get_subt_lang(&ttxlang_list, &lang_numttx);

                if(((lang_num + lang_numttx) > 0) && (sys_data->osd_set.subtitle_lang < (lang_num + lang_numttx)))
                {
                    if((lang_numttx>0) && (sys_data->osd_set.subtitle_lang <lang_numttx))
                    {
                        sub_mode = 1;/*ttx-subtitle*/
                        sel_lang = sys_data->osd_set.subtitle_lang;
                    }
                    else if((lang_num>0) && (sys_data->osd_set.subtitle_lang <(lang_num + lang_numttx)))
                    {
                        sub_mode = 0;/*normal-subtitle*/
                        sel_lang = (sys_data->osd_set.subtitle_lang - lang_numttx);
                    }
                }
                else
                {
                    PRINTF("NO SUBTITLE DATA.\n");
                }

            }

            if(sel_lang != 0XFF)/*subt languages exist*/
            {
                if(sub_mode == 0)
                {
#if (CC_ON ==1)
    #ifdef CC_BY_OSD
        api_atsccc_show_onoff(FALSE);
        #endif
#endif
#if (TTX_ON ==1)
                    api_ttx_show_onoff(FALSE);
#endif

#if (SUBTITLE_ON == 1)
                 if(TRUE == subt_is_available())
                 {
                    api_subt_show_onoff(TRUE);
                    //subt_set_language(sel_lang);                                    	
                	ret=start_atsc_tasks(sel_lang);
					if(ret==ERR_FAILUE)
						start_sdec_task(sel_lang);
                 }
#endif
                }
                else
                {
#if (CC_ON ==1)
    #ifdef CC_BY_OSD
        api_atsccc_show_onoff(FALSE);
        #endif
#endif
#if (SUBTITLE_ON == 1 )
	  ret=terminate_atsc_tasks();
			
	  if(SUCCESS==ret)
	  	ret=terminate_sdec_task();
	  
	  if(SUCCESS==ret)					
                    api_subt_show_onoff(FALSE);
#endif

#if (TTX_ON ==1)
//modify for TTX and EPG share buffer 2011 10 10
#ifdef TTX_EPG_SHARE_MEM
                    epg_off();
                    epg_release();
                        ttx_enable(TRUE);
#endif//modify end
                    ttx_enable(TRUE);
                    api_ttx_show_onoff(TRUE);
                    ttxeng_set_subt_lang(sel_lang);
#endif
                }
            }
#endif

#if (ISDBT_CC == 1)
        //libc_printf("\nOsd mode change and lang num(%d)",bLangNum);
    if(TRUE == isdbtcc_is_available())
    {
        api_isdbtcc_show_onoff(TRUE);
        if((SUBTITLE_LANGUAGE_INVALID==b_cc_lang) || (0>=b_lang_num) || (b_lang_num<=b_cc_lang))
        {
            //api_isdbtcc_show_onoff(FALSE);
        }
        else
        {
            isdbtcc_set_language(b_cc_lang);
        }
    }
#endif
        #ifndef _BUILD_OTA_E_
        current_sub_mode = sub_mode;
        #endif
        }
        else
        {
            /*Even subt off,we should still close ttx*/
#if (TTX_ON ==1)
                api_ttx_show_onoff(FALSE);
#endif
        }
        g_second_osd_mode = OSD_SUBTITLE;
    }
    else if(mode == OSD_TELETEXT)
    {
#if (TTX_ON ==1)
        PRINTF("TELETEXT\n");
#if (SUBTITLE_ON == 1 )
			ret=terminate_atsc_tasks();
			
			if(SUCCESS==ret)
				ret=terminate_sdec_task();
			if(SUCCESS==ret)
        api_subt_show_onoff(FALSE);
#endif
#if (CC_ON ==1)
    #ifdef CC_BY_OSD
        api_atsccc_show_onoff(FALSE);
        #endif
#endif
        api_ttx_show_onoff(TRUE);
        g_second_osd_mode = OSD_TELETEXT;
#endif
    }
    else if(mode == OSD_INVALID_MODE)
    {
        g_second_osd_mode = OSD_INVALID_MODE;
    }
    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    osal_semaphore_release(bc_osd_sema_id);
    return mode;
    #endif
    return pre_mode;
}

UINT8 api_get_current_sub_mode(void)
{
    #ifndef _BUILD_OTA_E_
    return current_sub_mode;
    #else
    return 0;
    #endif
}
#endif



/*    num:    bit14~bit0  longitude/latitude
    bit15  0  east/north
    1  west/south
    return:  -180 ~ 180 / -90 ~ 90   longitude/latitude*/
double api_usals_local_word2double(UINT16 num)
{
    double result = 0;

    result = num&0x7fff;
    if(num&0x8000)
    {
        result *= -1;
    }
    result /= 100.0;           //current longitude
    return result;
}

BOOL api_check_usals_angle( UINT16 sat_pos)
{
    double __MAYBE_UNUSED__ sat_orbit = 0;
    double __MAYBE_UNUSED__ local_longitude = 0;
    double __MAYBE_UNUSED__ local_latitude = 0;
    S_NODE s_node;
    const double target_num = 90.0;
    SYSTEM_DATA *sys = NULL;

	MEMSET(&s_node, 0, sizeof(S_NODE));
    sys = sys_data_get();
    get_sat_at(sat_pos , VIEW_ALL, &s_node);
    sat_orbit = api_usals_get_sat_orbit(s_node.sat_orbit);
    local_longitude = api_usals_local_word2double(sys->local.local_longitude);
    local_latitude = api_usals_local_word2double(sys->local.local_latitude);

    /*usals_target_formula_calcuate*/

    if(api_usals_target_formula_calcuate(sat_orbit, local_longitude,local_latitude) > target_num)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/**************************************************************
    id:  logo  id
    addr: return address
    len: retrun data length
    return : false , true
****************************************************************/
//#ifdef LOGO_SUPPORT
UINT8 api_get_chunk_add_len(UINT32 id,UINT32 *addr,UINT32 *len)
{
    INT32 ret = 0;
    CHUNK_HEADER chuck_hdr;

    ret = sto_get_chunk_header(id,&chuck_hdr);
    if(0 == ret)
    {
        return 0;
    }

    *addr = (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE;
    *len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

    return 1;
}

void api_show_bootup_logo(void)
{
    int i = 0;
    UINT32 usizegot = 0;
    RET_CODE ret_code = 0;
    UINT32 logo_id = 0;
    UINT32 addr = 0;
    UINT32 len = 0;
    struct sto_device *sto_flash_dev = NULL;

#ifdef DUAL_ENABLE
    UINT8 *logo_buf = NULL;
#else
    void *pwritebuffer = NULL;
#endif
	if(0 == ret_code)
	{
		;
	}
#ifdef APP_PIC_LOGO_SUPPORT
    {
        osal_task_sleep(100);
        if(ad_pic_jpegbmp_logoshow(RADIO_LOGO_ID+0x200) == TRUE)
        {
            //osal_task_sleep(1000); // delay output time, can adjust this time
            libc_printf("%s, show jpg logo end\n", __FUNCTION__);
            return;
        }
        else
        {
            //flash no jpg logo, not return, show m2v logo
            libc_printf("%s flash no jpg logo!show default m2v logo!\n",__FUNCTION__);
        }
    }
#endif

    logo_id = BOOT_LOGO_ID;
    if(0 == api_get_chunk_add_len(logo_id,&addr,&len))
    {
        return;
    }
#ifdef DUAL_ENABLE
     logo_buf = (UINT8 *)MALLOC(len);
    if (NULL == logo_buf)
    {
        return;
    }
#endif

    ret_code = vdec_start((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0));
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    for(i=0;i<2;i++)
    {
    #ifdef DUAL_ENABLE
        sto_get_data(sto_flash_dev, (UINT8 *)logo_buf, addr, len);
    osal_cache_flush(logo_buf,len);
        vdec_copy_data((UINT32)g_decv_dev,(UINT32)logo_buf,len,&usizegot);
    #else
        ret_code = vdec_vbv_request(g_decv_dev, len, &pwritebuffer,&usizegot,NULL);
        sto_get_data(sto_flash_dev, (UINT8 *)pwritebuffer, addr, usizegot);
        vdec_vbv_update(g_decv_dev, usizegot);
    #endif
    }
#ifdef DUAL_ENABLE
    FREE(logo_buf);
#endif
}

#ifndef NEW_DEMO_FRAME
void api_video_set_pause(UINT8 flag)
{
    if(flag)
    {
        uich_chg_pause_prog();
    }
    else
    {
        uich_chg_resume_prog();
    }
}

void api_show_radio_logo(void)
{
    UINT32 addr, len;
    api_set_vpo_dit(TRUE);
    if(api_get_chunk_add_len(RADIO_LOGO_ID, &addr, &len))
    {
        uich_chg_show_logo((UINT8*)addr, len, TRUE);
    }
}

void api_show_menu_logo(void)
{
    UINT32 addr, len;
#if (SUBTITLE_ON == 1)
    api_subt_show_onoff(FALSE);
#endif
#if (CC_ON ==1)
    api_atsccc_show_onoff(FALSE);
#endif
#if (ISDBT_CC == 1)
    api_isdbtcc_show_onoff(FALSE);
#endif
    api_set_vpo_dit(TRUE);
    if(api_get_chunk_add_len(MENU_LOGO_ID, &addr, &len))
    {
        uich_chg_show_logo((UINT8*)addr, len, TRUE);
    }
}
#else
void api_video_set_pause(UINT8 flag)
{
    #if 0
    struct vdec_device *vdec = is_cur_decoder_avc() ? ((struct vdec_device *)dev_get_by_name("DECV_AVC_0")) :\
                                            ((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0));
    #else
    struct vdec_device *vdec = (struct vdec_device *)get_selected_decoder();
    #endif
    
    if ( flag )
    {
        chchg_pause_video(vdec);
    }
    else
    {
        chchg_resume_video(vdec);
    }
}

void api_show_radio_logo(void)
{
#ifndef _BUILD_OTA_E_
    UINT32 addr = 0;
    UINT32 len = 0;
    struct cc_logo_info logo;
    struct cc_device_list dev_list;
   // BOOL is_avc = FALSE;
#ifdef _NV_PROJECT_SUPPORT_
    TBlockParam     xBlkParam;
#endif

#ifdef APP_PIC_LOGO_SUPPORT
    {
        api_stop_play(0);
        osal_task_sleep(100);
        if(ad_pic_jpegbmp_logoshow(RADIO_LOGO_ID+0x200) == TRUE)
        {
            libc_printf("%s, show jpg logo end\n", __FUNCTION__);
            return;
        }
        else
        {
            //flash no jpg logo, not return, show m2v logo
            libc_printf("%s flash no jpg logo!show default m2v logo!\n",__FUNCTION__);
        }
    }
#endif

#ifdef _NV_PROJECT_SUPPORT_
    BlkParamGet(FLASH_BLK_LOGO, &xBlkParam);
    addr = xBlkParam.xRamOffset;
    //len = xBlkParam.xSize;
    len = 0xebf9;
#else
    if(api_get_chunk_add_len(RADIO_LOGO_ID, &addr, &len))
#endif
    {
        MEMSET(&logo, 0, sizeof(logo));
        logo.addr = (UINT8 *)addr;
        logo.size = len;
        logo.sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
        MEMSET(&dev_list, 0, sizeof(struct cc_device_list));
        dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        dev_list.vdec_hdtv = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        //is_avc = is_cur_decoder_avc();
        //if (TRUE == is_avc)
        //{
        //    dev_list.vdec_stop = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        //}
        //else
        //{
            dev_list.vdec_stop = (struct vdec_device *)get_selected_decoder();//dev_list.vdec;
        //}
        dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ap_get_main_dmx());
        chchg_show_logo(&logo, &dev_list);
    }
#endif
}

void api_show_menu_logo(void)
{
#ifndef _BUILD_OTA_E_
    UINT32 addr = 0;
    UINT32 len = 0;
    struct cc_logo_info logo;
    struct cc_device_list dev_list;
   // BOOL is_avc = FALSE;
#ifdef _NV_PROJECT_SUPPORT_
    TBlockParam     xBlkParam;
#endif

#ifdef _NV_PROJECT_SUPPORT_
    BlkParamGet(FLASH_BLK_LOGO, &xBlkParam);
    addr = xBlkParam.xRamOffset;
    //len = xBlkParam.xSize;
    len = 0xebf9;
#else
    if(api_get_chunk_add_len(MENU_LOGO_ID, &addr, &len))
#endif
    {
        MEMSET(&logo, 0, sizeof(logo));
        logo.addr = (UINT8 *)addr;
        logo.size = len;
        logo.sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
        MEMSET(&dev_list, 0, sizeof(struct cc_device_list));
        dev_list.vpo = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        dev_list.vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        dev_list.vdec_hdtv = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        //is_avc = is_cur_decoder_avc();
        //if (TRUE == is_avc)
        //{
        //    dev_list.vdec_stop = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        //}
        //else
        //{
            dev_list.vdec_stop = (struct vdec_device *)get_selected_decoder();//dev_list.vdec;
        //}
        dev_list.deca = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
        dev_list.snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
        dev_list.dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ap_get_main_dmx());
        chchg_show_logo(&logo, &dev_list);
    }
#endif
}

#endif
static BOOL is_vdec_first_shown_other = FALSE;
/* First Video Frame Already Show */
static void cb_vdec_first_show_other(UINT32 uparam1, UINT32 uparam2)
{
    is_vdec_first_shown_other = TRUE;
}

#if 0
/* Check whether First Vedio Frame Already Show or not */
static BOOL ck_vdec_first_show_other(void)
{
    return is_vdec_first_shown_other;
}
#endif
/* Reset First Vedio Frame not Show */
static void reset_vdec_first_show_other(void)
{
    is_vdec_first_shown_other = FALSE;
}
void api_show_row_logo(UINT32 logo_id)
{
    int i = 0;  
    UINT32 usizegot = 0;
    RET_CODE ret_code = RET_FAILURE;
    UINT32 addr= 0;
    UINT32 len = 0;
    struct vdec_status_info curstatus;
#ifdef DUAL_ENABLE
    UINT8 *logo_buf = NULL;
#else
    void *pwritebuffer = NULL;
#endif
    UINT32 count = 0;
    struct vdec_io_reg_callback_para tpara;
    struct sto_device *sto_flash_dev = NULL;
    CHUNK_HEADER chuck_hdr;
    const UINT32 first_pic_loop = 400;

	if(RET_FAILURE == ret_code)
	{
		;
	}
#ifdef _NV_PROJECT_SUPPORT_
    api_show_radio_logo();
    return;
#else
    if(0 == sto_get_chunk_header(logo_id, &chuck_hdr))
    {
        return;
    }
    MEMSET(&curstatus, 0, sizeof(struct vdec_status_info));
    MEMSET(&tpara, 0x0, sizeof(struct vdec_io_reg_callback_para));
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    addr = (UINT32)sto_chunk_goto(&logo_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
#endif
    
#ifdef DUAL_ENABLE
    logo_buf = (UINT8 *)MALLOC(len);
    if(NULL == logo_buf)
    {
        PRINTF("%s() not enough memory!\n",__FUNCTION__);
        ASSERT(0);
        return;
    }
#endif

    ret_code = vdec_stop(g_decv_dev,FALSE,FALSE);    
    reset_vdec_first_show_other();
    tpara.e_cbtype = VDEC_CB_FIRST_SHOWED;
    tpara.p_cb = cb_vdec_first_show_other;
    vdec_io_control(g_decv_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
#ifdef DUAL_ENABLE
    vdec_io_control(g_decv_dev, VDEC_SET_DMA_CHANNEL, (UINT32)DMA_INVALID_CHA);
#endif
    ret_code = vdec_start(g_decv_dev);
    for(i=0; i<2; i++)
    {
    #ifdef DUAL_ENABLE
        sto_get_data(sto_flash_dev, (UINT8 *)logo_buf, addr, len);
        osal_cache_flush(logo_buf,len);
        vdec_copy_data((UINT32)g_decv_dev,(UINT32)logo_buf,len,&usizegot);
    #else
        ret_code = vdec_vbv_request(g_decv_dev, len, &pwritebuffer,&usizegot,NULL);
        sto_get_data(sto_flash_dev, (UINT8 *)pwritebuffer, addr, usizegot);
        vdec_vbv_update(g_decv_dev, usizegot);
    #endif
    }

    //adopt the new method to ensure de_o getting frame under dual_output mode
    osal_task_sleep(10);
    while(1)
    {
        vdec_io_control(g_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&curstatus);
        if(!curstatus.u_first_pic_showed)
        {
            osal_task_sleep(10);
            ++count;
            if(count > first_pic_loop)
            {
                libc_printf("show log failed\n");
                //SDBBP();
                break;
            }
        }
        else
        {
            break;
        }
    }

    ret_code = vdec_stop(g_decv_dev,FALSE,FALSE);
#ifdef DUAL_ENABLE
    FREE(logo_buf);
#endif
    return;
}


static int _api_parse_def_set_subblock_by_flash(void)
{
    unsigned chid = 0;
    int i = 0;
    int sub_cnt = 0;
    unsigned char sb_type = 0;
    unsigned long addr = 0;
    unsigned long end_addr = 0;
    SUBBLOCK_HEADER hd;
    struct sto_device *sto = NULL;
    unsigned char item_type = 0;
    unsigned char byte = 0;
    unsigned long item_datalen = 0;
    unsigned long n = 0;
    SYSTEM_DATA *sys_data = NULL;
    unsigned char favgrp_cnt = 0;
    unsigned char favgrp_namelen = 0;
    unsigned char favgrp_code = 0;
    const UINT8 n_loop = 4;

    MEMSET(&hd, 0x0, sizeof (hd));

    sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if(NULL == sto)
    {
        return RET_FAILURE;
    }
    sys_data = sys_data_get();

    chid = 0x03FC0100;
    sb_type = 0x02;

    sub_cnt = sto_subblock_count(chid);
    if(0 == sub_cnt)
    {
        return RET_FAILURE;
    }
    addr = sto_subblock_goto(chid, &sb_type,0xFF,1);
    if(addr & 0x80000000)
    {
        return RET_FAILURE;
    }
    i = sto_get_subblock_header(0x03FC0100,sb_type,&hd);
    if(0 == i)
    {
        return RET_FAILURE;
    }
    addr += SUBBLOCK_HEADER_SIZE;

    end_addr = addr + hd.len;
    while(addr<end_addr)
    {
        sto_get_data(sto, &item_type, addr, 1);
        addr++;
        if((0x00 == item_type) || (0xFF == item_type))
        {
            break;
        }
        n = 0;
        item_datalen = 0;
        while(n < n_loop)
        {
            n++;
            item_datalen <<= 7;
            sto_get_data(sto, &byte, addr, 1);
            addr++;
            item_datalen += byte & 0x7F;
            if (0x00 == (byte & 0x80))    /* length field byte end. */
            {
                break;
            }
        }

        if(byte & 0x80)        /* item data length field error (>4Bs) */
        {
            break;
        }

        switch(item_type)
        {
        case 0x01:    /* Fav group name */
#ifdef FAV_GROP_RENAME
            {
                favgrp_cnt = 0;
                favgrp_namelen = 0;
                favgrp_code = 0;
       
                sto_get_data(sto, &favgrp_cnt,        addr + 0, 1);
                sto_get_data(sto, &favgrp_namelen,    addr + 1, 1);
                sto_get_data(sto, &favgrp_code,    addr + 2, 1);
                if(favgrp_cnt > MAX_FAVGROUP_NUM)
                {
                  favgrp_cnt = MAX_FAVGROUP_NUM;
                }
                if((0xFF == favgrp_cnt) || (0xFF == favgrp_namelen) || (0xFF == favgrp_code))
                {
                  break;
                }
                if(favgrp_code != 0x00 && favgrp_code != 0x01)
                {
                    break;
                }
                byte     = favgrp_namelen;
                if(byte>FAV_GRP_NAME_LEN)
                {
                    byte = FAV_GRP_NAME_LEN;
                }

                for(i=0;i<favgrp_cnt;i++)
                {
                    sto_get_data(sto, (UINT8 *)sys_data->favgrp_names[i],    addr + 3 + favgrp_namelen*i, byte);

                    if(0 == favgrp_code)
                    {
                        if(favgrp_namelen > FAV_GRP_NAME_LEN)
                        {
                            sys_data->favgrp_names[i][FAV_GRP_NAME_LEN] = '\0';
                        }
                        else
                        {
                            sys_data->favgrp_names[i][favgrp_namelen] = '\0';
                        }
                    }
                }
            }
#endif
            break;
        default:
            break;
        }

        addr += item_datalen;
    }

    return RET_SUCCESS;
}


/*
 *Desc: find specific subblock address from default db at ram.

 *Return: 1:means find it.
*/
static int _find_defdb_subblock_by_ram(UINT32 defdb_addr, unsigned char sbblock_type, UINT32 *addr)
{
    UINT32 address = defdb_addr;
    UINT32 end_addr = address + 64*1024;
    unsigned long len = 0;
    unsigned long offset = 0;
    unsigned char sbtype = 0;

    while(address < end_addr)
    {
        sbtype = fetch_long_ext((UINT8*)(address + SUBBLOCK_TYPE), 1);
        if (sbtype == 0 || sbtype == 0xFF)
            break;

        if (sbtype == sbblock_type)
        {
            *addr = address;
            return 1;
        }

        len = fetch_long_ext((UINT8*)(address + SUBBLOCK_LENGTH), 3);
        offset = fetch_long_ext((UINT8*)(address + SUBBLOCK_OFFSET), 3);

        if (((offset != 0) && (len > offset))||(0 == offset))
            break;
        address += offset;
    }

    return 0;
}

static int _api_parse_def_set_subblock_by_ram(UINT32 defdb_addr)
{
    int i = 0;
    unsigned long n = 0;
    unsigned long addr = 0;
    unsigned long end_addr = 0;
    SUBBLOCK_HEADER sub_header;
    unsigned long item_datalen = 0;
    unsigned char sb_type = 0;
    unsigned char item_type = 0;
    unsigned char byte = 0;
    unsigned char favgrp_cnt = 0;
    unsigned char favgrp_namelen = 0;
    unsigned char favgrp_code = 0;
    SYSTEM_DATA* sys_data = NULL;

    sys_data = sys_data_get();
    if(sys_data == NULL)
    {
        return 0;
    }

    //1. get subblock start address
    sb_type = 0x02;
    if(_find_defdb_subblock_by_ram(defdb_addr, sb_type, &addr) == 0)
    {
        libc_printf("%s(): _find_defdb_subblock_by_ram failed!\n", __FUNCTION__);
        return 0;
    }

    sub_header.type = sb_type;
    sub_header.len = fetch_long_ext((UINT8*)(addr + SUBBLOCK_LENGTH), 3);
    sub_header.offset = fetch_long_ext((UINT8*)(addr + SUBBLOCK_OFFSET), 3);
    addr += SUBBLOCK_HEADER_SIZE;
    end_addr = addr + sub_header.len;

    while(addr < end_addr)
    {
        //get item type
        item_type = *((unsigned char *)(addr));
        addr += 1;
        if(item_type == 0x00 || item_type == 0xFF)
        {
            libc_printf("%s(): item_type(%x) is wrong!\n", __FUNCTION__, item_type);
            break;
        }

        //get item data len
        n = 0;
        item_datalen = 0;
        while(n<4)
        {
            n++;
            item_datalen <<= 7;
            byte = *((unsigned char *)(addr));
            addr += 1;
            item_datalen += byte & 0x7F;
            if ((byte & 0x80) == 0x00)    /* length field byte end. */
                break;
        }

        if(byte & 0x80)
        {
            libc_printf("get item data length field failed!\n");
            break;    /* item data length field error (>4Bs) */
        }

        //init favourite group.
        switch(item_type)
        {
        case 0x01:  /* Fav group name */
#ifdef FAV_GROP_RENAME
            {
                favgrp_cnt = *((unsigned char *)(addr+0));
                favgrp_namelen = *((unsigned char *)(addr+1));
                favgrp_code = *((unsigned char *)(addr+2));

                if(favgrp_cnt > MAX_FAVGROUP_NUM)
                    favgrp_cnt = MAX_FAVGROUP_NUM;
                if(favgrp_cnt == 0xFF || favgrp_namelen == 0xFF || favgrp_code == 0xFF)
                    break;
                if(favgrp_code != 0x00 && favgrp_code != 0x01)
                    break;

                byte = favgrp_namelen;
                if(byte>FAV_GRP_NAME_LEN)
                    byte = FAV_GRP_NAME_LEN;

                for(i = 0; i < favgrp_cnt; i++)
                {
                    MEMCPY(sys_data->favgrp_names[i], (void*)(addr + 3 + favgrp_namelen*i), byte);

                    if(favgrp_code == 0)
                    {
                        if(favgrp_namelen > FAV_GRP_NAME_LEN)
                            sys_data->favgrp_names[i][FAV_GRP_NAME_LEN] = '\0';
                        else
                            sys_data->favgrp_names[i][favgrp_namelen] = '\0';
                    }
                }
            }
#endif
            break;
        default:
            break;
        }

        addr += item_datalen;
    }

    return 0;
}

int api_parse_defaultset_subblock(void)
{
#if (!defined(_BUILD_UPG_LOADER_)) && ((!defined(_BUILD_OTA_E_)))
    UINT32 defdb_ramaddr = 0;

    defdb_ramaddr = get_ramaddr_of_defdb();
    if (0 == defdb_ramaddr)
    {
        return _api_parse_def_set_subblock_by_flash();
    }
    else
    {
        return _api_parse_def_set_subblock_by_ram(defdb_ramaddr-16);
    }
#endif
    return 0;
}




BOOL    api_tv_radio_switch(void)
{
    UINT16    channel = 0;
    UINT8    avmode = 0;

    avmode = sys_data_get_cur_chan_mode();

    //1Switch TV to Audio
    if(PROG_TV_MODE == avmode)
    {
        sys_data_set_cur_chan_mode(PROG_RADIO_MODE);
    }
    else
    {
        sys_data_set_cur_chan_mode(PROG_TV_MODE);
    }

    channel = get_prog_num(VIEW_ALL | avmode, 0);

    //no program
    if(0 == channel)
    {
        //popup no program message
        sys_data_set_cur_chan_mode(avmode);
        return FALSE;
    }
    else
    {
        if(PROG_TV_MODE == avmode)
        {
            api_show_radio_logo();
        }
        sys_data_change_group(sys_data_get_cur_group_index());
        return TRUE;
    }
}


// reverse cch characters
void api_str_reverse(UINT16*psz, INT32 cch)
{
    INT32 ich = 0;
    UINT16 chtemp = 0;

    while (1)
    {
        cch--;
        if (ich < cch)
        {
            chtemp = psz[ich];
            psz[ich] = psz[cch];
            psz[cch] = chtemp;
            ich++;
        }
        else
        {
            break;
        }
    }
}

#ifdef _INVW_JUICE //v0.1.4
UINT8 api_get_chuck_addlen(UINT32 id,UINT32* addr,UINT32* len)
{
    INT32 ret = 0;
    CHUNK_HEADER chuck_hdr;

    MEMSET(&chuck_hdr, 0x0, sizeof(CHUNK_HEADER));
    ret = sto_get_chunk_header(id,&chuck_hdr);
    if(0 == ret)
    {
        return 0;
    }

    *addr = (UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE;
    *len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

    return 1;
}
#endif

UINT16 get_frontend_satid(UINT8 frontend_type, UINT8 is_isdbt)
{
    if(frontend_type == dev_get_device_sub_type(dev_get_by_id(HLD_DEV_TYPE_NIM, 0), HLD_DEV_TYPE_NIM) &&
        is_isdbt == board_frontend_is_isdbt(0))
    {
        return board_get_frontend_satid(0);
    }
    else if(frontend_type == dev_get_device_sub_type(dev_get_by_id(HLD_DEV_TYPE_NIM, 1), HLD_DEV_TYPE_NIM) &&
        is_isdbt == board_frontend_is_isdbt(1))
    {
        return board_get_frontend_satid(1);
    }
    else
    {
        return INVALID_SAT_ID;
    }
}

//add on 2011-11-02 for combo ui
UINT8 get_combo_config_frontend(void)
{
    INT32 i = 0;
    UINT8 count = 0;
    UINT8 s_front = 0;
    UINT8 t_front = 0;
    UINT8 isdbt_front = 0;
    UINT8 c_front = 0;
    const UINT8 dual_tuner_cnt = 2;
    const UINT8 single_tuner_cnt = 1;

    for(i = 0; i < 2; i++)
    {
        switch(board_get_frontend_type(i))
        {
            case FRONTEND_TYPE_S:
                s_front++;
                break;
            case FRONTEND_TYPE_ISDBT:
            case FRONTEND_TYPE_T:
                if(board_frontend_is_isdbt(i))
                {
                    isdbt_front++;
                }
                else
                {
                    t_front++;
                }
                break;
            case FRONTEND_TYPE_C:
                c_front++;
                break;
            default:
                break;
        }
    }

    if((s_front == dual_tuner_cnt) || (t_front == dual_tuner_cnt) || (isdbt_front == dual_tuner_cnt) \
        || (c_front == dual_tuner_cnt) || (s_front + t_front + isdbt_front + c_front == single_tuner_cnt))
    {
        count = 1;
    }
    else
    {
        if(s_front + t_front + isdbt_front + c_front == dual_tuner_cnt)
        {
            count = 2;
        }
    }

    return count;
}

UINT8 get_combo_tuner_num(void)
{
    INT32 i = 0;
    UINT8 count = 0;
    UINT8 s_front = 0;
    UINT8 t_front = 0;
    UINT8 isdbt_front = 0;
    UINT8 c_front = 0;
    const UINT8 dual_tuner = 2;
    const UINT8 single_tuner = 1;

    for(i = 0; i < 2; i++)
    {
        switch(board_get_frontend_type(i))
        {
            case FRONTEND_TYPE_S:
                s_front++;
                break;
            case FRONTEND_TYPE_ISDBT:
            case FRONTEND_TYPE_T:
                if(board_frontend_is_isdbt(i))
                {
                    isdbt_front++;
                }
                else
                {
                    t_front++;
                }
                break;
            case FRONTEND_TYPE_C:
                c_front++;
                break;
            default:
                break;
        }
    }

    if((s_front == dual_tuner) || (t_front == dual_tuner) || (isdbt_front == dual_tuner) || (c_front == dual_tuner))
    {
        count = 2;
    }
    else
    {
        if(s_front + t_front + isdbt_front + c_front == dual_tuner)
        {
            count = 2;
        }
        else if(s_front + t_front + isdbt_front + c_front == single_tuner)
        {
            count = 1;
        }
    }

    return count;
}

//add on 2011-11-02 for combo ui
BOOL check_frontend_type(UINT8 frontend_type, UINT8 is_isdbt)
{

    if(frontend_type == board_get_frontend_type(0) && is_isdbt == board_frontend_is_isdbt(0))
    {
        return TRUE;
    }
    else if(frontend_type == board_get_frontend_type(1) && is_isdbt == board_frontend_is_isdbt(1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//add on 2011-11-02 for combo ui
void set_combo_chan_idx(UINT8 frontend_kind,  T_NODE *t_node, P_NODE *p_node, UINT16 chan_idx, char *str)
{
    UINT8 size = 10;

#if defined( _LCN_ENABLE_) && !defined(_BUILD_OTA_E_)
    UINT8 dvb_combo=0;
    //const UINT8 combo_c_s = 0;
    const UINT8 combo_t = 1;
    const UINT8 combo_isdbt = 2;
    const UINT8 combo_t_s = 3;
    const UINT8 combo_isdbt_s = 4;
    const UINT8 r_ctl_key_id = 100;

    db_get_dvb_combo(&dvb_combo); // 0:dvbc, dvbs  1:dvbt  2:isdbt  3:dvbt+dvbs 4:isdbt+dvbs

    if((dvb_combo == combo_isdbt || dvb_combo == combo_isdbt_s) && sys_data_get_lcn())
    {
        if(t_node->remote_control_key_id != r_ctl_key_id)
        {
            snprintf(str,size, "%02d-%01d%1d    ", t_node->remote_control_key_id, \
                (p_node->prog_number & 0x1f)>>3, (p_node->prog_number & 0x07)+1);
        }
        else
        {
            snprintf(str,size,"%02d-%04d", 0x00, p_node->LCN);
        }
    }
    else if((dvb_combo == combo_t || dvb_combo==combo_t_s) && sys_data_get_lcn())
    {
         snprintf(str,size, "%04d",p_node->LCN);
    }
    else
#endif
        snprintf(str, size, "%04d",chan_idx + 1);
}
