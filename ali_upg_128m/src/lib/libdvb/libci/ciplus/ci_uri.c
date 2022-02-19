/*****************************************************************************
 *    Copyrights(C) 2010 ALI Corp. All Rights Reserved.
 *
 *    FILE NAME:        ci_uri.c
 *
 *    DESCRIPTION:     CI+ URI Mapping Table
 *
 *    HISTORY:
 *        Date         Author         Version     Notes
 *        =========    =========    =========    =========
 *        2010/5/14   Elliott      0.1
 *
 *****************************************************************************/
#include <sys_config.h>
#include <api/libci/ci_plus.h>
#include <api/libci/ci_cc_api.h>
#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>
#include <bus/hdmi/m36/hdmi_api.h>
//#include <api/libtsi/db_3l.h>
#include <hld/decv/decv.h>
#include "ci_stack.h"

#ifdef CI_PLUS_SUPPORT

OSAL_ID ci_uri_mutex = OSAL_INVALID_ID;

#define enter_uri_mutex    \
    do                    \
    {                    \
        if(ci_uri_mutex == OSAL_INVALID_ID)                        \
        {                                                        \
            ci_uri_mutex = osal_mutex_create();                    \
            ASSERT(ci_uri_mutex != OSAL_INVALID_ID);            \
        }                                                         \
        osal_mutex_lock(ci_uri_mutex, OSAL_WAIT_FOREVER_TIME);    \
    }while(0)
#define leave_uri_mutex    do{osal_mutex_unlock(ci_uri_mutex);}while(0)

static struct UIR_DATA uri_data = {0};
struct UIR_DATA uri_data_bak = {0};

static struct UIR_DATA live_uri_data = {0};

static UINT8 last_mg = 0;    //keep last mg/cgms config
static UINT8 last_aps = 0;    //keep last mg/cgms config
static UINT8 last_cgms = 0;    //keep last mg/cgms config
UINT8 dis_analog_out = 0;
UINT8 dis_yuv_output_for_ict = 0;
#ifdef CI_PLUS_NO_SINGLE_SLOT
static int ci_slot = 1;//support 2 slot and slot 0 not ok
#else
static int ci_slot = 0;//single slot
#endif

void uri_enable_analog_output();

enum URI_STATE uri_get_msg(struct ci_uri_message *uri)
{
    enter_uri_mutex;
    if (uri_data.state == URI_RECEIVED)
    {
        APPL_PRINTF("%s: success\n", __FUNCTION__);
        MEMCPY(uri, &uri_data.uri_msg, sizeof(struct ci_uri_message));
    }
    else
    {
        APPL_PRINTF("%s: fail\n", __FUNCTION__);
    }
    leave_uri_mutex;

    return uri_data.state;
}

//uri_msg: IN, URI_CLEAR - clear URI
//               URI_NOT_RECEIVED - URI not received
void notify_uri(int slot, struct ci_uri_message *uri_msg)
{
    enter_uri_mutex;
    if ((UINT32)(uri_msg) == URI_CLEAR)
    {
        uri_data.state = URI_CLEAR;
        live_uri_data.state = uri_data.state;
        uri_data.slot = -1;
        live_uri_data.slot = uri_data.slot;
        APPL_PRINTF("%s - no uri\n",__FUNCTION__);
    }
    else if ((UINT32)(uri_msg) == URI_NOT_RECEIVED)
    {
        uri_data.state = URI_NOT_RECEIVED;
        live_uri_data.state = uri_data.state;
        uri_data.slot = -1;
        live_uri_data.slot = uri_data.slot;
        APPL_PRINTF("%s - uri not received\n", __FUNCTION__);
    }
    else
    {
        APPL_PRINTF("%s - uri received\n", __FUNCTION__);
        uri_data.state = URI_RECEIVED;
        MEMCPY(&uri_data.uri_msg, uri_msg, sizeof(struct ci_uri_message));
        uri_data.slot = slot;
        if(uri_msg->uri_from_pvr == 0)
        {
            live_uri_data.state = uri_data.state;
            MEMCPY(&live_uri_data.uri_msg, &uri_data.uri_msg, sizeof(struct ci_uri_message));
            live_uri_data.slot = uri_data.slot;
            APPL_PRINTF("%s - live uri also updated\n", __FUNCTION__);
        }
    }

    leave_uri_mutex;
}

void (*ci_uri_dac_reg)() = NULL;
void (*ci_uri_switch_tv_mode)(enum tvsystem tvsys, BOOL b_progressive) = NULL;

void api_ci_register_tv_mode(char *cic_name, void (*callback)(enum tvsystem tvsys, BOOL b_progressive))
{
    ci_uri_switch_tv_mode = callback;
}

void api_ci_register_dac_reg(char *cic_name, void (*callback)())
{
    ci_uri_dac_reg = callback;
}

static struct vpo_io_cgms_info l_cgms_info;
static UINT8 l_mg_info;

void api_ci_get_cgms_info(UINT8 *cgms, UINT8 *aps)
{
    if (cgms) *cgms = l_cgms_info.cgms;
    if (aps) *aps = l_cgms_info.aps;
}

UINT8 api_ci_get_mg_info()
{
    return l_mg_info;
}

void apply_uri(int slot, struct ci_uri_message *uri_msg)
{
    UINT8 mg_480p;
    struct vpo_io_cgms_info cgms;
    UINT8 image_constraint;

    UINT8 hdcp;
    UINT8 dtcp_epn;
    UINT8 dtcp_e_cci;
    UINT8 dtcp_ict;
    UINT8 dtcp_aps;
    struct snd_spdif_scms spdif;

    struct vpo_io_get_info dis_info;
    enum tvsystem sd_tvsys;
    static UINT8 ict_state = 0;

    ci_slot = slot;

    //lsbf vs. msbf
    //spdif.category_code = CATEGORY_CODE_EUROPE;//default category code, ref "IEC 60958-3-2006: Table7"
    spdif.category_code = 0x0c;

    APPL_PRINTF("%s - Apply with URI data!\n", __FUNCTION__);

    //URI mapping table, ref "CI_PLUS_DEVICE_INTERIM_LICENSE_AGREEMENT_2008_12_16r1: Exhibit E"
    if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x00)
    {
        mg_480p = 0;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
        cgms.aps = 0x00;
        cgms.cgms = uri_msg->emi_copy_control_info;
        image_constraint = uri_msg->ict_copy_control_info;//0=>no; 1=>520k

        hdcp = 1;//yes
        dtcp_epn = (uri_msg->rct_copy_control_info==0 ? 1 : 0);
        dtcp_e_cci = uri_msg->emi_copy_control_info;
        dtcp_ict = (uri_msg->ict_copy_control_info==0 ? 1 : 0);
        dtcp_aps = uri_msg->aps_copy_control_info;
        spdif.copyright = 1;
        spdif.l_bit = 0;
    }
    else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x01)
    {
        mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
        cgms.aps = uri_msg->aps_copy_control_info;
        cgms.cgms = uri_msg->emi_copy_control_info;
        image_constraint = uri_msg->ict_copy_control_info;//0=>no; 1=>520k

        hdcp = 1;//yes
        dtcp_epn = (uri_msg->rct_copy_control_info==0 ? 1 : 0);
        dtcp_e_cci = uri_msg->emi_copy_control_info;
        dtcp_ict = (uri_msg->ict_copy_control_info==0 ? 1 : 0);
        dtcp_aps = uri_msg->aps_copy_control_info;
        spdif.copyright = 0;
        spdif.l_bit = 1;
    }
    else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x02)
    {
        if(uri_msg->uri_from_pvr == 0)//If the Host has not made one copy, then go to use case: Copy Once, now default
        {
            mg_480p = 0;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = uri_msg->aps_copy_control_info;
            cgms.cgms = uri_msg->emi_copy_control_info;
        }
        else//If the Host has not made a copy, then go to use case: Copy No More
        {
            mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = uri_msg->aps_copy_control_info;
            cgms.cgms = 0x01;
        }
        image_constraint = uri_msg->ict_copy_control_info;//0=>no; 1=>520k

        hdcp = 1;//yes
        dtcp_epn = (uri_msg->rct_copy_control_info==0 ? 1 : 0);
        dtcp_e_cci = uri_msg->emi_copy_control_info;
        dtcp_ict = (uri_msg->ict_copy_control_info==0 ? 1 : 0);
        dtcp_aps = uri_msg->aps_copy_control_info;
        spdif.copyright = 0;
        spdif.l_bit = 0;
    }
    else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x03)
    {
        mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
        cgms.aps = uri_msg->aps_copy_control_info;
        cgms.cgms = uri_msg->emi_copy_control_info;
        image_constraint = uri_msg->ict_copy_control_info;//0=>no; 1=>520k

        hdcp = 1;//yes
        dtcp_epn = (uri_msg->rct_copy_control_info==0 ? 1 : 0);
        dtcp_e_cci = uri_msg->emi_copy_control_info;
        dtcp_ict = (uri_msg->ict_copy_control_info==0 ? 1 : 0);
        dtcp_aps = uri_msg->aps_copy_control_info;
        spdif.copyright = 0;
        spdif.l_bit = 1;
    }
    else if(uri_msg->rct_copy_control_info == 1 && uri_msg->emi_copy_control_info == 0x00)
    {
        mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
        cgms.aps = 0x00;
        cgms.cgms = uri_msg->emi_copy_control_info;
        image_constraint = uri_msg->ict_copy_control_info;//0=>no; 1=>520k

        hdcp = 1;//yes
        dtcp_epn = (uri_msg->rct_copy_control_info==0 ? 1 : 0);
        dtcp_e_cci = uri_msg->emi_copy_control_info;
        dtcp_ict = (uri_msg->ict_copy_control_info==0 ? 1 : 0);
        dtcp_aps = uri_msg->aps_copy_control_info;
        spdif.copyright = 1;
        spdif.l_bit = 0;
    }
    else
    {
        //shall revert to the final default URI when no subsequent URI is received within the timeout
        APPL_PRINTF("%s: URI is illegal and shall not be transmitted!\n", __FUNCTION__);
        return;
    }

    //keep last parameters
    last_mg = mg_480p;
    last_aps = cgms.aps;
    last_cgms = cgms.cgms;

    //operate with these parameters
    //1.Mg--only support 576i and 480i mode
    //while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_close_analog_vpo, NULL, NULL));

    l_cgms_info.aps = cgms.aps;
    l_cgms_info.cgms = cgms.cgms;
    l_mg_info = mg_480p;

#ifndef CI_NO_MG
#ifndef CI_PLUS_TEST_MG_OR_CGMS        //for release
    //for MV Patch enable while receive URI, Unreg and Reg DAC
    api_ci_msg_to_app(slot, API_MSG_ENABLE_ANALOG_OUTPUT);
    api_ci_msg_to_app(slot, API_MSG_MGCGMS_SET_VPO);

#else                                //for VM700T test
#ifndef CI_PLUS_TEST_ONLY_CGMS
    api_ci_msg_to_app(slot, API_MSG_SET_MG_INFO);
#else
    api_ci_msg_to_app(slot, API_MSG_SET_CGMS_INFO);
#endif
#endif
#else
    //GXX project: no license, close analog output
    //According to table in CI_Plus_DEVICE_INTERIM_LICENSE_AGREEMENT_2008_12_16r1.pdf
    if((uri_msg->rct_copy_control_info == 0 && (uri_msg->emi_copy_control_info == 0x00))
        || (uri_msg->rct_copy_control_info == 1 && uri_msg->emi_copy_control_info == 0x00 && uri_msg->aps_copy_control_info == 0x00))
    {
        if(dis_analog_out == 1)
        {
            dis_analog_out = 0;
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
            dis_yuv_output_for_ict = 0;
#endif
            api_ci_msg_to_app(slot, API_MSG_ENABLE_ANALOG_OUTPUT);
        }
    }
    else
    {
    /* No analog protect, we should close Analog output when recieve protected URI */
        //just unreg Analog DAC
        if(dis_analog_out == 0)
        {
            dis_analog_out = 1;
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
            dis_yuv_output_for_ict = 1;
#endif
            api_ci_msg_to_app(slot, API_MSG_DISABLE_ANALOG_OUTPUT);
        }
    }
#endif

    //3.SPDIF
    snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_SPDIF_SCMS, (UINT32)&spdif);

    //4.HDMI
    api_set_hdmi_hdcp_onoff(TRUE);

    //5.ICT
#ifndef CI_PLUS_NO_YPBPR
    //Operation Mode 2--Analog SD(DE scaling down) + HDMI SD(DE scaling down) + CVBS
    if((uri_msg->ict_copy_control_info == 1)
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
     && (ict_state == 0)   //avoid flickering exit from menu
#endif
     && (dis_analog_out == 0))
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
        if((!dis_info.bprogressive) && (dis_info.tvsys <= SECAM))
            return;
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
        api_ci_msg_to_app(slot, API_MSG_DISABLE_YUV_OUTPUT);
        dis_yuv_output_for_ict = 1;
#else
        api_ci_msg_to_app(slot, API_MSG_CI_ICT_ENABLE);
#endif
    }
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
    else if (uri_msg->ict_copy_control_info == 0
             && dis_yuv_output_for_ict == 1
             && dis_analog_out == 0) {
        api_ci_msg_to_app(slot, API_MSG_ENABLE_YUV_OUTPUT);
        dis_yuv_output_for_ict = 0;
    }

#endif
    ict_state = uri_msg->ict_copy_control_info;

    //Operation Mode 3--Analog SD(DE scaling down) + HDMI Still
    //osal_task_sleep(50);
    //if(uri_msg->ict_copy_control_info == 1)
    //    vpo_ioctl(g_vpo_dev, VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEO);
    //else
    //    vpo_ioctl(g_vpo_dev, VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEN);
#endif

#ifdef CI_PLUS_TEST_CAM_USED
    if (!api_ci_camup_in_progress())
    {
        api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
        api_ci_msg_to_stack(slot, API_MSG_EXIT_MENU);
    }
#endif

}

void ci_notify_uri(int slot, struct ci_uri_message *uri_msg)
{
    struct ci_cc_device *cc_dev;

    if (NULL == (cc_dev = ci_cc_get_dev(slot)))
        return;
    else
    {
        if (NULL == uri_msg)
            return;
        else
        {
            uri_msg->program_number = cc_dev->program_number;
        }
    }

    APPL_PRINTF("%s\n", __FUNCTION__);

    //when user channel change frequently in PUB, need sync with Stack for CI+ flow is too long!
    if (ci_get_state(slot) == CI_STATE_START)
    {
        enter_dsc_mutex();    //20101102, fix cc, not scramble issue

        notify_uri(slot, uri_msg);

        apply_uri(slot, uri_msg);

        if (uri_msg->emi_copy_control_info != 0)
        {
            //CI+ start descrambler
            if (!api_ci_camup_in_progress())
                ciplus_dsc_start(slot);
        }
        leave_dsc_mutex(); //20101102, fix cc, not scramble issue

    }
    else
    {
        APPL_PRINTF("%s: ci stopped!\n", __FUNCTION__);
    }

}

void pvr_notify_uri(int slot, struct ci_uri_message *uri_msg)
{
    APPL_PRINTF("%s\n", __FUNCTION__);

    notify_uri(slot, uri_msg);

    apply_uri(slot, uri_msg);

}

void ci_clear_uri()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    if(uri_data.uri_msg.uri_from_pvr == 0)
    {
        MEMSET(&uri_data, 0, sizeof(struct UIR_DATA));
        last_mg = 0;
        last_aps = 0;
        last_cgms = 0;
    }

    MEMSET(&live_uri_data, 0, sizeof(struct UIR_DATA));
}

void pvr_clear_uri()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    MEMSET(&uri_data, 0, sizeof(struct UIR_DATA));
}

void uri_resume_from_live()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    MEMCPY(&uri_data, &live_uri_data, sizeof(struct UIR_DATA));
    apply_uri(uri_data.slot, &uri_data.uri_msg);
}

void uri_handle_after_cam_out(int slot)
{
    UINT8 mg_480p;
    struct vdec_status_info cur_status;

    APPL_PRINTF("%s\n", __FUNCTION__);

    MEMSET(&uri_data_bak, 0x00, sizeof(struct UIR_DATA));

    if(uri_data.uri_msg.uri_from_pvr == 0 && uri_data.state == URI_RECEIVED)
    {
        if (last_mg || last_aps || last_cgms)    //MG/CGMS-A
        {
            APPL_PRINTF("%s: MG/CGMS-A resume\n", __FUNCTION__);
            //resume analog output
#ifdef CI_NO_MG
            dis_analog_out = 0;
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
            dis_yuv_output_for_ict = 0;
#endif
            api_ci_msg_to_app(slot, API_MSG_ENABLE_ANALOG_OUTPUT);
            osal_task_sleep(50);
#endif

            //stop CGMS-A output
#ifndef CI_NO_MG
#ifndef CI_PLUS_TEST_MG_OR_CGMS

            l_cgms_info.aps = 0;
            l_cgms_info.cgms = 0;
            l_mg_info = 0;

            api_ci_msg_to_app(slot, API_MSG_MGCGMS_SET_VPO);
#endif
#endif
            //when poll-out CI+ CAM, should not showed any remained pictures!
            /* vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&CurStatus);
            if(CurStatus.uCurStatus == VDEC27_STARTED)
            {
                vdec_stop(get_selected_decoder(), TRUE, FALSE);
                vdec_start(get_selected_decoder());
            } */
        }
        else
        {
            //do nothing
        }

        if (uri_data.uri_msg.ict_copy_control_info)    //ICT
        {
            //resume from ICT
            APPL_PRINTF("%s: ICT resume\n", __FUNCTION__);
            ci_clear_uri();        //the ICT effect resume TV sys, need clear here!
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
            api_ci_msg_to_app(slot, API_MSG_RESUME_TV_MODE);
#else
#ifndef CI_PLUS_NO_YPBPR
            api_ci_msg_to_app(slot, API_MSG_ENABLE_YUV_OUTPUT);
            dis_yuv_output_for_ict = 0;
#endif
#endif
        }

        if (uri_data.uri_msg.aps_copy_control_info != 0 /* api_mg_control() */)
        {
#ifndef CI_PLUS_NO_YPBPR
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
            ci_clear_uri();
            api_ci_msg_to_app(slot, API_MSG_ENABLE_YUV_OUTPUT);
            dis_yuv_output_for_ict = 0;
#endif
#endif
        }
    }
}

void uri_set_received()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    enter_uri_mutex;

    uri_data.state = URI_RECEIVED;

    leave_uri_mutex;
}

// Set uri to default one
void api_uri_clear()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    enter_uri_mutex;

    uri_data.state = URI_CLEAR;
    set_uri_default(&uri_data.uri_msg);
    /* it cause analog enable uncorrectly at the case: when live uri disable analog, goto PVR, live uri was clear,
       then exist PVR, stb enable analog uncorrectly.
    comment it: it is save because only uri_resume_from_live() use it when exist from PVR
    MEMSET(&live_uri_data, 0, sizeof(struct UIR_DATA));
    */

    leave_uri_mutex;
}

void api_uri_bak()
{
    enter_uri_mutex;
    MEMCPY(&uri_data_bak, &uri_data, sizeof(struct UIR_DATA));
    leave_uri_mutex;
}

void api_uri_bak_resume()
{
    enter_uri_mutex;
    MEMCPY(&uri_data, &uri_data_bak, sizeof(struct UIR_DATA));
    leave_uri_mutex;
}

void api_uri_bak_clear()
{
    APPL_PRINTF("%s\n", __FUNCTION__);
    enter_uri_mutex;

    uri_data_bak.state = URI_CLEAR;
    set_uri_default(&uri_data_bak.uri_msg);

    leave_uri_mutex;
}

BOOL api_analog_output_enable()
{
    return !dis_analog_out;
}

void uri_enable_analog_output()
{
    APPL_PRINTF("%s\n", __FUNCTION__);

    if(dis_analog_out == 1)
    {
        dis_analog_out = 0;
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
        dis_yuv_output_for_ict = 0;
#endif
        api_ci_msg_to_app(ci_slot, API_MSG_ENABLE_ANALOG_OUTPUT);
        osal_task_sleep(50);
    }
}

#ifndef CI_PLUS_TEST_CASE

//#define MAX_URI_DATA_LEN 1024
//static UINT8 uri_buf[MAX_URI_DATA_LEN];

BOOL api_ict_control()
{
    struct ci_uri_message uri_msg_data;
    struct ci_uri_message *uri_msg = &uri_msg_data;
    enum URI_STATE state;

    state = uri_get_msg(uri_msg);

    if (state == URI_RECEIVED && (uri_msg->ict_copy_control_info == 1))
    {
        return TRUE;
    }
    else
        return FALSE;
}

BOOL api_mg_control()
{
    struct ci_uri_message uri_msg_data;
    struct ci_uri_message *uri_msg = &uri_msg_data;
    enum URI_STATE state;

    state = uri_get_msg(uri_msg);

    if (state == URI_RECEIVED)
    {
        if((uri_msg->rct_copy_control_info == 0 && (uri_msg->emi_copy_control_info == 0x01 || (uri_msg->emi_copy_control_info == 0x02 && uri_msg->uri_from_pvr == 0x01) || uri_msg->emi_copy_control_info == 0x03))
            ||(uri_msg->rct_copy_control_info == 1 && uri_msg->emi_copy_control_info == 0x00))
        {
            if(uri_msg->aps_copy_control_info != 0)
                return TRUE;
        }
    }
    return FALSE;
}

void api_mg_cgms_reset()
{
    struct ci_uri_message uri_msg_data;
    struct ci_uri_message *uri_msg = &uri_msg_data;
    UINT8 mg_480p;
    struct vpo_io_cgms_info cgms;
    enum URI_STATE state;

    state = uri_get_msg(uri_msg);

    if (state == URI_RECEIVED)
    {
        if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x00)
        {
            mg_480p = 0;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = 0x00;
            cgms.cgms = uri_msg->emi_copy_control_info;
        }
        else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x01)
        {
            mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = uri_msg->aps_copy_control_info;
            cgms.cgms = uri_msg->emi_copy_control_info;
        }
        else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x02)
        {
            if(uri_msg->uri_from_pvr == 0)//If the Host has not made one copy, then go to use case: Copy Once, now default
            {
                mg_480p = 0;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
                cgms.aps = uri_msg->aps_copy_control_info;
                cgms.cgms = uri_msg->emi_copy_control_info;
            }
            else//If the Host has not made a copy, then go to use case: Copy No More
            {
                mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
                cgms.aps = uri_msg->aps_copy_control_info;
                cgms.cgms = 0x01;
            }
        }
        else if(uri_msg->rct_copy_control_info == 0 && uri_msg->emi_copy_control_info == 0x03)
        {
            mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = uri_msg->aps_copy_control_info;
            cgms.cgms = uri_msg->emi_copy_control_info;
        }
        else if(uri_msg->rct_copy_control_info == 1 && uri_msg->emi_copy_control_info == 0x00)
        {
            mg_480p = uri_msg->aps_copy_control_info;//0=>none; 1=>AGC; 2=>AGS+2CS; 3=>AGC+4CS
            cgms.aps = 0x00;
            cgms.cgms = uri_msg->emi_copy_control_info;
        }
        else
        {
            //shall revert to the final default URI when no subsequent URI is received within the timeout
            return;
        }

        l_cgms_info.aps = cgms.aps;
        l_cgms_info.cgms = cgms.cgms;
        l_mg_info = mg_480p;

#ifndef CI_PLUS_TEST_MG_OR_CGMS        //for release
        api_ci_msg_to_app(ci_slot, API_MSG_MGCGMS_SET_VPO);   //we don't care about the slot info here

#else                                //for VM700T test
#ifndef CI_PLUS_TEST_ONLY_CGMS
        api_ci_msg_to_app(ci_slot, API_MSG_SET_MG_INFO);
#else
        api_ci_msg_to_app(ci_slot, API_MSG_SET_CGMS_INFO);
#endif
#endif
    }
}

#else
void api_mg_cgms_reset()

{
    return;
}

#endif
#endif

