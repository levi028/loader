/*****************************************************************************
*    ALi Corp. All Rights Reserved. 2009 Copyright (C)
*
*    File:    ci_app_plus.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack - application layer system management
*                     resources of CI+.
*    History:
*               Date                Athor            Version          Reason
*        ==========   ========   =======    =========    =================
*    1.    Jul.20.2009       Steven          Ver 0.1          Add new resources for CI+
*
*****************************************************************************/

#include <sys_config.h>

#ifdef CI_PLUS_SUPPORT
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <api/libtsi/db_3l.h>
#include <openssl/aes.h>
#include <openssl/x509.h>

#include <api/libci/ci_plus.h>

#include "ci_stack.h"
#include <api/libci/ci_cc_api.h>
#include "ci_license.h"
#include "ci_srm.h"

//#define TEST_CASE1_NONCE_INVALID
//#define TEST_CASE2_DHPH_INVALID
//#define TEST_CASE3_CERT_INVALID
//#define TEST_CASE4_CERT_VERIFY_FAIL
//#define TEST_CASE5_CERT_EXPIRED
//#define TEST_CASE6_SIGB_VERIFY_FAIL
//#define TEST_CASE7_DHPM_INVALID
//#define TEST_CASE8_AKH_INVALID
//#define TEST_CASE9_DHSK_INVALID
//#define TEST_CASE10_CC_CAMID_NOT_MATCH
//#define TEST_CASE10_CAL_CCK_FAIL
//#define TEST_CASE10_CCK_HOST_BUSY
//#define TEST_CASE10_CCK_RESERVED_STATUS
//#define TEST_CASE11_SAC_CAMID_NOT_MATCH
//#define TEST_CASE11_SAC_HOST_BUSY
//#define TEST_CASE11_SAC_RESERVED_STATUS
//#define TEST_CASE11_SAC_MSG_COUNTER_INVALID
//#define TEST_CASE11_SAC_PROTOCOL_INVALID
//#define TEST_CASE11_SAC_VERIFY_FAIL


//see Annex F Error Code Definition and Handling
#define CIPLUS_ARC(...)

//Table 5.12
#define URI_DEFAULT        0x01
#define EMI_DEFAULT        0x03
#define ICT_FINAL_DEFAULT     0x01

static BOOL ci_camup_timeout = FALSE;
static BOOL ci_camup_in_progress = FALSE;

void (*ci_camup_progress_callback)(int percent);
void (*ci_play_channel_callback)(UINT32 prog_id);

/* CI+: Host Language and Country resource */
static void ci_hlc_delete(struct ci_senssion_connect *sc);
static INT32 ci_hlc_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

struct ci_senssion_connect* ci_hlc_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    ci_senss_conn_init(sc, scnb, RSC_ID_HOST_LANGUAGE_COUNTRY, tc);
    sc->appdelete = ci_hlc_delete;
    sc->callback = ci_hlc_process;
    sc->appdata = NULL;
    return sc;
}

static void ci_hlc_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

static INT32 ci_hlc_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    int tag;
    UINT8 iso_3166_country_code[3] = CI_ISO3166_COUNTRY_CODE;    /*ISO 3166-1 alpha 3*/
    UINT8 iso_639_2_language_code[3] = CI_ISO639_2_LANGUAGE_CODE;    /*ISO 639 Part 2*/

    APPL_PRINTF("ci_hlc_process: data %X, len %d\n", data, len);
    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_HOST_COUNTRY_ENQ:
            APPL_PRINTF("ci_hlc_process: %d: <== host country enq\n", sc->scnb);
            APPL_PRINTF("ci_hlc_process: %d: ==> host country\n", sc->scnb);
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_HOST_COUNTRY, 3, iso_3166_country_code) != SUCCESS)
            {
                APPL_PRINTF("ci_hlc_process: Send APPOBJ_TAG_HOST_COUNTRY failure\n");
                return ERR_FAILURE;
            }

            break;
        case APPOBJ_TAG_HOST_LANGUAGE_ENQ:
            APPL_PRINTF("ci_hlc_process: %d: <== host language enq\n", sc->scnb);
            APPL_PRINTF("ci_hlc_process: %d: ==> host language\n", sc->scnb);
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_HOST_LANGUAGE, 3, iso_639_2_language_code) != SUCCESS)
            {
                APPL_PRINTF("ci_hlc_process: Send APPOBJ_TAG_HOST_LANGUAGE failure\n");
                return ERR_FAILURE;
            }

            break;
        default:
            APPL_PRINTF("ci_hlc_process: host country & language: unknown tag %06X\n", tag);
            //return ERR_FAILURE;
            break;
        }
    }
    return SUCCESS;
}

/* CI+: CAM Upgrade Resource */
static void ci_camup_delete(struct ci_senssion_connect *sc);
static INT32 ci_camup_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

struct ci_senssion_connect* ci_camup_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    ci_senss_conn_init(sc, scnb, RSC_ID_CAM_UPGRADE, tc);
    sc->appdelete = ci_camup_delete;
    sc->callback = ci_camup_process;
    sc->appdata = NULL;

    return sc;
}

static void ci_camup_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

extern UINT32 ciplus_camup_prog_id;
static UINT8 ci_cur_slot;
static UINT32 g_ci_camup_end_prog = 0;
static BOOL g_ci_camup_can_answer = TRUE;
#define uimsbf16(data)    ((data[0]<<8)|data[1])

void api_ci_camup_answer_setstatus(BOOL can_answer)
{
    g_ci_camup_can_answer = can_answer;
}

BOOL api_ci_camup_answer_getstatus()
{
    return g_ci_camup_can_answer;
}


static INT32 cam_firmware_upgrade(int slot, int len, UINT8 *data, UINT8 *answer)
{
    UINT8 upgrade_type;    //0x00: Delayed Upgrade mode
                        //0x01: Immediate Upgrade mode
    UINT16 download_time;

    if (len < 3)
        return ERR_FAILURE;

    upgrade_type = *data++;
    download_time = uimsbf16(data);
    APPL_PRINTF("cam_firmware_upgrade: type %d, time %d\n",upgrade_type,download_time);

    //TO DO...
    ci_notify_cam_upgrade(upgrade_type, download_time, answer);
    //api_ci_msg_to_app(slot, API_MSG_CI_CAMUP);

    return SUCCESS;
}

BOOL api_ci_camup_timeout()
{
    return ci_camup_timeout;
}

BOOL api_ci_camup_in_progress()
{
    return ci_camup_in_progress;
}

void api_ci_camup_init_progress()
{
    ci_camup_in_progress = FALSE;
}

static void ci_camup_monitor(struct ci_senssion_connect *sc, void *param)
{
    UINT32 prog_id;
    UINT8 *slot = (UINT8 *)param;
    APPL_PRINTF("ci_camup_monitor: Upgrade timeout!\n");

    ci_camup_timeout = TRUE;
    ci_camup_in_progress = FALSE;
/* Just process in app
    //resume to normal operation
    if (ci_camup_progress_callback)
        ci_camup_progress_callback(100);
*/
    api_ci_msg_to_app(*slot, API_MSG_CAMUP_OK);
/*
    // For CAM plugout timeout screen freezen case
    if (NULL != param)
    {
        prog_id = ci_get_cur_program_id(*slot);
        if (ci_play_channel_callback)
            ci_play_channel_callback(prog_id);
    }
*/
    //PCMCIA reset
    if((NULL != sc) && (NULL != sc->tc) && (NULL != sc->tc->link)
     && (ci_reset_cam(sc->tc->link, sc->tc->slot) != SUCCESS))
    {
        APPL_PRINTF("ci_camup_monitor: PCMCIA reset failed!\n");
    }

    FREE(slot);
}

INT32 api_ci_register_callback(char *cic_name, void (*callback)(int percent))
{
    ci_camup_progress_callback = callback;
}

INT32 api_ci_register_playch_callback(char *cic_name, void (*callback)(UINT32 prog_id))
{
    ci_play_channel_callback = callback;
}

UINT32 api_get_camup_end_prog()
{
    return g_ci_camup_end_prog;
}

static void ci_camup_info_monitor(UINT8 *slot, void *param)
{
    UINT32 *prog_id = (UINT32 *)MALLOC(sizeof(UINT32));

    MEMCPY(prog_id, param, sizeof(UINT32));

    if(1 == ci_get_mmi_visualstatus() && 1 == api_ci_get_mmi_enablestatus())
    {
        ci_set_timer(300, (ci_timer_handler)ci_camup_info_monitor, (void *)slot, prog_id);
    }
    else
    {
        //CI+ CAM have strictly timing, we must play channel here, or
        //we can't descramble program after re-insertion
        /* osal_interrupt_disable();
        if (ci_play_channel_callback)
            ci_play_channel_callback (*prog_id);
        osal_interrupt_enable(); */
        //osal_task_sleep(3000);
        api_ci_msg_to_app(*slot, API_MSG_CAMUP_END_PLAY);
        if (!api_ci_camup_in_progress())
            api_ci_not_selected(*slot);
        while (SUCCESS == ci_del_timer((ci_timer_handler)ci_camup_info_monitor, (void *)CI_TIMER_PARAM_ANY, (void *)CI_TIMER_PARAM_ANY));
    }
}

static INT32 ci_camup_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    int tag;
    int l;
    UINT8 *d;
    UINT8 answer = 0x02;//0x00 means NO.
                        //0x01 means YES.
                        //0x02 means ASK the user. The CICAM shall open MMI dialogue to get feedback from the user.
    UINT8 download_progress_status = 0;    //[0-100]
    UINT8 reset_request_status; //0x00 PCMCIA reset requested 每 The host sets the RESET signal active then inactive.
                                //0x01 CI Plus CAM reset requested 每 Host sets the RS flag and begins interface initialisation
                                //0x02 No reset required 每 Normal Operation continues
    UINT32 prog_id = 0;
    static BOOL is_normal_complete = FALSE;

    UINT8 *slot = (UINT8 *)MALLOC(sizeof(UINT8));

    APPL_PRINTF("ci_camup_process: data %X, len %d\n", data, len);
    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        l = 0;
        d = ci_senss_get_apdu(sc, &l, data);
        switch (tag)
        {
        case APPOBJ_TAG_CAM_FIRMWARE_UPGRADE:
            APPL_PRINTF("ci_camup_process: %d: <== cam firmware upgrade\n", sc->scnb);
            if (l >0 && d != NULL)
            {
                cam_firmware_upgrade(sc->tc->slot, l, d, &answer);
            }
            else
            {
                APPL_PRINTF("ci_camup_process: bad apdu of CamFirmwareUpgrade\n");
            }

            //Answer according current status
            //TO DO...

            ci_camup_timeout = FALSE;

            APPL_PRINTF("ci_camup_process: %d: ==> cam firmware upgrade reply %d\n", sc->scnb,answer);
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_REPLY, 1, &answer) != SUCCESS)
            {
                APPL_PRINTF("ci_camup_process: Send APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_REPLY failure\n");
                return ERR_FAILURE;
            }

            g_ci_camup_can_answer = FALSE;
            ci_camup_in_progress = TRUE;

            break;
        case APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_PROGRESS:

            *slot = sc->tc->slot;
            //When no chosen, we never have progress
            ci_camup_in_progress = TRUE;
            //TO DO...
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_camup_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            ci_set_timer(CI_DURATION_CAMUP_GUARD, (ci_timer_handler)ci_camup_monitor, (void *)sc, (void *)slot);

            if (l >0 && d != NULL)
            {
                download_progress_status = *d;
                APPL_PRINTF("ci_camup_process: %d: <== cam firmware upgrade progress %d\n", sc->scnb,download_progress_status);
                //show upgrade progress
                if (ci_camup_progress_callback)
                    ci_camup_progress_callback(download_progress_status);

                if (100 == download_progress_status && !is_normal_complete)
                    is_normal_complete = TRUE;
            }
            else
            {
                APPL_PRINTF("ci_camup_process: %d: download progress is NULL\n", sc->scnb);
            }

            break;
        case APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_COMPLETE:
            APPL_PRINTF("ci_camup_process: %d: <== cam firmware upgrade complete\n", sc->scnb);

            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_camup_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            ci_camup_timeout = FALSE;

            //do resume to normal operation
            if (ci_camup_progress_callback)
                ci_camup_progress_callback(100);

            if (l == 0 || d == NULL)
            {
                APPL_PRINTF("no reset_request_status\n");
                //CICAM wishes to cancel the firmware upgrade
                //TO DO...
            }
            else
            {
                reset_request_status = *d;
                APPL_PRINTF("reset_request_status: %d\n", reset_request_status);
                //TO DO...

                if (is_normal_complete)
                {
                    g_ci_camup_can_answer = TRUE;
                    is_normal_complete = FALSE;
                }

                if(reset_request_status == 0x00)
                {
                    /*
                    //after CAM Reset, need to descramble current channel
                    prog_id = ci_get_cur_program_id(sc->tc->slot);
                    // api_play_channel(prog_id, TRUE, FALSE, TRUE);
                    */
                    APPL_PRINTF("\t\tReset_request_status=%d,CAM need reset!\n",reset_request_status);
                    /*
                    if (ci_play_channel_callback)
                       ci_play_channel_callback (prog_id);
                       */

                       /*Just process in app*/
                        api_ci_msg_to_app(sc->tc->slot, API_MSG_CAMUP_OK);

                    //PCMCIA reset
                    if(ci_reset_cam(sc->tc->link, sc->tc->slot) != SUCCESS)
                    {
                        APPL_PRINTF("Upgrade completed and PCMCIA reset failed!\n");
                    }
                }
                else if(reset_request_status == 0x01)
                {
                    /*/after CAM Reset, need to descramble current channel
                    prog_id = ci_get_cur_program_id(sc->tc->slot);
                    // api_play_channel(prog_id, TRUE, FALSE, TRUE);
                    */
                    APPL_PRINTF("\t\tReset_request_status=%d,CAM need reset!\n",reset_request_status);
                    /*
                    if (ci_play_channel_callback)
                        ci_play_channel_callback (bak_ci_camup_prog_id);
                       */
                        api_ci_msg_to_app(sc->tc->slot, API_MSG_CAMUP_OK);

                    //CI Plus CAM RS reset
                    if(ci_rs_reset_cam(sc->tc->link, sc->tc->slot) != SUCCESS)
                    {
                        APPL_PRINTF("Upgrade completed and RS reset failed!\n");
                    }
                }
                else if(reset_request_status == 0x02)
                {
                    //NO reset

                    APPL_PRINTF("\t\tReset_request_status=%d,CAM need't reset!\n",reset_request_status);
            #ifdef CI_PLUS_TEST_CAM_USED //we don't know if product CAM also need no reset case
                    g_ci_camup_end_prog = ci_get_cur_program_id(sc->tc->slot);
                    ci_cur_slot = sc->tc->slot;
                    // When yes chosen and no need reset CAM, we play the program after HINT disappear
                    ci_camup_info_monitor((void *)&ci_cur_slot, (void *)&g_ci_camup_end_prog);
            #endif
                }
            }

            ci_camup_in_progress = FALSE;

            break;
        default:
            APPL_PRINTF("ci_camup_process: CAM Upgrade unknown tag %06X\n", tag);
            //return ERR_FAILURE;
            break;
        }
    }
    return SUCCESS;
}

/* Application MMI Resource */
static void ci_appmmi_delete(struct ci_senssion_connect *sc);
static INT32 ci_appmmi_process(struct ci_senssion_connect *sc, int len, UINT8 *data);
static INT32 ci_appmmi_request_start(struct ci_senssion_connect *sc,
                    int len, UINT8 *data);
static INT32 ci_appmmi_file_acknowledge(struct ci_senssion_connect *sc,
                        int len, UINT8 *data);
static INT32 ci_appmmi_abort_request(int len, UINT8 *data, int slot);
static INT32 ci_appmmi_abort_ack(int len, UINT8 *data);

struct ci_senssion_connect* ci_appmmi_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_appmmi *tp;

    ci_senss_conn_init(sc, scnb, RSC_ID_APPLICATION_MMI, tc);
    sc->appdelete = ci_appmmi_delete;
    sc->callback = ci_appmmi_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_appmmi))) == NULL)
    {
        APPL_PRINTF("ci_appmmi_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_appmmi*)(sc->appdata);
    return sc;
}

static void ci_appmmi_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

static INT32 ci_appmmi_request_start(struct ci_senssion_connect *sc,
                    int len, UINT8 *data)
{
    struct ci_appmmi *tp;
    UINT8 app_domain_identifier_length;
    UINT8 initial_object_length;
    int i;

    APPL_PRINTF("ci_appmmi_request_start: data %X, len %d\n", data, len);

    if (sc->appdata == NULL)
    {
        return ERR_FAILURE;
    }
    tp = (struct ci_appmmi*)(sc->appdata);

    //init parameter
    tp->app_domain_identifier_length = 0;
    tp->initial_object_length = 0;

    if (len < 2)
    {
        APPL_PRINTF("ci_appmmi_request_start: buffer length %d < %d\n",len,2);
        return ERR_FAILURE;
    }

    app_domain_identifier_length = *data++;
    initial_object_length = *data++;

    if (len < 2+app_domain_identifier_length)
    {
        APPL_PRINTF("ci_appmmi_request_start: buffer length %d < %d\n",len,2+app_domain_identifier_length);
        return ERR_FAILURE;
    }

    tp->app_domain_identifier_length = app_domain_identifier_length;
    MEMCPY(tp->app_domain_identifier, data, app_domain_identifier_length);
    APPL_PRINTF("ci_appmmi_request_start: AppDomainIdentifier %d:\n",app_domain_identifier_length);
    for (i = 0; i < app_domain_identifier_length; i++)
    {
        //AppDomainIdentifier 8 bslbf
        APPL_PRINTF("%c",data[i]);
    }
    APPL_PRINTF("\n");
    data += app_domain_identifier_length;

    if (len < 2+app_domain_identifier_length+initial_object_length)
    {
        APPL_PRINTF("ci_appmmi_request_start: buffer length %d < %d\n",len,2+app_domain_identifier_length+initial_object_length);
        return ERR_FAILURE;
    }

    tp->initial_object_length = initial_object_length;
    MEMCPY(tp->initial_object, data, initial_object_length);
    APPL_PRINTF("ci_appmmi_request_start: InitialObject %d:\n",initial_object_length);
    for (i = 0; i < initial_object_length; i++)
    {
        //InitialObject 8 bslbf
        APPL_PRINTF("%c",data[i]);
    }
    APPL_PRINTF("\n");
    data += initial_object_length;

    return SUCCESS;
}

//buffer: OUT, AppDomainIdentifierLength[1] + AppDomainIdentifier[n] + InitialObjectLength[1] + InitialObject[n]
INT32 ci_appmmi_get_initial_object(void *buffer, int len, struct ci_senssion_connect *sc)
{
    struct ci_appmmi *tp;
    UINT8 *data = buffer;

    *(int*)(buffer) = 0;
    if (sc->appdata == NULL)
    {
        return ERR_FAILURE;
    }
    tp = (struct ci_appmmi*)(sc->appdata);

    if (len < tp->app_domain_identifier_length + tp->initial_object_length + 2)
    {
        APPL_PRINTF("ci_appmmi_get_initial_object: buffer length %d < %d\n",len,tp->app_domain_identifier_length + tp->initial_object_length + 2);
        return ERR_FAILURE;
    }

    data[0] = tp->app_domain_identifier_length;
    MEMCPY(data+1, tp->app_domain_identifier, tp->app_domain_identifier_length);
    data[tp->app_domain_identifier_length + 1] = tp->initial_object_length;
    MEMCPY(data+1+tp->app_domain_identifier_length+1, tp->initial_object, tp->initial_object_length);

    return SUCCESS;
}

//AckCode: IN,
//0x01 OK
//     The application execution environment will attempt to load an execute the
//     initial object specified in the RequestStart message.
//0x02 Wrong API
//     Application domain not supported.
//0x03 API busy
//     Application domain supported but not currently available.
//0x80 to 0xFF Domain specific API busy
//             Application domain specific responses equivalent to response 0 x 03 but
//             providing application domain specific information on why the execution
//             environment is busy, (or not available for some other reason such as
//             resource contention), when it will become available etc.
INT32 ci_appmmi_request_start_ack(struct ci_senssion_connect *sc, UINT8 ack_code)
{
    APPL_PRINTF("ci_appmmi_request_start_ack: send AckCode 0x%02X\n",ack_code);
    if (SUCCESS != ci_senss_send_apdu(sc, APPOBJ_TAG_REQUEST_START_ACK, 1, &ack_code))
    {
        APPL_PRINTF("ci_appmmi_request_start_ack: Send APPOBJ_TAG_REQUEST_START_ACK failure\n");
        return ERR_FAILURE;
    }

    return SUCCESS;
}

//RequestType: IN,
//File 0x00
//Data 0x01
INT32 ci_appmmi_file_request(struct ci_senssion_connect *sc,
                            UINT8 request_type, UINT8 *data, int len)
{
/*    struct ci_appmmi_file_request {
        UINT8 RequestType;
        UINT8 DataByte[256];
    } file_request;
*/
    int i;
    UINT8 file_request[258];

    APPL_PRINTF("ci_appmmi_file_request: RequestType %d, data %X, len %d\n", request_type,data,len);
    for (i = 0; i < len; i++)
    {
        //FileNameByte
        APPL_PRINTF("%c",data[i]);
    }
    APPL_PRINTF("\n");

    if (len <= 256)
    {
        file_request[0] = request_type;
/*
        if (MEMCMP("CI://Apps/PNG/Apps/PNG/PNG.mhg", data, len) == 0)
        {
            len = 21;
            MEMCPY(&file_request[1], "CI://Apps/PNG/PNG.mhg", 21);
        } else if (MEMCMP("CI://PNG.mhg", data, len) == 0)
        {
            len = 12;
            MEMCPY(&file_request[1], "CI://startup", 12);
        }else if (MEMCMP("CI://Apps/CharSet/Apps/CharSet/Chars.mhg", data, len) == 0)
        {
            len = 27;
            MEMCPY(&file_request[1], "CI://Apps/CharSet/Chars.mhg", 27);
        }
        else
*/
        MEMCPY(&file_request[1], data, len);

        return ci_senss_send_apdu(sc, APPOBJ_TAG_FILE_REQUEST, len + 1,
                  file_request);
    }
    else
    {
        APPL_PRINTF("ci_appmmi_file_request: data len > 256\n");
        return ERR_FAILURE;
    }
}

#define uimsbf32(data)    ((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3])
static INT32 ci_appmmi_file_acknowledge(struct ci_senssion_connect *sc,
                        int len, UINT8 *data)
{
    struct ci_appmmi *tp;
    UINT8 file_ok;
    UINT8 request_type;
    UINT8 file_name_length;
    UINT32 file_data_length;
    int i;

    APPL_PRINTF("ci_appmmi_file_acknowledge: data %X, len %d\n", data, len);

    if (sc->appdata == NULL)
    {
        return ERR_FAILURE;
    }
    tp = (struct ci_appmmi*)(sc->appdata);

    //init parameter
    tp->file_name_length = 0;
    tp->data_length = 0;

    if (len < 2)
    {
        APPL_PRINTF("ci_appmmi_file_acknowledge: len %d < 2\n",len);
        return ERR_FAILURE;
    }

    file_ok = data[0]&0x01;
    request_type = data[1];
    data += 2;
    APPL_PRINTF("ci_appmmi_file_acknowledge: FileOK %d, RequestType %d\n",file_ok,request_type);

    if (request_type == CI_APPMMI_REQ_TYPE_FILE)
    {
        if (len < 3)
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: len %d < 3\n",len);
            return ERR_FAILURE;
        }

        file_name_length = *data++;
        if (len < 3 + file_name_length)
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: len %d < %d\n",len,3 + file_name_length);
            return ERR_FAILURE;
        }

        tp->file_name_length = file_name_length;
        MEMCPY(tp->file_name, data, file_name_length);
        APPL_PRINTF("ci_appmmi_file_acknowledge: FileNameByte %d:\n",file_name_length);
        for (i=0; i<file_name_length; i++)
        {
            //FileNameByte
            APPL_PRINTF("%c",data[i]);
        }
        APPL_PRINTF("\n");
        data += file_name_length;

        if (len < 3 + file_name_length + 4)
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: len %d < %d\n",len,3 + file_name_length + 4);
            return ERR_FAILURE;
        }

        file_data_length = uimsbf32(data);
        data += 4;
        if (file_data_length > CI_MAX_FILE_DATA_LEN)
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: FileDataLength %d > %d\n",file_data_length,CI_MAX_FILE_DATA_LEN);
            return ERR_FAILURE;
        }

        if (len < (int)(3 + file_name_length + 4 + file_data_length))
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: len %d < %d\n",len,3 + file_name_length + 4 + file_data_length);
            return ERR_FAILURE;
        }

        tp->data_length = file_data_length;
        MEMCPY(tp->data, data, tp->data_length);
        APPL_PRINTF("ci_appmmi_file_acknowledge: FileDataByte %d:\n",file_data_length);
        for (i=0; i<(int)file_data_length; i++)
        {
            if (i >= 256)
            {
                APPL_PRINTF("...");
                break;
            }
            //FileDataByte
            APPL_PRINTF("%02X ",data[i]);
        }
         if (file_ok)
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CI_FILE_OK);
        APPL_PRINTF("\n");
        data += file_data_length;
    }
    else if (request_type == CI_APPMMI_REQ_TYPE_DATA)
    {
        if (len-2 > CI_MAX_FILE_DATA_LEN)
        {
            APPL_PRINTF("ci_appmmi_file_acknowledge: DataLength %d > %d\n",len-2,CI_MAX_FILE_DATA_LEN);
            return ERR_FAILURE;
        }

        tp->data_length = len - 2;
        MEMCPY(tp->data, data, tp->data_length);

        APPL_PRINTF("ci_appmmi_file_acknowledge: DataByte %d:\n",len-2);
        for (i=0; i<(len-2); i++)
        {
            if (i >= 256)
            {
                APPL_PRINTF("...");
                break;
            }
            //DataByte
            APPL_PRINTF("%02X ",data[i]);
        }
        APPL_PRINTF("\n");
        data += tp->data_length;
    }

    return SUCCESS;
}

//buffer: OUT, FileNameLength[1] + FileName[n] + FileDataLength[4] + FileData[n]
INT32 ci_appmmi_get_file_data(void *buffer, int len, struct ci_senssion_connect *sc)
{
    struct ci_appmmi *tp;
    UINT8 *data = buffer;

    //should set zero all
    MEMSET(data, 0, 5);

    if (sc->appdata == NULL)
    {
        return ERR_FAILURE;
    }
    tp = (struct ci_appmmi*)(sc->appdata);

    if (len < (int)(tp->file_name_length + tp->data_length + 5))
    {
        APPL_PRINTF("ci_appmmi_get_file_data: buffer len %d < %d\n",len,tp->file_name_length + tp->data_length + 5);
        return ERR_FAILURE;
    }

    //file name length
    data[0] = tp->file_name_length;
    //file name
    MEMCPY(data+1, tp->file_name, tp->file_name_length);
    //file data length
    data[tp->file_name_length + 1] = tp->data_length>>24;
    data[tp->file_name_length + 2] = (tp->data_length>>16) & 0xFF;
    data[tp->file_name_length + 3] = (tp->data_length>>8) & 0xFF;
    data[tp->file_name_length + 4] = tp->data_length & 0xFF;
    //file data
    MEMCPY(data+1+tp->file_name_length+4, tp->data, tp->data_length);

    return SUCCESS;
}

static INT32 ci_appmmi_abort_request(int len, UINT8 *data, int slot)
{
    UINT8 abort_req_code;
    int i;

    abort_req_code = *data;

    APPL_PRINTF("ci_appmmi_abort_request: AbortReqCode %d:\n",len);
    for (i = 0; i < len; i++)
    {
        //AbortReqCode 8 bslbf
        APPL_PRINTF("%02X ",data[i]);
    }
    APPL_PRINTF("\n");
    data += len;

    //TO DO...
    //send msg to AP
    //how about AbortReqCode?
    api_ci_msg_to_app((abort_req_code<<8)|slot, API_MSG_CI_APP_ABORT);

    return SUCCESS;
}

//AbortReqCode: IN,
//0x01 User Cancel 每 The user has initiated termination of the application domain.
//0x02 System Cancel 每 The system has pre-empted the application domain to perform another task.
INT32 ci_appmmi_send_abort_request(struct ci_senssion_connect *sc, UINT8 abort_req_code)
{
    APPL_PRINTF("ci_appmmi_send_abort_request: send AbortReqCode 0x%02X\n",abort_req_code);
    if (SUCCESS != ci_senss_send_apdu(sc, APPOBJ_TAG_APP_ABORT_REQUEST, 1, &abort_req_code))
    {
        APPL_PRINTF("ci_appmmi_send_abort_request: Send APPOBJ_TAG_APP_ABORT_REQUEST failure\n");
        return ERR_FAILURE;
    }

    return SUCCESS;
}

static INT32 ci_appmmi_abort_ack(int len, UINT8 *data)
{
    int i;

    APPL_PRINTF("ci_appmmi_abort_ack: AbortAckCode %d:\n",len);
    for (i = 0; i < len; i++)
    {
        //AbortAckCode 8 bslbf
        APPL_PRINTF("%02X ",data[i]);
    }
    APPL_PRINTF("\n");
    data += len;

    //TO DO...

    return SUCCESS;
}

INT32 ci_appmmi_send_abort_ack(struct ci_senssion_connect *sc, UINT8 abort_ack_code)
{
    APPL_PRINTF("ci_appmmi_send_abort_ack: send AbortReqCode 0x%02X\n",abort_ack_code);
    if (SUCCESS != ci_senss_send_apdu(sc, APPOBJ_TAG_APP_ABORT_ACK, 1, &abort_ack_code))
    {
        APPL_PRINTF("ci_appmmi_send_abort_ack: Send APPOBJ_TAG_APP_ABORT_ACK failure\n");
        return ERR_FAILURE;
    }

    return SUCCESS;
}

static INT32 ci_appmmi_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    int tag;
    int l;
    UINT8 *d;

    APPL_PRINTF("ci_appmmi_process: data %X, len %d\n", data, len);

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_REQUEST_START:
            APPL_PRINTF("ci_appmmi_process: %d: <== request start\n", sc->scnb);
            //FileRequest or RequestStartAck
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                ci_appmmi_request_start(sc, l, d);
            }
            else
            {
                APPL_PRINTF("ci_appmmi_process: bad apdu of RequestStart\n");
            }
            /* Remove timer if CAM feedback */
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, CI_TIMER_PARAM_ANY, (void *)(sc->tc->slot)));

            //send msg to AP
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CI_REQUEST_START);

            break;
        case APPOBJ_TAG_FILE_ACKNOWLEDGE:
            APPL_PRINTF("ci_appmmi_process: %d: <== file acknowledge\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                ci_appmmi_file_acknowledge(sc, l, d);
            }
            else
            {
                APPL_PRINTF("ci_appmmi_process: bad apdu of FileAcknowledge\n");
            }

            //send msg to AP
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CI_FILE_RECEIVED);

            break;
        case APPOBJ_TAG_APP_ABORT_REQUEST:
            APPL_PRINTF("ci_appmmi_process: %d: <== app abort request\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                ci_appmmi_abort_request(l, d, sc->tc->slot);
            }
            else
            {
                APPL_PRINTF("ci_appmmi_process: bad apdu of AppAbortRequest\n");
            }

            break;
        case APPOBJ_TAG_APP_ABORT_ACK:
            APPL_PRINTF("ci_appmmi_process: %d: <== app abort ack\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                ci_appmmi_abort_ack(l, d);
            }
            else
            {
                APPL_PRINTF("ci_appmmi_process: bad apdu of AppAbortAck\n");
            }

            break;
        default:
            APPL_PRINTF("ci_appmmi_process: unknown tag %06X\n", tag);
            //return ERR_FAILURE;
            break;
        }
    }
    return SUCCESS;
}

/* CI+: Content Control Resource */
static void ci_cc_delete(struct ci_senssion_connect *sc);
static INT32 ci_cc_process(struct ci_senssion_connect *sc, int len, UINT8 *data);
static void uri_message(UINT8 *data, int len, struct ci_uri_message *uri_msg);
void ci_uri_monitor(struct ci_senssion_connect *sc, UINT32 program_number);

void set_uri_default(struct ci_uri_message *uri_msg)
{
    //ch 5.7.4
    MEMSET(uri_msg, 0, sizeof(struct ci_uri_message));
    uri_msg->protocol_version = URI_DEFAULT;
    uri_msg->emi_copy_control_info = EMI_DEFAULT;
}

extern RET_CODE write_akh_dhsk(UINT8 *buf);
extern void ci_notify_uri(int slot, struct ci_uri_message *uri_msg);
extern RET_CODE ci_get_aes_seed(UINT8 *aes_seed);
extern RET_CODE ci_get_hdr(UINT32 type, CIPLUS_KEY_HDR *hdr);

void ci_cc_dev_init(UINT8 slot)
{
    struct ci_cc_device *dev;

    dev = ci_cc_get_dev(slot);

    dev->akh_dhsk_writer = write_akh_dhsk;
    dev->ci_del_timer = ci_del_timer;
    dev->ci_notify_uri = ci_notify_uri;
    dev->ci_set_timer = ci_set_timer;
    dev->ci_uri_monitor = ci_uri_monitor;
    dev->dsc_close = ciplus_dsc_close;
    dev->dsc_start = ciplus_dsc_start;
    dev->dsc_stop = ciplus_dsc_stop;
    dev->get_aes_seed = ci_get_aes_seed;
    dev->uri_message = uri_message;
    dev->srm_parser = api_ciplus_parse_apply_srm;
    dev->get_hdr = ci_get_hdr;
    dev->ci_f_cc = ci_f_cc;
    dev->ci_f_sac = ci_f_sac;
    dev->ce_key_generate = ce_key_generate;
    dev->tdt_status = is_time_inited;
    dev->get_stream_time = get_utc;
}

struct ci_senssion_connect* ci_cc_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_cc_device *dev;
    struct ci_uri_message *uri_msg;

    ci_senss_conn_init(sc, scnb, RSC_ID_CONTENT_CONTROL, tc);
    sc->appdelete = ci_cc_delete;
    sc->callback = ci_cc_process;

    // The CC device should be registered when boot
    dev = ci_cc_attach(sc->tc->slot, ci_cc_dev_init);
    ci_cc_set_cur_slot(dev->priv, sc->tc->slot);

    if (NULL == dev)
    {
        APPL_PRINTF("%s, CC device attach failure.\n", __FUNCTION__);
        return NULL;
    }

    dev->sc = (void *)sc;

    if ((sc->appdata = ci_cc_get_data(dev)) == NULL)
    {
        APPL_PRINTF("ci_cc_create: get CC data faile\n");
        return NULL;
    }

    if (NULL == (uri_msg = ci_cc_get_uri_buf(sc->appdata, sizeof(struct ci_uri_message))))
    {
        APPL_PRINTF("ci_cc_create: get CC uri buffer faile\n");
        return NULL;
    }
    set_uri_default(uri_msg);

    //set CICAM-type
    sc->tc->link->slot[sc->tc->slot].cam_type = CICAM_CI_PLUS;
    //re-set AKH read index
    sc->tc->link->slot[sc->tc->slot].akh_idx = 0x00;

    api_ci_msg_to_app(sc->tc->slot, API_MSG_CIPLUS_CC_CREATE);

    return sc;
}

static void ci_cc_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    ciplus_dsc_stop(sc->tc->slot);
    ci_cc_dettach(sc->tc->slot);
    sc->appdata = NULL;
}

void ci_notify_uri(int slot, struct ci_uri_message *uri_msg);
extern struct UIR_DATA uri_data_bak;

static INT32 ci_cc_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    int tag;
    int l;
    UINT8 *d;
    UINT8 cc_system_id_bitmask = CC_SYSTEM_ID_BITMASK;
    UINT8 status_field;
    void *data_cnf = NULL;// = &tmp_data_cnf;

    if (sc && sc->appdata)
        data_cnf = ci_cc_get_cnf_data(sc->appdata);

//    APPL_PRINTF("ci_cc_process: data %X, len %d\n", data, len);

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_CC_OPEN_REQ:
            APPL_PRINTF("ci_cc_process: %d: <== open req\n", sc->scnb);

            APPL_PRINTF("ci_cc_process: %d: ==> open cnf\n", sc->scnb);
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_CC_OPEN_CNF, 1, &cc_system_id_bitmask) != SUCCESS)
            {
                APPL_PRINTF("ci_cc_process: Send APPOBJ_TAG_CC_OPEN_CNF failue\n");
                return ERR_FAILURE;
            }

            break;
        case APPOBJ_TAG_CC_DATA_REQ:
            APPL_PRINTF("ci_cc_process: %d: <== data req\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                if (data_cnf && SUCCESS == cc_data_req(sc->appdata, l, d, data_cnf))
                {
                    UINT32 len = 256;
                    UINT8 *data;

                    APPL_PRINTF("ci_cc_process: %d: ==> data cnf\n", sc->scnb);
                    data = cc_data_cnf(sc->appdata, data_cnf, &len);

                    //HOST -> CICAM
                    if (data && (ci_senss_send_apdu(sc, APPOBJ_TAG_CC_DATA_CNF, len, data) != SUCCESS))
                    {
                        APPL_PRINTF("cc_data_cnf: Send APPOBJ_TAG_CC_DATA_CNF failure\n");
                        return ERR_FAILURE;
                    }
                }
            }
            else
            {
                APPL_PRINTF("ci_cc_process: bad apdu of CC Data Req\n");
            }

            break;
        case APPOBJ_TAG_CC_SYNC_REQ:
            APPL_PRINTF("ci_cc_process: %d: <== sync req\n", sc->scnb);

            //Table 11.17
            //00: OK
            //01: No CC Support
            //02: Host Busy
            //03: Authentication failed
            if (sc && sc->appdata)
                status_field = ci_cc_get_status_field(sc->appdata);
            else
                status_field = STATUS_NO_CC_SUPPORT;

            APPL_PRINTF("ci_cc_process: %d: ==> sync cnf: status_field %d\n", sc->scnb,status_field);
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_CC_SYNC_CNF, 1, &status_field) != SUCCESS)
            {
                APPL_PRINTF("ci_cc_process: Send APPOBJ_TAG_CC_SYNC_CNF failue\n");
                return ERR_FAILURE;
            }

            break;
        case APPOBJ_TAG_CC_SAC_DATA_REQ:
            APPL_PRINTF("ci_cc_process: %d: <== sac data req\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                if (data_cnf && SUCCESS == cc_sac_data_req(sc->appdata, l, d, data_cnf))
                {
                    UINT8 *sac_message = NULL;
                    UINT32 sac_len = 256;
                    sac_message = cc_sac_data_cnf(sc->appdata, data_cnf, &sac_len);
                    APPL_PRINTF("ci_cc_process: %d: ==> sac data cnf\n", sc->scnb);

                    //sac_message: HOST -> CICAM
                    if (sac_message && (ci_senss_send_apdu(sc, APPOBJ_TAG_CC_SAC_DATA_CNF, sac_len, sac_message) != SUCCESS))
                    {
                        if (sac_message)
                        {
                            FREE(sac_message);
                        }
                        APPL_PRINTF("cc_sac_data_cnf: Send APPOBJ_TAG_CC_SAC_DATA_CNF failure\n");
                        return ERR_FAILURE;
                    }

                    if (sac_message)
                    {
                        FREE(sac_message);
                    }
                }
            }
            else
            {
                APPL_PRINTF("ci_cc_process: bad apdu of CC SAC DATA REQ\n");
            }

            break;
        case APPOBJ_TAG_CC_SAC_SYNC_REQ:
            APPL_PRINTF("ci_cc_process: %d: <== sac sync req\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l >0 && d != NULL)
            {
                UINT8 *sac_message = NULL;
                UINT32 sac_len = 256;
                cc_sac_sync_req(sc->appdata, l, d);

                APPL_PRINTF("ci_cc_process: %d: ==> sac sync cnf\n", sc->scnb);

                sac_message = cc_sac_sync_cnf(sc->appdata, &sac_len);

                //sac_message: HOST -> CICAM
                if (sac_message && (ci_senss_send_apdu(sc, APPOBJ_TAG_CC_SAC_SYNC_CNF, sac_len, sac_message) != SUCCESS))
                {
                    APPL_PRINTF("cc_data_cnf: Send APPOBJ_TAG_CC_SAC_SYNC_CNF failure\n");
                    return ERR_FAILURE;
                }
            }
            else
            {
                APPL_PRINTF("ci_cc_process: bad apdu of CC SAC SYNC REQ\n");
            }

            break;
        default:
            APPL_PRINTF("ci_cc_process: unknown tag %06X\n", tag);
            //return ERR_FAILURE;
            break;
        }
    }

    return SUCCESS;
}

//URI message
static void uri_message(UINT8 *data, int len, struct ci_uri_message *uri_msg)
{
    APPL_PRINTF("uri_message: %d\n", len);
    if (len < 8)
    {
        APPL_PRINTF("uri_message: length %d < 8\n", len);
        return;
    }

    //0x01 default version URI_DEFAULT
    uri_msg->protocol_version = data[0];
    APPL_PRINTF("uri_message: protocol_version 0x%X\n", uri_msg->protocol_version);
    if (uri_msg->protocol_version != URI_DEFAULT)
    {
        APPL_PRINTF("uri_message: protocol_version 0x%X not supported!\n", uri_msg->protocol_version);

        /* Future URI versions shall not override
           existing bits in default URI version 0x01.
         */
    }

    //0x0 00 Copy Protection Encoding Off
    //0x1 01 AGC Process On, Split Burst Off
    //0x2 10 AGC Process On, 2 line Split Burst On
    //0x3 11 AGC Process On, 4 line Split Burst On
    uri_msg->aps_copy_control_info = data[1] >> 6;    //2 uimsbf
    APPL_PRINTF("uri_message: aps_copy_control_info 0x%X\n", uri_msg->aps_copy_control_info);
    //TO DO...
    //Analogue output protection

    //0x0 00 Copying not restricted
    //0x1 01 No further copying is permitted
    //0x2 10 One generation copy is permitted
    //0x3 11 Copying is prohibited
    uri_msg->emi_copy_control_info = (data[1] >> 4) & 0x03;    //2 uimsbf
    APPL_PRINTF("uri_message: emi_copy_control_info 0x%X\n", uri_msg->emi_copy_control_info);
    //TO DO...
    //copy protection

    //0x0 0 No Image Constraint asserted
    //0x1 1 Image Constraint required
    uri_msg->ict_copy_control_info = (data[1] >> 3) & 0x01;    //1 uimsbf
    APPL_PRINTF("uri_message: ict_copy_control_info 0x%X\n", uri_msg->ict_copy_control_info);

    //0x0 0 No Redistribution Control asserted. Default.
    //0x1 1 Redistribution Control asserted
    uri_msg->rct_copy_control_info = (data[1] >> 2) & 0x01;    //1 uimsbf
    APPL_PRINTF("uri_message: rct_copy_control_info 0x%X\n", uri_msg->rct_copy_control_info);
// reserved for future use 4 uimsbf

    //0x00 000000 Default retention limit of 90 minutes applies
    //0x01 000001 Retention limit of 6 hours applies
    //0x02 000010 Retention limit of 12 hours applies
    //0x03..0x3F 000011-111111 Retention limit of 1-61 multiples of 24 Hrs applies as signalled by bits
    uri_msg->rl_copy_control_info = data[2] & 0x3F;    //6 uimsbf
    APPL_PRINTF("uri_message: rl_copy_control_info 0x%X\n", uri_msg->rl_copy_control_info);
    //TO DO...
    //PVR record & tms retention limit

    MEMCPY(&uri_data_bak.uri_msg, uri_msg, sizeof(struct ci_uri_message));
    uri_data_bak.state = URI_RECEIVED;

// reserved for future use 40 uimsbf
}

void ci_uri_monitor(struct ci_senssion_connect *sc, UINT32 program_number)
{
    struct ci_uri_message *uri_msg;

    if (sc->appdata == NULL)
    {
        return;
    }
    uri_msg = ci_cc_get_uri_buf(sc->appdata, sizeof(struct ci_uri_message));

    //set URI final default
    uri_msg->ict_copy_control_info = ICT_FINAL_DEFAULT;
    uri_msg->program_number = program_number;

    APPL_PRINTF("%s: Timer set URI data!\n", __FUNCTION__);

    //api_ci_msg_to_app(sc->tc->slot, API_MSG_URI_RECEIVED); /* Notify App of URI Message received */
    ci_notify_uri(sc->tc->slot, uri_msg);
}

#endif    //CI_PLUS_SUPPORT

