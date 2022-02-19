 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_hdmi_factory.c
*
*    Description:   The realize for hdmi factory
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
#include <api/libfs2/fcntl.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <hld/hld_dev.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "vkey.h"
#include "win_hdmi_factory.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "ctrl_key_proc.h"


/*******************************************************************************
* Objects declaration
*******************************************************************************/
#define HDMI_PRINTF(...) //libc_printf


//#define   CW_L     //256//90
//#define   CW_T     //100//20
#define CW_W     800    //440
#define CW_H     460    //300
#define CW_L     GET_MID_L(CW_W)     //256//90
#define CW_T     GET_MID_T(CW_H)     //100//20

#define ITEM_H  40

#undef WIN_SH_IDX
#define WIN_SH_IDX  WSTL_DIAG_WIN_8BIT //WSTL_POPUP_WIN_02//WSTL_POP_WIN_01

#undef TXTN_SH_IDX
#define TXTN_SH_IDX WSTL_DIAG_BUTTON_01_8BIT


#define INFO_TXT_NAME_H 2
#define INFO_TXT_NAME_H1 (INFO_TXT_NAME_H + 50)
#define INFO_TXT_NAME_H2 (INFO_TXT_NAME_H1 + 50)
#define INFO_TXT_NAME_H3 (INFO_TXT_NAME_H2 + 50)
#define INFO_TXT_NAME_H4 (INFO_TXT_NAME_H3 + 50)

static VACTION  hdmi_factest_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT  hdmi_factest_callback(POBJECT_HEAD p_obj, \
                VEVENT event, UINT32 param1, UINT32 param2);

#ifndef _CAS9_CA_ENABLE_
static char *hdmi_strcat ( char * dst, const char * src);
static void hdmi_debug_message_callback(UINT8 message_type, UINT8* message);
static char    test[60];
static UINT8   key_id[5];
static UINT32 edid_log_file= 0;
static UINT32 debug_log_file= 0;
#endif

static UINT16 hdmi_info_name_str[60];
static UINT16 hdmi_info1_str[60];
static UINT16 hdmi_info2_str[60];
static UINT16 hdmi_info3_str[60];
static UINT16 hdmi_info4_str[60];

#ifndef _CAS9_CA_ENABLE_
static sys_state_t last_system_state = SYS_STATE_NORMAL;
#endif
#ifdef _CAS9_CA_ENABLE_
//const char version[] = "3602.0201";
//const char code[] = "ALI";
//const char keyid[] = "*****";
#define HDMIVERSION " "
#define HDCPMFCCODE " "
#define CRYPTOMEMORY " "
#define HDCPKEYID   " "
#else
const char menuname[] = "HDMI Factory Test menu";
//const char version[] = "3602.0201";
//const char code[] = "ALI";
//const char keyid[] = "*****";
#define HDMIVERSION " Software Version:  "
#define HDCPMFCCODE " HDCP Manufacturer Code: "
#define CRYPTOMEMORY " Crypto Memory: "
#define HDCPKEYID   " HDCP Key ID:"
#endif

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_wnd,NULL,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
        hdmi_factest_keymap,hdmi_factest_callback,  \
        nxt_obj, focus_id,0)

#define LDEF_TXT(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,\
    TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,\
    TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_VCENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LIST_TEXT(root,var_nametxt,var_txt1,\
    var_txt2,var_txt3,var_txt4,l,t,w,h) \
    LDEF_TXT_TITLE(&root,var_nametxt,&var_txt1 ,1,l ,\
     t + INFO_TXT_NAME_H,w,h,0,hdmi_info_name_str)   \
    LDEF_TXT(&root,var_txt1,&var_txt2 ,2,l ,\
     t+INFO_TXT_NAME_H1,w,h,0,hdmi_info1_str) \
    LDEF_TXT(&root,var_txt2,&var_txt3 ,3,l ,\
     t+INFO_TXT_NAME_H2,w,h,0,hdmi_info2_str) \
    LDEF_TXT(&root,var_txt3,&var_txt4 ,4,l, \
    t+INFO_TXT_NAME_H3,w,h,0,hdmi_info3_str)      \
    LDEF_TXT(&root,var_txt4, NULL ,5,l, t+INFO_TXT_NAME_H4,w,h,0,hdmi_info4_str)
/*
LDEF_LIST_ITEM(win_info_con,info_item_con,\
    NULL,info_txt_name,info_txt1,info_txt2,\
    CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H)
*/
LDEF_LIST_TEXT(g_win_hdmi_factory_test,hdmi_info_name,\
     hdmi_info1,hdmi_info2,hdmi_info3,hdmi_info4,\
    (CW_L+8),CW_T,CW_W-16,ITEM_H);

LDEF_WIN(g_win_hdmi_factory_test,&hdmi_info_name,CW_L,CW_T,CW_W,CW_H,1);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
#ifndef _CAS9_CA_ENABLE_
static void hdmi_display_info_set(void)
{

    int     position= 0;
    UINT8 i= 0;
    char *tmp_str = NULL;
    char *retval __MAYBE_UNUSED__ = NULL;

    osd_set_text_field_content(&hdmi_info_name,STRING_ANSI, (UINT32)menuname);

    MEMSET(test,0x00,sizeof(test));
    strncpy(test,HDMIVERSION, sizeof(test)-1);
    //hdmi_strcat(test,get_hdmi_sw_version());
    tmp_str = get_hdmi_sw_version();
    if (NULL != tmp_str)
    {
        retval = hdmi_strcat(test, tmp_str);
    }
    HDMI_PRINTF("%s\n",get_hdmi_sw_version());

    osd_set_text_field_content(&hdmi_info1,STRING_ANSI, (UINT32)test);

    MEMSET(test,0x00,sizeof(test));
    strncpy(test,HDCPMFCCODE, sizeof(test)-1);
    //hdmi_strcat(test, get_hdmi_cm_mfc_code());
    tmp_str = (char *)get_hdmi_cm_mfc_code();
    if (NULL != tmp_str)
    {
        retval = hdmi_strcat(test, tmp_str);
    }

    HDMI_PRINTF("%s\n",get_hdmi_cm_mfc_code());
    osd_set_text_field_content(&hdmi_info2,STRING_ANSI, (UINT32)test);

    MEMSET(test,0x00,sizeof(test));
    strncpy(test,CRYPTOMEMORY, sizeof(test)-1);
    //hdmi_strcat(test, get_hdmi_cm_status() );
    tmp_str = (char *)get_hdmi_cm_status();
    if (NULL != tmp_str)
    {
        retval = hdmi_strcat(test, tmp_str);
    }

    osd_set_text_field_content(&hdmi_info3,STRING_ANSI, (UINT32)test);

    MEMSET(test,0x00,sizeof(test));
    MEMSET(key_id,0x00,sizeof(key_id));
    position = 0;
    position += snprintf( test+position, 60, "%s",HDCPKEYID);
    //MEMCPY(Key_id,get_hdmi_cm_key_id(),sizeof(Key_id));
    tmp_str = (char *)get_hdmi_cm_key_id();
    if (NULL != tmp_str)
        MEMCPY(key_id,tmp_str,sizeof(key_id));
    for(i = 0; i < sizeof(key_id) - 1; i++)
    {
        position += snprintf(test+position,60-position,"0x%.2x,",key_id[i]);
        HDMI_PRINTF("0x%.2x,",key_id[i]);
    }
    position += snprintf(test+position,60-position,"0x%.2x",key_id[i]);
    HDMI_PRINTF("0x%.2x",key_id[i]);
    HDMI_PRINTF("\n");
    osd_set_text_field_content(&hdmi_info4,STRING_ANSI, (UINT32)test);
    osd_draw_object((POBJECT_HEAD )&g_win_hdmi_factory_test,C_UPDATE_ALL);

}
#endif

static VACTION  hdmi_factest_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act= PROC_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT  hdmi_factest_callback(POBJECT_HEAD p_obj, \
                VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

#ifndef _CAS9_CA_ENABLE_
    struct hdmi_device *hdmi_dev = (struct hdmi_device *)\
    dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    UINT8 b_id __MAYBE_UNUSED__;
    //POBJECT_HEAD nxt_obj;
    POBJECT_HEAD topmenu;
    b_id = osd_get_obj_id(p_obj);
    //UINT8 buf[512];
    UINT32 v_key;//, i;
    //FILE *fp;
    //MULTISEL *p_msel;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    static BOOL log_edid_info = FALSE;
    static BOOL log_dbg_info = FALSE;
    //static BOOL ignore_hotplug = FALSE;

    system_state = api_get_system_state();

    switch(event)
    {
    case EVN_PRE_OPEN:

        //ap_enable_key_task_get_key(TRUE);
        //key_set_upgrade_check_flag(0);
        last_system_state = system_state;
        //system_state = SYS_STATE_UPGRAGE_HOST;
        MEMSET(hdmi_info_name_str,0x00,sizeof(hdmi_info_name_str));
        MEMSET(hdmi_info1_str,0x00,sizeof(hdmi_info1_str));
        MEMSET(hdmi_info2_str,0x00,sizeof(hdmi_info2_str));
        MEMSET(hdmi_info3_str,0x00,sizeof(hdmi_info3_str));
        MEMSET(hdmi_info4_str,0x00,sizeof(hdmi_info4_str));
        wincom_close_title();
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_clear_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
            topmenu = menu_stack_get_top();
            if(topmenu)
                osd_clear_object( topmenu, C_UPDATE_ALL);
        }
        break;
    case EVN_POST_OPEN:
        hdmi_dev->io_control(hdmi_dev,HDMI_CMD_REG_CALLBACK,\
     HDMI_CB_DBG_MSG, (UINT32)hdmi_debug_message_callback);
        hdmi_display_info_set();
        break;
    case EVN_POST_CLOSE:
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
            osd_track_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
        api_set_system_state(last_system_state);
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_hk_to_vk(0, param1, &v_key);
        switch(v_key)
        {
            case V_KEY_RED:     // Red Button : Log EDID Information //old IR Red key is 65
                if(log_edid_info == FALSE)
                {
                    log_edid_info = TRUE;

                    // Check log file exist or not, if exist delete it.
                    edid_log_file= fs_open("/c/hdmi_edid_log.txt",\
             O_RDONLY, 0666);
//                      if(edid_log_file >= 0)   /*always is true ,clean the warning */
                    {
                        fs_close(edid_log_file);
                        fs_remove("/c/hdmi_edid_log.txt");
                    }

                        edid_log_file = fs_open("/c/hdmi_edid_log.txt", O_RDWR | O_CREAT,
                        S_IFREG | S_IRWXU);
                    hdmi_log_edid_start();                     
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,
                       "Start to Log EDID Information", 0);
                }
                else
                {
                    log_edid_info = FALSE;
                    fs_close( edid_log_file );
                    fs_sync("/c/");
                    hdmi_log_edid_stop();
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,"Stop to Log EDID Information", 0);             
                }
                osal_task_sleep(2000);
                win_compopup_close();
                hdmi_display_info_set();
                break;
            case V_KEY_GREEN:       // Green Button : Log debug Information
                if(log_dbg_info == FALSE)
                {
                    log_dbg_info = TRUE;

                    // Check log file exist or not, if exist delete it.
                    debug_log_file= fs_open("/c/hdmi_debug_log.txt", O_RDONLY, 0666);
//                      if(debug_log_file >= 0)     /*always is true ,clean the warning */
                    {
                        fs_close(debug_log_file);
                        fs_remove("/c/hdmi_debug_log.txt");
                    }

                        debug_log_file = fs_open("/c/hdmi_debug_log.txt",
                         O_RDWR | O_CREAT, S_IFREG | S_IRWXU);
                    hdmi_log_debug_start();   
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,
                       "Start to Log Debug Information", 0);
                }
                else
                {
                    log_dbg_info = FALSE;
                    fs_close( debug_log_file );
                    fs_sync("/c/");
                    hdmi_log_debug_stop();
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,"Stop to Log Debug Information", 0);
                }
                osal_task_sleep(2000);
                win_compopup_close();
                hdmi_display_info_set();
                break;
            case V_KEY_YELLOW:      // Yellow Button : Switch on/off HDCP
                if(api_get_hdmi_hdcp_onoff() == TRUE)
                {
                    api_set_hdmi_hdcp_onoff(FALSE);
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,"Turn HDCP Off", 0);
                }
                else
                {
                    api_set_hdmi_hdcp_onoff(TRUE);
                    win_com_popup_open(WIN_POPUP_TYPE_SMSG,"Turn HDCP On", 0);
                }
                osal_task_sleep(2000);
                win_compopup_close();
                hdmi_display_info_set();
                break;

            default:
                break;
        }

        break;
    }
#endif
    return ret;
}

#ifndef _CAS9_CA_ENABLE_
static void hdmi_debug_message_callback(UINT8 message_type, UINT8 *message)
{
    switch(message_type)
    {
        case 1:
            fs_write(edid_log_file, message, strlen((char*)message));
            libc_printf((char*)message);
            break;
        case 2:
            fs_write(debug_log_file, message, strlen((char*)message));
            libc_printf((char*)message);
            break;
        default:
            break;
    }

    return;
}
static char *hdmi_strcat ( char *dst, const char *src)
{
        char *cp = dst;

        while( *cp )
    {
                cp++;                   /* find end of dst */
    }
        while(( *cp++ = *src++ ))
    {
        ;       /* Copy src to end of dst */
    }
        return( dst );                  /* return dst */
}
#endif

