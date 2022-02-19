/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_ttx.c
 *
 *    Description: This source file contains control application's teletext
      relate process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <mediatypes.h>
#include <hld/dis/vpo.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "menus_root.h"
#include "vkey.h"
#if (TTX_ON == 1)
#include <hld/vbi/vbi.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#endif
#if (TTX_ON == 1)
static void api_enable_vbi(BOOL enable)
{
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_ENABLE_VBI, enable);
}
#ifdef TTX_BY_OSD
static UINT8 ttx_keymap(UINT32 __MAYBE_UNUSED__ msgtype, UINT32 msgcode)
{
    TTX_KEYTYPE ttx_key = PROC_PASS;

    switch (msgcode)
    {
    case V_KEY_0:
        ttx_key = TTX_KEY_NUM0;
        break;

    case V_KEY_1:
        ttx_key = TTX_KEY_NUM1;
        break;

    case V_KEY_2:
        ttx_key = TTX_KEY_NUM2;
        break;

    case V_KEY_3:
        ttx_key = TTX_KEY_NUM3;
        break;

    case V_KEY_4:
        ttx_key = TTX_KEY_NUM4;
        break;

    case V_KEY_5:
        ttx_key = TTX_KEY_NUM5;
        break;

    case V_KEY_6:
        ttx_key = TTX_KEY_NUM6;
        break;

    case V_KEY_7:
        ttx_key = TTX_KEY_NUM7;
        break;

    case V_KEY_8:
        ttx_key = TTX_KEY_NUM8;
        break;

    case V_KEY_9:
        ttx_key = TTX_KEY_NUM9;
        break;

    case V_KEY_UP:
        ttx_key = TTX_KEY_UP;
        break;

    case V_KEY_DOWN:
        ttx_key = TTX_KEY_DOWN;
        break;

    case V_KEY_LEFT:
        ttx_key = TTX_KEY_LEFT;
        break;

    case V_KEY_RIGHT:
        ttx_key = TTX_KEY_RIGHT;
        break;

    case V_KEY_RED:
        ttx_key = TTX_KEY_RED;
        break;

    case V_KEY_GREEN:
        ttx_key = TTX_KEY_GREEN;
        break;

    case V_KEY_YELLOW:
        ttx_key = TTX_KEY_YELLOW;
        break;

    case V_KEY_BLUE:
        ttx_key = TTX_KEY_CYAN;
        break;

    case V_KEY_ENTER:
        ttx_key = TTX_KEY_ALPHA;
        break;

    case V_KEY_TEXT:
    case V_KEY_EXIT:
        ttx_key = PROC_LEAVE;
        break;

    default:
        ttx_key = PROC_PASS;
        break;
    }

    return ttx_key;
}
#endif
void ttx_osd_open(void)
{
    UINT8               ttx_key = PROC_LOOP;
    struct t_ttx_lang   *lang_list = NULL;
    UINT8               lang_num = 0;
    UINT8               sel_lang = 0;
    UINT32              key = 0;
    UINT32              vkey = 0;
    UINT32              pre = 0;
    UINT32              cur = 0;    
    SYSTEM_DATA         *sys_data = NULL;
    BOOL                old_value = FALSE;
#ifdef TTX_EPG_SHARE_MEM
    UINT16  cur_channel;
    P_NODE  p_node;
    struct ts_route_info    l_ts_route;
    UINT32                  dmx_id = 0;
    struct dmx_device       *dmx = NULL;
#endif
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL                bskip = FALSE;
	if(FALSE == bskip)
	{
		;
	}
    pvr_info  = api_get_pvr_info();
    sys_data = sys_data_get();


#ifdef TTX_EPG_SHARE_MEM
    epg_off();
    epg_release();
    ttx_enable(TRUE);
#endif

    ttxeng_get_init_lang(&lang_list, &lang_num);
    sel_lang = api_ttxsub_getlang(lang_list, NULL, sys_data->lang.ttx_lang, lang_num);
    if (0xFF == sel_lang)
    {
        return; //NO Teletext!!
    }

    api_enable_vbi(FALSE);  //not show VBI and ttx at the same time
    osddrv_show_on_off((HANDLE) g_osd_dev, OSDDRV_OFF);
    bskip = FALSE;
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    //BC_API_PRINTF("%s-%d- OSD_TELETEXT\n", __FILE__, __LINE__);
    handle_osm_complement_in_ignore_window(TRUE);
    if (OSD_TELETEXT != api_osd_mode_change(OSD_TELETEXT))
    {
        //BC_API_PRINTF("%s-Force skip TTX\n", __FUNCTION__);
        bskip = TRUE;
    }

#else
    api_osd_mode_change(OSD_TELETEXT);
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if (FALSE == bskip)
    {
#endif
#ifdef TTX_BY_OSD

        ttxeng_set_init_lang(sel_lang);
#endif
        old_value = ap_enable_key_task_get_key(TRUE);
        pre=osal_get_tick();  
        while (ttx_key != PROC_LEAVE)
        {
            key = 0;
            key = ap_get_key_msg();
            if (INVALID_HK == key)
            {
                PRINTF("invalid hk!\n");
            }

            ap_hk_to_vk(0, key, &vkey);
            if (V_KEY_NULL == vkey)
            {
                continue;
            }
		#ifdef TTX_BY_OSD
            ttx_key = ttx_keymap(CTRL_MSG_SUBTYPE_KEY, vkey);
            if (PROC_LEAVE == ttx_key)
            {
                break;
            }
		#endif
            cur=osal_get_tick();
            if((cur-pre)>450)
            {
                ttxeng_send_key(ttx_key);
                pre=cur;
            }
        }

        ap_enable_key_task_get_key(old_value);
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    }
#endif
    api_enable_vbi(TRUE);
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    //BC_API_PRINTF("%s-%d- OSD_NO_SHOW\n", __FILE__, __LINE__);
#endif
    api_osd_mode_change(OSD_NO_SHOW);   //to show subt
    osddrv_show_on_off((HANDLE) g_osd_dev, OSDDRV_ON);
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    handle_osm_complement_in_ignore_window(FALSE);
#endif

#ifdef TTX_EPG_SHARE_MEM
    ttx_enable(FALSE);
    epg_init(SIE_EIT_WHOLE_TP, (UINT8 *)__MM_EPG_BUFFER_START, __MM_EPG_BUFFER_LEN, ap_epg_call_back);
    cur_channel = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(cur_channel, &p_node);

    if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &l_ts_route) == RET_SUCCESS)
    {
        dmx_id = l_ts_route.dmx_id;
    }

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
#ifdef _INVW_JUICE
    epg_on_by_inview(dmx, p_node.sat_id, p_node.tp_id, p_node.prog_number, 0);
#else
    epg_on_ext(dmx, p_node.sat_id, p_node.tp_id, p_node.prog_number);
#endif
#endif

#if (DVR_PVR_SUPPORT_SUBTITLE && defined(DVR_PVR_SUPPORT))
    if ( pvr_info->pvr_state != PVR_STATE_IDEL)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win_pvr_ctrl, TRUE);
    }
    else
#endif
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) CHANNEL_BAR_HANDLE, TRUE);
    }
#ifdef _INVW_JUICE
#ifdef DISABLE_ALI_INFORBAR
    int inview_code;
    int ret_code;
    ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
    inview_resume(inview_code);
    inview_handle_ir(inview_code);
#endif
#endif
}
#endif
