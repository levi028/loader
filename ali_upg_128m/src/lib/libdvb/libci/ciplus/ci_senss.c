/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_senss.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack - senssion layer.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Nov.15.2004       Justin Wu      Ver 0.1     Create file.
*    2.    Feb.23.2010       Steven         Ver 0.2     Add new resources for CI+
*
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include "ci_stack.h"

extern void ci_cas_monitor(struct ci_senssion_connect *sc, int *times);

static INT32 ci_senss_process(struct ci_senssion_layer *senss, void *dummy);
static INT32 ci_resourceid_to_int(UINT8 *data);
static struct ci_senssion_connect *ci_get_senss_by_scnb(struct ci_senssion_layer *senss, int scnb);
static struct ci_senssion_connect * ci_senss_create(struct ci_senssion_layer *senss, int rscid);
//static INT32 ci_senss_create_conn_requ(struct ci_senssion_layer *senss, int rscid);
static INT32 ci_senss_open_conn_resp(struct ci_senssion_layer *senss, int len, UINT8 *data);
INT32 ci_senss_close_conn_requ(struct ci_senssion_connect *sc);
static INT32 ci_senss_close_conn_resp(struct ci_senssion_connect *sc);
//static int ci_senss_close_all_conn(struct ci_senssion_layer *senss, int slot);
static int ci_senss_close_all_conn_d(struct ci_senssion_layer *senss, int slot);
static INT32 ci_senss_send_spdu(struct ci_senssion_connect *sc, UINT8 tag, int rscid, char status);

/* Initialize Session Connection */
void ci_senss_conn_init(struct ci_senssion_connect *sc, int scnb,
                        int rscid, struct ci_transport_connect *tc)
{
    sc->scnb = scnb;
    sc->rscid = rscid;
    sc->tc = tc;
}

/* Session Layer Attach */
INT32 ci_senss_attach(struct ci_senssion_layer *senss)
{
    int i;

    senss->tc = NULL;

    // Initialize All Session Connections
    for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
    {
        ci_senss_conn_init(&(senss->connect[i]), 0, 0, NULL);
        senss->connect[i].callback = NULL;
    }
    /* Create Session Layer Process Cycle */
    ci_set_timer(CI_DURATION_SENSS_PROCESS, (ci_timer_handler)ci_senss_process, (void *)(senss), NULL);

    // Transmission Layer Attach
    return ci_trans_attach(&(senss->trans));
}

/* Session Layer Detach */
INT32 ci_senss_detach(struct ci_senssion_layer *senss)
{
    int i;

    // Close All Session Connections
    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        //fix BUG08526, When Standby, Close All Session Connections Directly
        ci_senss_close_all_conn_d(senss, i);
    }

    // Transmission Layer Detach
    return ci_trans_detach(&(senss->trans));
}

/* Send SPDU in Session Connection (Only Session Header, Not Including Body which Contains APDU Data) */
static INT32 ci_senss_send_spdu(struct ci_senssion_connect *sc, UINT8 tag, int rscid, char status)
{
    UINT8 buffer[16];
    UINT8 *p = buffer;

    *p++ = tag;                        /* SPDU Tag */
    *p++ = 0x00;                        /* Later will contain length */
    if (status >= 0)                    /* Session Connection Status */
    {
        *p++ = status;
    }
    if (rscid)                            /* Resource ID */
    {
        *p++ = ((rscid >> 24) & 0xff);
        *p++ = ((rscid >> 16) & 0xff);
        *p++ = ((rscid >> 8) & 0xff);
        *p++ = ( rscid & 0xff);
    }
    *p++ = ((sc->scnb >> 8) & 0xff);    /* Session Connection Number */
    *p++ = ( sc->scnb & 0xff);
    buffer[1] = p - buffer - 2;            /* Length */

    // Send SPDU in Transmission Connection
    if (sc->tc)
    {
        return ci_trans_send_spdu(sc->tc, p - buffer, buffer);
    }

    return ERR_FAILUE;
}

/* MMI WatchDog Function */
void ci_mmi_watchdog(struct ci_senssion_connect *sc, int slot)
{
    // Delete Last MMI WatchDog Timer Function
    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, (void *)sc, (void *)slot));

    // After Select Menu, If No Response From CAM Some Time Later, then Cancel Menu
    if ((INT32)sc == (-2))
    {
        //for Digisat3 Platinum card, BUG09188
        api_ci_msg_to_stack(slot, API_MSG_CANCEL_MENU);
    }
    //
    else
    {
        if (sc == NULL)
        {
            // Send Enter Menu to CAM, If CAM not Feedback after 6s, then Cancel Resend, Exit Menu
            while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(slot), (void *)API_MSG_ENTER_MENU));
        }
        else
        {
            ci_senss_close_conn_requ(sc);
        }
        api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
    }
}

extern int ci_trans_close_all_conn_d(struct ci_transport_layer *trans, int slot);
extern void ci_cas_cp_pmt(struct ci_ca_pmt* l_ca_pmt, int slot);
/* Process for Received Message in CI Stack */
void ci_message_process(struct ci_senssion_layer *senss, struct api_ci_msg *msg)
{
    struct ci_senssion_connect *sc =  NULL;
    UINT32 slot = 0;
    struct ci_mmi *mmi = NULL;
    UINT8 cmd_id = CI_CPCI_QUERY;
    //enum api_ci_msg_type msg_type;

    /* Process Stack Message */
    if (msg)
    {
        SENS_PRINTF("Receive Msg: 0x%x\n", msg->type);
        slot = msg->slot;
        switch (msg->type)
        {
        case API_MSG_CAMHW:    /* CAM HW Active */
            // Detect CAM in Link Layer
            ci_link_cam_detect(&(senss->trans.link), slot);
            break;
        case API_MSG_CAMIN:        /* CAM Plug In */
            // Link Layer Attach
            ci_link_cam_attach(&(senss->trans.link), slot);
            break;
        case API_MSG_NEGO:        /* CAM Negotiation */
            // Negotiate Buffer Size
            ci_link_negotiation(&(senss->trans.link), slot);
            break;
        case API_MSG_CTC:        /* CAM Attached, Create Init TC */
            // Open Transmission Connection
            if (NULL == ci_trans_open_conn(&(senss->trans), slot))    /* Open Transmission Connection Failure */
            {
                // Should Del Polling Timer
                while (SUCCESS == ci_del_timer((ci_timer_handler)ci_senss_process, (void *)senss, NULL));

                SENS_PRINTF("ci_message_process: Create transport connect failue\n");
                senss->trans.link.slot[slot].state = CAM_STACK_DETACH;
                api_ci_msg_to_stack(slot, API_MSG_CAMOUT);
                api_ci_msg_to_app(slot, API_MSG_CAMOUT);
            }
            else                                                    /* Open Transmission Connection Success */
            {
#ifdef CI_PLUS_SUPPORT
                /* Some CIPLUS CAM need set ts pass CAM because it need get current TIME
                 * from TS, not using date_time resource, so here set TS pass CAM when CAM insert.
                 * BUT: It will cause video freeze 2~3 seconds after CI insert, so here just enable AT CIPLUS project */
                ci_link_cam_pass(&(senss->trans.link), slot, TRUE);
#endif
                while (SUCCESS == ci_del_timer((ci_timer_handler)ci_senss_process, (void *)senss, NULL));
                ci_set_timer(CI_DURATION_SENSS_PROCESS, (ci_timer_handler)ci_senss_process, (void *)senss, NULL);
            }
            break;
        case API_MSG_CAMOUT:    /* CAM Pull Out */
            // Link Layer Detach
            if (ci_link_cam_detach(&(senss->trans.link), slot) == SUCCESS)
            {
                // Notify Stack of Delete Transmission Connection
                api_ci_msg_to_stack(slot, API_MSG_DTC);
            }

            ci_set_cam_name(NULL, 0, slot);
/*
#if ((!defined(CC_USE_TSG_PLAYER))&&(!defined(CI_SLOT_DYNAMIC_DETECT)))
            api_ci_init_pmtcrc(slot);
#endif
*/
#ifdef CI_PLUS_SUPPORT
            uri_handle_after_cam_out(slot);
            if (CICAM_CI_PLUS == api_ci_get_cam_type(slot))
                ci_clear_uri();

            api_ci_init_cam_type(slot);
            api_ci_camup_answer_setstatus(TRUE);
            api_ci_camup_init_progress();
#endif
            break;
        case API_MSG_DTC:        /* CAM Deteched, Delete all SC/TC */
            if (senss->trans.link.slot[slot].state == CAM_STACK_DETACH)
            {
                // When CAM Pull Out, shall Del All Timers of ci_cas_send_PMT of This Slot
                if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONDITIONAL_ACCESS_SUPPORT, slot)) != NULL)
                {
                    SENS_PRINTF("ci_message_process: del all ca_pmt timers of slot %d!\n",slot);
                    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
                    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
                }
                // Close All Session Connection Directly
                ci_senss_close_all_conn_d(senss, slot);
                // Close All Transmission Connection Directly
                ci_trans_close_all_conn_d(&(senss->trans), slot);
                // Notify Stack of  CAM Detach to Detect CAM Again and Notify App of Delete Transmission Connection Already
                senss->trans.link.slot[slot].state = CAM_DETACH;
                api_ci_msg_to_stack(slot, API_MSG_CAMHW);
                api_ci_msg_to_app(slot, API_MSG_DTC);
            }
            break;
        case API_MSG_GET_APTYPE:            /* Get Application Type */
            SENS_PRINTF("ci_message_process: AI <== Get Application Type\n");
            *(UINT8 *)(msg->param) = 0;
            // Get Application Information Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, senss->trans.link.slot[slot].app_info_type, slot)) != NULL)
            {
                struct ci_application_information *tp;

                tp = (struct ci_application_information *)(sc->appdata);
                if (tp->state == 2)
                {
                    // Get Application Type
                    *(UINT8 *)(msg->param) = ci_ai_get_application_type(sc);
                }
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_APTYPE);
            break;
#ifdef CI_PLUS_SUPPORT
        case API_MSG_GET_CAMBID:            /* Get CICAM Brand ID */
            SENS_PRINTF("ci_message_process: CC ==> Get CICAM Brand ID\n");
            *(UINT16 *)(msg->param) = 0;
            // Get Content Control Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONTENT_CONTROL, slot)) != NULL)
            {
                // We may shunning the service at boot when we can't
                // get CAM_ID in time, so we need set a timer for this case
                extern UINT8 ciplus_service_shunning_timeout;
                ci_del_timer((ci_timer_handler)api_ci_msg_to_app, (void*)slot,
                            (void*)API_MSG_SERVICE_SHUNNING_UPDATE);
                if (ci_cc_get_cur_state(sc->appdata) == 2)
                {
                    *(UINT16 *)(msg->param) = ci_cc_get_cicam_brand_id(sc->appdata);
                    ciplus_service_shunning_timeout = 0;
                }
                else
                {
                    ciplus_service_shunning_timeout++;
                    // We think about 3 minutes to be a upper limit
                    // How about we have new CICAM BRAND ID available after 3 mins ?
                    // It may be suitable to notify at ci_auth.c, when we have
                    // new right CICAM BRAND ID received, but that modification
                    // may have some cross impaction. And, 3 mins should be a
                    // reasonable time out for normal CAM authentication ---- Owen
                    if (ciplus_service_shunning_timeout < 180)
                    {
                        ci_set_timer(1000, (ci_timer_handler)api_ci_msg_to_app,
                                    (void*)slot, (void*)API_MSG_SERVICE_SHUNNING_UPDATE);
                    }
                    else
                    {
                        ciplus_service_shunning_timeout = 0;
                    }
                }
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_CAMBID);
            break;
#endif
        case API_MSG_GET_APMANUFACTURER:    /* Get Application Manufacturer */
            SENS_PRINTF("ci_message_process: AI <== Get Application Manufacturer\n");
            *(UINT16 *)(msg->param) = 0;
            // Get Application Information Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, senss->trans.link.slot[slot].app_info_type, slot)) != NULL)
            {
                struct ci_application_information *tp;

                tp = (struct ci_application_information *)(sc->appdata);
                if (tp->state == 2)
                {
                    // Get Application Manufacturer
                    *(UINT16 *)(msg->param) = ci_ai_get_application_manufacturer(sc);
                }
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_APMANUFACTURER);
            break;
        case API_MSG_GET_MENUSTRING:        /* Get Application Menu String */
            SENS_PRINTF("ci_message_process: AI <== Get Menu\n");
            // Get Application Information Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, senss->trans.link.slot[slot].app_info_type, slot)) != NULL)
            {
                struct ci_application_information *tp;
                void *buffer;
                int len;

                tp = (struct ci_application_information*)(sc->appdata);
                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);
                if (tp->state == 2)
                {
                    // Get Application Menu String
                    *(char**)(msg->param) = ci_ai_get_menu_string(buffer, len, sc);
                }
                else
                {
                    *(int*)(buffer) = 0;
                }
            }
            else
            {
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_MENUSTRING);
            break;
        case API_MSG_GET_CASIDS:            /* Get CA System ID */
            SENS_PRINTF("ci_message_process: CAS <== Get CA System ID\n");
            // Get Conditional Access Support Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONDITIONAL_ACCESS_SUPPORT, slot)) != NULL)
            {
                struct ci_conditional_access *tp;
                void *buffer;
                int len;

                tp = (struct ci_conditional_access*)(sc->appdata);
                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);
                if (tp->state == 2 && (int)((CI_MAX_CAS_NUM + 1) * sizeof(unsigned short)) < len)
                {
                    //Get CA System ID
                    MEMCPY(buffer, ci_cas_get_ids(sc), ((CI_MAX_CAS_NUM + 1) * sizeof(unsigned short)));
                }
                else
                {
                    *(int*)(buffer) = 0;
                }
            }
            else
            {
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_CASIDS);
            break;
        case API_MSG_CA_PMT:                /* Send CA PMT to CAM */
            SENS_PRINTF("ci_message_process: ==> Send CA PMT to CAM\n");
            // Delete Last API_MSG_CA_PMT Message which will be sended to Stack
		while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(slot), (void *)API_MSG_CA_PMT));

            //Memory Copy the Received ca_pmt to local ca_pmt even if the CAM is not ready!
            ci_cas_cp_pmt((struct ci_ca_pmt *)msg->param, slot);

            // Get Conditional Access Support Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONDITIONAL_ACCESS_SUPPORT, slot)) != NULL)
            {
                if (msg->param != 0)
                {
                    cmd_id = ((struct ci_ca_pmt*)(msg->param))->cmd_id;    /* ca_pmt_cmd_id */
                    //cmd_id = CI_CPCI_OK_DESCRAMBLING;
                }

                // Send CA PMT to CAM
                if (ci_cas_send_pmt(sc, cmd_id) != SUCCESS)
                {
                    /* Do something here send CA PMT fail */
                    SENS_PRINTF("ci_message_process: Send CA PMT again!\n");
                }
                SENS_PRINTF("[%d]Send CI_CPCI_QUERY(cmd: %d) here! \n",slot,cmd_id);
            }
            else
            {
                SENS_PRINTF("ci_message_process: CAS     Session Connection is NULL!\n");
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_SENDCAPMT);
            break;
        case API_MSG_ENTER_MENU:            /* Enter Menu in AI */
            SENS_PRINTF("ci_message_process: AI ==> Enter Menu\n");
            // Delete Last API_MSG_ENTER_MENU Message which will be sended to Stack
			while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(slot), (void *)API_MSG_ENTER_MENU));

            // Get Application Information Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, senss->trans.link.slot[slot].app_info_type, slot)) != NULL)
            {
                // Send Enter Menu Request to CAM
                ci_ai_enter_menu(sc);
                // Set Timer Retry Send Enter Menu if CAM not Feedback
				ci_set_timer(CI_DURATION_MMI_RETRY, (ci_timer_handler)api_ci_msg_to_stack, (void *)slot, (void *)API_MSG_ENTER_MENU);
                // 6s Later, If CAM not Feedback, then Exit Menu
                ci_set_timer(CI_DURATION_MMI_ENTER, (ci_timer_handler)ci_mmi_watchdog, (void *)NULL, (void *)slot);
            }
            else
            {
                // Notify App of Exit Menu
                api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
            }
            break;
        case API_MSG_EXIT_MENU:                /* Exit Top Level Menu in MMI */
            SENS_PRINTF("ci_message_process: MMI ==> Exit Menu\n");
            // Delete Last API_MSG_ENTER_MENU Message which will be sended to Stack, that is, Cancel Enter Menu
			while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(slot), (void *)API_MSG_ENTER_MENU));

            // Get MMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL)
            {
                // Close MMI Session Connection
                ci_mmi_close_mmi(sc);
                ci_senss_close_conn_requ(sc);
            }
            break;
        case API_MSG_GET_MENU:                /* Get Menu Structure in MMI */
            SENS_PRINTF("ci_message_process: MMI <== Get Menu\n");
            // Get MMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL)
            {
                void *buffer;
                int len;

                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);                /* Length Stored in msg->param */
                // Get Menu Structure
                *(struct ci_menu**)(msg->param) = ci_mmi_get_menu(buffer, len, sc);
            }
            else
            {
                // If Error, then Return NULL
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_MENU);
            break;
        case API_MSG_GET_ENQUIRY:            /* Get Enquiry in MMI */
            SENS_PRINTF("ci_message_process: MMI <== Get Enquiry\n");
            // Get MMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL)
            {
                void *buffer;
                int len;

                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);                /* Length Stored in msg->param */
                // Get Enquiry Structure
                *(struct ci_enquiry**)(msg->param) = ci_mmi_get_enquiry(buffer, len, sc);
            }
            else
            {
                // If Error, then Return Null
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_ENQUIRY);
            break;
        case API_MSG_SELECT_MENU:            /* Select Current Level Menu in MMI */
            SENS_PRINTF("ci_message_process: MMI ==> Select Menu\n");
            // Get MMI Resource Session Connection and Check MMI Current Menu
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL &&
                (mmi = (struct ci_mmi*)(sc->appdata)) != NULL &&
                mmi->cur_menu >= 0 && mmi->menu_list[mmi->cur_menu] != NULL)
            {
                // Select Current Level Menu
                if (ci_menu_select(mmi->menu_list[mmi->cur_menu], msg->param) == SUCCESS)
                {
                    /* Set timer cancel MMI senss if CAM not feedback */
                    //for Digisat3 Platinum card, BUG09188
                    ci_set_timer(CI_DURATION_MMI_MONITOR, (ci_timer_handler)ci_mmi_watchdog, (void *)(-2), (void *)slot);
                }
                else
                {
                    // If Select Menu Fail, then Request Close MMI Session Connection and Exit Menu
                    ci_senss_close_conn_requ(sc);
                    api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
                }
            }
            else
            {
                // If Error, then Exit Menu
                api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
            }
            break;
        case API_MSG_CANCEL_MENU:            /* Cancel Current Level Menu in MMI */
            SENS_PRINTF("ci_message_process: MMI ==> Cancel Menu\n");
            // Delete Last API_MSG_ENTER_MENU Message which will be sended to Stack
			while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void*)(slot), (void*)API_MSG_ENTER_MENU));

            // Get MMI Resource Session Connection and Check MMI Current Menu
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL &&
                (mmi = (struct ci_mmi*)(sc->appdata)) != NULL &&
                mmi->cur_menu >= 0 && mmi->menu_list[mmi->cur_menu] != NULL)
            {
                // Cancel Current Level Menu
                if (ci_menu_cancel(mmi->menu_list[mmi->cur_menu]) == SUCCESS)
                {
                    // Return to Upper Level Menu
                    ci_mmi_pop_menu(mmi);
                    /* Set Timer Cancel MMI Session if CAM not feedback */
                    ci_set_timer(CI_DURATION_MMI_MONITOR, (ci_timer_handler)ci_mmi_watchdog, (void*)sc, (void*)slot);
                }
                else
                {
                    // If Cancel Menu Fail, then Request Close MMI Session Connection and Exit Menu
                    ci_senss_close_conn_requ(sc);
                    api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
                }
            }
            break;
        case API_MSG_REPLY_ENQUIRY:            /* Reply Enquiry in MMI */
            SENS_PRINTF("ci_message_process: MMI ==> Reply Enquiry\n");
            // Get MMI Resource Session Connection and Check MMI Current Enquiry
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL &&
                (mmi = (struct ci_mmi*)(sc->appdata)) != NULL && mmi->enquiry != NULL)
            {
                // Reply Enquiry
                if (ci_enquiry_reply(mmi->enquiry, (char*)msg->param) == SUCCESS)
                {
                    /* Set Timer Cancel MMI Session if CAM not feedback */
                    ci_set_timer(CI_DURATION_MMI_MONITOR, (ci_timer_handler)ci_mmi_watchdog, (void*)sc, (void*)slot);
                }
                else
                {
                    // If Reply Enquiry Fail, then Request Close MMI Session Connection and Exit Menu
                    ci_senss_close_conn_requ(sc);
                    api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
                }
            }
            else
            {
                // If Error, then Exit Menu
                api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_ENQUIRYRPLY);
            break;
        case API_MSG_CANCEL_ENQUIRY:        /* Cancel Enquiry in MMI */
            SENS_PRINTF("ci_message_process: MMI ==> Cancel Enquiry\n");
            // Delete Last API_MSG_ENTER_MENU Message which will be sended to Stack
			while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(slot), (void *)API_MSG_ENTER_MENU));

            // Get MMI Resource Session Connection and Check MMI Current Enquiry
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_MMI, slot)) != NULL &&
                (mmi = (struct ci_mmi *)(sc->appdata)) != NULL && mmi->enquiry != NULL)
            {
                // Cancel Enquiry
                if (ci_enquiry_cancel(mmi->enquiry) == SUCCESS)
                {
                    /* Set Timer Cancel MMI Session if CAM not feedback */
                    ci_set_timer(CI_DURATION_MMI_MONITOR, (ci_timer_handler)ci_mmi_watchdog, (void *)sc, (void *)slot);
                }
                else
                {
                    // If Cancel Enquiry Fail, then Request Close MMI Session Connection and Exit Menu
                    ci_senss_close_conn_requ(sc);
                    api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
                }
            }
            else
            {
                // If Error, then Exit Menu
                api_ci_msg_to_app(slot, API_MSG_EXIT_MENU);
            }
            break;
        case API_MSG_CI_NOT_SELECTED:        /* Cancel Enquiry in MMI */
            if (senss->trans.link.slot[slot].state == CAM_STACK_ATTACH)
            {
                // When Receive CI Not Selected Message, shall Del All Timers of ci_cas_send_PMT of This Slot
                if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONDITIONAL_ACCESS_SUPPORT, slot)) != NULL)
                {
                    SENS_PRINTF("ci_message_process: not select ci card in slot %d!\n", slot);
                    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
                    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
                }
                else
                {
                    SENS_PRINTF("ci_message_process: CAS     Session Connection is NULL in slot %d!\n", slot);
                }
            }
            break;
#ifdef CI_PLUS_SUPPORT
/* APP MMI */
        case API_MSG_GET_INITIAL_OBJECT:
            SENS_PRINTF("ci_message_process: APP MMI <== Get Initial Object\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                void *buffer;
                int len;

                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);                /* Length Stored in msg->param */
                ci_appmmi_get_initial_object(buffer, len, sc);
            }
            else
            {
                // If Error, then Return Null
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_INITIAL_OBJECT);
            break;
        case API_MSG_REQUEST_START_ACK:
            SENS_PRINTF("ci_message_process: APP MMI ==> Request Start Ack\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                ci_appmmi_request_start_ack(sc, msg->param);
            }
            break;
        case API_MSG_FILE_REQUEST:
            SENS_PRINTF("ci_message_process: APP MMI ==> File Request\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                struct ci_file_request *request = (struct ci_file_request*)(msg->param);

                if (request != NULL)
                    ci_appmmi_file_request(sc, request->requesttype, request->data, request->len);
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_FILE_REQUEST);
            break;
        case API_MSG_GET_FILE_DATA:
            SENS_PRINTF("ci_message_process: APP MMI <== Get File Data\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                void *buffer;
                int len;

                buffer = *(void**)(msg->param);
                len = *(int*)(buffer);                /* Length Stored in msg->param */
                ci_appmmi_get_file_data(buffer, len, sc);
            }
            else
            {
                // If Error, then Return Null
                *(int*)(*(void**)(msg->param)) = 0;
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_FILE_DATA);
            break;
        case API_MSG_ABORT_REQUEST:
            SENS_PRINTF("ci_message_process: APP MMI ==> Abort Request\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                ci_appmmi_send_abort_request(sc, msg->param);
            }
            break;
        case API_MSG_ABORT_ACK:
            SENS_PRINTF("ci_message_process: APP MMI ==> Abort Ack\n");
            // Get APPMMI Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_APPLICATION_MMI, slot)) != NULL)
            {
                ci_appmmi_send_abort_ack(sc, msg->param);
            }
            break;

        /*case API_MSG_GET_URI_MSG:
            SENS_PRINTF("ci_message_process: CC ==> Get URI msg\n");
            // Get CC Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONTENT_CONTROL, slot)) != NULL)
            {
                ci_cc_get_uri(sc, *(struct ci_uri_message **)(msg->param));
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_URI_MSG);
            break;
        case API_MSG_SET_URI_MSG:
            SENS_PRINTF("ci_message_process: CC ==> Set URI msg\n");
            // Get CC Resource Session Connection
            if ((sc = ci_get_senss_by_rscid(senss, RSC_ID_CONTENT_CONTROL, slot)) != NULL)
            {
                ci_cc_set_uri(sc, *(struct ci_uri_message **)(msg->param));
            }
            osal_flag_set(senss->trans.link.slot[slot].flag_id, API_FLAG_SET_URI_MSG);
            break;*/

#endif
        default:
            break;
        }
    }
}

/* Process for Session Connection */
static INT32 ci_senss_process(struct ci_senssion_layer *senss, void *dummy)
{
    struct ci_senssion_connect *sc = NULL;
    int slot = 0, len = 0, scnb = 0, rscid = 0;
    UINT8 *data = NULL;
    INT32 result = SUCCESS;
    INT32 i = 0;
    BOOL other_card_exist = FALSE;

    /* Process Session Connection */
    for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
    {
        // First, Process for Transmission Connection
        senss->tc = ci_trans_process(&(senss->trans), slot);

        if (senss->tc != NULL && senss->tc != (struct ci_transport_connect *)-1)
        {
            // Receive SPDU
            data = ci_trans_get_spdu(senss->tc, &len);

            if (data && len > 1)
            {
                switch (*data)
                {
                case SENSS_TAG_SESSION_NUMBER:            /* Receive and Process APDU from CAM */
                    if (len > 4)
                    {
                        // Get Session Connection
                        scnb = ntohs(&data[2]);
                        sc = ci_get_senss_by_scnb(senss, scnb);
                        if (sc != NULL)
                        {
                            // Callback Function Process APDU
                            if (sc->callback(sc, len - 4, data + 4) != SUCCESS)
                            {
                                SENS_PRINTF("ci_senss_process: application callback failue\n");
                                senss->tc = NULL;
                            }
                        }
                        else
                        {
                            SENS_PRINTF("ci_senss_process: Unknown session nb: %04x\n", scnb);
                        }
                    }
                    break;
                case SENSS_TAG_OPEN_SESSION_REQUEST:        /* Open Session Connection Request from CAM */
                    // Send Open Session Connection Response to CAM
                    rscid = ci_resourceid_to_int(data + 2);
                    if (ci_senss_open_conn_resp(senss, len, data) == SUCCESS)
                    {
                        // Match with "Wait a minute (about 500ms), if CAM Continue Request Open an MMI Session,
                        // then don't Exit Menu, Or Notify App of Exit Menu (Exit MMI Session)"
                        if (rscid == RSC_ID_MMI)
                        {
                            while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_app, (void*)slot, (void*)API_MSG_EXIT_MENU));
                        }
                        SENS_PRINTF("ci_senss_process: Create senss OK!\n");
                    }
                    else
                    {
                        SENS_PRINTF("ci_senss_process: Create senss failue!\n");
                        senss->tc = NULL;
                    }
                    break;
                case SENSS_TAG_CLOSE_SESSION_REQUEST:        /* Close Session Connection Request from CAM */
                    if (len == 4)
                    {
                        // Get Session Connection
                        scnb = ntohs(&data[2]);
                        sc = ci_get_senss_by_scnb(senss, scnb);
                        if (sc != NULL)
                        {
                            // Wait a minute (about 500ms), if CAM Continue Request Open an MMI Session, then don't Exit Menu,
                            // Or Notify App of Exit Menu (Exit MMI Session)
                            if (sc->rscid == RSC_ID_MMI)
                            {
                                // 090901, CI+ APPMMI start, will close MMI
                                ci_set_timer(CI_DURATION_MMI_GUARD, (ci_timer_handler)api_ci_msg_to_app, (void*)slot, (void*)API_MSG_EXIT_MENU);
                            }
                            // Send Close Session Connection Response to CAM
                            ci_senss_close_conn_resp(sc);
                        }
                    }
                    break;
                case SENSS_TAG_CREATE_SESSION_RESPONSE:    /* Create Session Connection Response from Resource Provider */
                    // If Create Session Connection to Resource Provider Fail, then Close Session Connection
                    if (data[2] != SENSS_STATUS_OK)
                    {
                        // Get Session Connection
                        scnb = ntohs(&data[7]);
                        sc = ci_get_senss_by_scnb(senss, scnb);
                        if (sc != NULL)
                        {
                            // Wait a minute (about 500ms), if CAM Continue Request Open an MMI Session, then don't Exit Menu,
                            // Or Notify App of Exit Menu (Exit MMI Session)
                            if (sc->rscid == RSC_ID_MMI)
                            {
                                ci_set_timer(CI_DURATION_MMI_GUARD, (ci_timer_handler)api_ci_msg_to_app, (void*)slot, (void*)API_MSG_EXIT_MENU);
                            }
                            // Send Close Session Connection Request to CAM
                            ci_senss_close_conn_requ(sc);
                        }
                        SENS_PRINTF("ci_senss_process: Create session failue %04x!\n", scnb);
                    }
                    break;
                case SENSS_TAG_CLOSE_SESSION_RESPONSE:    /* Close Session Connection Response from CAM */
                    scnb = ntohs(&data[3]);
                    if (data[2] != SENSS_STATUS_OK)
                    {
                        SENS_PRINTF("ci_senss_process: Delete session failue %04x!\n", scnb);
                    }
                    else
                    {
                        // Get Session Connection
                        if ((sc = ci_get_senss_by_scnb(senss, scnb)) != NULL)
                        {
                            // Delete Session Connection
                            if (sc->appdelete)
                            {
                                sc->appdelete(sc);
                            }
                            ci_senss_conn_init(sc, 0, 0, NULL);
                        }
                    }
                    break;
                default:
                    SENS_PRINTF("ci_senss_process: Unknown session tag: %02x\n", *data);
                }
            }
        }

        /* Error when communication */
        if (slot < CI_MAX_SLOT_NUM
            && (senss->trans.link.slot[slot].state == CAM_ATTACH || senss->trans.link.slot[slot].state == CAM_STACK_ATTACH)
            && senss->tc == NULL)
        {
            SENS_PRINTF("ci_senss_process: Trans Conneciton NULL --- CAM Out.\n");
            // Detach CAM and Notify Stack and App of CAM Pull Out
            senss->trans.link.slot[slot].state = CAM_STACK_DETACH;
            api_ci_msg_to_stack(slot, API_MSG_CAMOUT);
            api_ci_msg_to_app(slot, API_MSG_CAMOUT);
            result = ERR_FAILUE;

            // Check Other Card Exist Or Not
            for (i=0; i<CI_MAX_SLOT_NUM; i++)
            {
                if (i != slot && senss->trans.link.slot[i].state == CAM_STACK_ATTACH)
                {
                    other_card_exist = TRUE;
                    break;
                }
            }
        }
    }

    // Cyclic Process for Session Connection
    if (result == SUCCESS || other_card_exist)
    {
        ci_set_timer(CI_DURATION_SENSS_PROCESS, (ci_timer_handler)ci_senss_process, (void*)senss, NULL);
    }

    return result;
}

#if (CI_MAX_SPDU_SIZE > 1024)
static UINT8 senss_buffer[CI_MAX_SPDU_SIZE];
#endif
/* Send APDU in Session Connection (Including Session Header and Body which Contains APDU Data) */
INT32 ci_senss_send_apdu(struct ci_senssion_connect *sc, int tag, int len, UINT8 *data)
{
#if (CI_MAX_SPDU_SIZE > 1024)
    UINT8 *buffer = senss_buffer;    /* do not use the stack buffer */
#else
    UINT8 buffer[CI_MAX_SPDU_SIZE];
#endif
    UINT8 *p = buffer;

    *p++ = SENSS_TAG_SESSION_NUMBER;    /* Session Number Tag */
    *p++ = 0x02;                            /* Length */
    *p++ = (sc->scnb >> 8) & 0xFF;        /* Session Connection Number */
    *p++ =  sc->scnb & 0xFF;
    *p++ = (tag >> 16) & 0xFF;            /* APDU Tag */
    *p++ = (tag >> 8) & 0xFF;
    *p++ =  tag & 0xFF;
    p = ci_set_length(p, len);                /* Length Field of APDU */
    if (NULL == p)
        return ERR_FAILURE;
//    if (p - buffer + len < (int)(sizeof(buffer)))
    if (p - buffer + len < CI_MAX_SPDU_SIZE)
    {
        MEMCPY(p, data, len);                /* Data of APDU */
        p += len;
        // Send APDU in Transmission Connection
        return ci_trans_send_spdu(sc->tc, p - buffer, buffer);
    }
    SENS_PRINTF("ci_trans_send_SPDU: data length (%d) exceeds buffer size\n", len);

    return ERR_FAILUE;
}

/* Get Session APDU from SPDU */
UINT8 * ci_senss_get_apdu(struct ci_senssion_connect *sc, int *len, UINT8 *data)
{
    data = ci_get_length(data, len);
    return *len ? data : NULL;
}

/* Create Session Connection */
static struct ci_senssion_connect * ci_senss_create(struct ci_senssion_layer *senss, int rscid)
{
    int i;
    struct ci_senssion_connect *sc;

    // Check Session Connection Exist Or Not
    if ((sc = ci_get_senss_by_rscid(senss, rscid, senss->tc->slot)) == NULL)
    {
        for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
        {
            if (senss->connect[i].tc == NULL)
            {
                switch (rscid)
                {
                case RSC_ID_RESOURCE_MANAGER:
                //case RSC_ID_RESOURCE_MANAGER_V2:
                    SENS_PRINTF("ci_senss_create: Create resource manager, %d\n", i + 1);
                    return ci_rm_create(&(senss->connect[i]), i + 1, senss->tc);
                //case RSC_ID_APPLICATION_INFORMATION:
                //case RSC_ID_APPLICATION_INFORMATION_V2:
                case RSC_ID_APPLICATION_INFORMATION_V1:
                case RSC_ID_APPLICATION_INFORMATION_V3:    /*CI+*/
                    SENS_PRINTF("ci_senss_create: Create application information, %d\n", i + 1);
                    return ci_ai_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_CONDITIONAL_ACCESS_SUPPORT:
                    SENS_PRINTF("ci_senss_create: Create CA support, %d\n", i + 1);
                    return ci_cas_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_HOST_CONTROL:
                    SENS_PRINTF("ci_senss_create: Create host control, %d\n", i + 1);
                    return ci_hc_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_DATE_TIME:
                    SENS_PRINTF("ci_senss_create: Create date and time, %d\n", i + 1);
                    return ci_dat_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_MMI:
                    SENS_PRINTF("ci_senss_create: Create MMI, %d\n", i + 1);
                    /* Remove Timer of Last MMI Session Connection If CAM Feedback */
                    while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void*)(senss->tc->slot), (void*)API_MSG_ENTER_MENU));
                    return ci_mmi_create(&(senss->connect[i]), i + 1, senss->tc);
//                case RSC_ID_SMART_CARD_READER:
//                    SENS_PRINTF("ci_senss_create: Create SCR, %d\n", i + 1);
//                    return ci_scr_create(&(senss->connect[i]), i + 1, senss->tc);
#ifdef CI_PLUS_SUPPORT
                /* CI+: Resources Begin */
                case RSC_ID_HOST_LANGUAGE_COUNTRY:
                    SENS_PRINTF("ci_senss_create: Create HLC, %d\n", i + 1);
                    return ci_hlc_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_CAM_UPGRADE:
                    SENS_PRINTF("ci_senss_create: Create CAM UPG, %d\n", i + 1);
                    return ci_camup_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_CONTENT_CONTROL:
                    SENS_PRINTF("ci_senss_create: Create CC, %d\n", i + 1);
                    return ci_cc_create(&(senss->connect[i]), i + 1, senss->tc);
                case RSC_ID_APPLICATION_MMI:
                    SENS_PRINTF("ci_senss_create: Create APP MMI, %d\n", i + 1);
                    return ci_appmmi_create(&(senss->connect[i]), i + 1, senss->tc);
                /* CI+: Resources End*/
#endif
#ifdef HD_PLUS_SUPPORT
                case RSC_ID_HD_PLUS:
                    SENS_PRINTF("ci_senss_create: Create HD Plus, %d\n", i + 1);
                    return ci_hdplus_create(&(senss->connect[i]), i + 1, senss->tc);
#endif
                default:
                    SENS_PRINTF("ci_senss_create: Unknown resource identifier: %08X\n", rscid);
                    return NULL;
                }
            }
        }
    }
    else
    {
        /* Blue Viaccess Card will Create Session MMI Again */
        // Return Session Connection which Already Exist
        return sc;
    }
    return NULL;
}

#if 0
/* Request Create Session Connection (Host to Resource Provider) */
static INT32 ci_senss_create_conn_requ(struct ci_senssion_layer *senss, int rscid)
{
    struct ci_senssion_connect *sc;

    SENS_PRINTF("ci_senss_create_conn_requ: Open session %08X\n", rscid);
    // Create Session Connection
    if ((sc = ci_senss_create(senss, rscid)) != NULL)
    {
        // Send Create Session Connection Request to Resource Provider
        if (SUCCESS == ci_senss_send_spdu(sc, SENSS_TAG_CREATE_SESSION, sc->rscid, SENSS_STATUS_NOT_ALLOCATED))
        {
            return SUCCESS;
        }
        // If Fail, then Delete this Session Connection
        else
        {
            if (sc->appdelete)
            {
                sc->appdelete(sc);
            }
            ci_senss_conn_init(sc, 0, 0, NULL);
            SENS_PRINTF("ci_senss_create_conn_requ: create session response failue\n");
        }
    }
    else
    {
        SENS_PRINTF("ci_senss_create_conn_requ: Can't create session for resource identifier: %08X\n", rscid);
    }
    return ERR_FAILUE;
}
#endif

/* Response to Open Session Connection Request */
static INT32 ci_senss_open_conn_resp(struct ci_senssion_layer *senss, int len, UINT8 *data)
{
    struct ci_senssion_connect *sc;
    int rscid;

    // Check Length of Open Session Connection Request
    if (len == 6 && *(data + 1) == 0x04)
    {
        // Get Resource ID
        rscid = ci_resourceid_to_int(data + 2);
        SENS_PRINTF("ci_senss_open_conn_resp: Open session %08X\n", rscid);
        // Create Session Connection
        if ((sc = ci_senss_create(senss, rscid)) != NULL)
        {
            // Send Open Session Connection Response
            if (SUCCESS == ci_senss_send_spdu(sc, SENSS_TAG_OPEN_SESSION_RESPONSE, sc->rscid, SENSS_STATUS_OK))
            {
                /* When Open Session Connection, Meanwhile Call Application Layer */
                if (sc->callback)
                {
                    return sc->callback(sc, 0, NULL);
                }
                return SUCCESS;
            }
            // If Fail, then Delete this Session Connection
            else
            {
                if (sc->appdelete)
                {
                    sc->appdelete(sc);
                }
                ci_senss_conn_init(sc, 0, 0, NULL);
                SENS_PRINTF("ci_senss_open_conn_resp: open session response failue\n");
            }
        }
        else
        {
            SENS_PRINTF("ci_senss_open_conn_resp: Can't create session for resource identifier: %08X\n", rscid);
        }
    }
    return ERR_FAILUE;
}

//20151102: do NOT mark this function for compile error
#if 1
/* Send Request for Close Session Connection */
INT32 ci_senss_close_conn_requ(struct ci_senssion_connect *sc)
{
    SENS_PRINTF("ci_senss_close_conn_requ: nb = %04X\n", sc->scnb);
    if (sc && sc->tc)
    {
        // Send Request (from Host to CAM) for Close Session Connection
        if (SUCCESS != ci_senss_send_spdu(sc, SENSS_TAG_CLOSE_SESSION_REQUEST, 0, SENSS_STATUS_NOT_ALLOCATED))
        {
            SENS_PRINTF("ci_senss_close_conn_requ: close session failue\n");
            // Delete Session Connection
            if (sc->appdelete)
            {
                sc->appdelete(sc);
            }
            ci_senss_conn_init(sc, 0, 0, NULL);
        }
        return SUCCESS;
    }
    else
    {
        SENS_PRINTF("ci_senss_close_conn_requ: unknown session id: %d\n", sc->scnb);
    }
    return ERR_FAILUE;
}
#endif
/* Close Session Connection Directly */
static INT32 ci_senss_close_conn_requ_d(struct ci_senssion_connect *sc)
{
    SENS_PRINTF("ci_senss_close_conn_requ_d: nb = %04X\n", sc->scnb);
    if (sc && sc->tc)
    {
        // Delete Session Connection
        if (sc->appdelete)
        {
            sc->appdelete(sc);
        }
        ci_senss_conn_init(sc, 0, 0, NULL);
        return SUCCESS;
    }
    else
    {
        SENS_PRINTF("ci_senss_close_conn_requ_d: unknown session id: %d\n", sc->scnb);
    }
    return ERR_FAILUE;
}

/* Send Close Session Connection Response to CAM */
static INT32 ci_senss_close_conn_resp(struct ci_senssion_connect *sc)
{
    SENS_PRINTF("ci_senss_close_conn_resp: nb = %04X\n", sc->scnb);
    if (sc && sc->tc)
    {
        // Send Close Session Connection Response to CAM in SPDU
        ci_senss_send_spdu(sc, SENSS_TAG_CLOSE_SESSION_RESPONSE, 0, SENSS_STATUS_OK);
        // Delete Session Connection
        if (sc->appdelete)
        {
            sc->appdelete(sc);
        }
        ci_senss_conn_init(sc, 0, 0, NULL);
        return SUCCESS;
    }
    else
    {
        SENS_PRINTF("ci_senss_close_conn_resp: unknown session id: %d\n", sc->scnb);
    }
    return ERR_FAILUE;
}

#if 0
/* Close All Session Connections (Send Close Session Connection Request to CAM) */
static int ci_senss_close_all_conn(struct ci_senssion_layer *senss, int slot)
{
    int i, result = 0;

    for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
    {
        if (NULL != senss->connect[i].tc && senss->connect[i].tc->slot == slot)
        {
            // Close One Session Connection (Send Close Session Connection Request to CAM)
            ci_senss_close_conn_requ(&(senss->connect[i]));
            result++;
        }
    }
    return result;
}
#endif
/* Close All Session Connections Directly (Don't Send Close Session Connection Request to CAM) */
static int ci_senss_close_all_conn_d(struct ci_senssion_layer *senss, int slot)
{
    int i, result = 0;

    for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
    {
        if (NULL != senss->connect[i].tc && senss->connect[i].tc->slot == slot)
        {
            // Close One Session Connection Directly (Don't Send Close Session Connection Request to CAM)
            ci_senss_close_conn_requ_d(&(senss->connect[i]));
            result++;
        }
    }
    return result;
}

/* Convert Resource ID to 32bit-Integer */
static INT32 ci_resourceid_to_int(UINT8 *data)
{
    return ntohl(data);
}

/* Get Session Connection according to Session Connection */
static struct ci_senssion_connect * ci_get_senss_by_scnb(struct ci_senssion_layer *senss, int scnb)
{
    int i;
    struct ci_senssion_connect *sc;

    for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
    {
        sc = &(senss->connect[i]);
        if (sc->tc && sc->scnb == scnb)
        {
            return sc;
        }
    }
    return NULL;
}

/* Get Session Connection according to Resource ID */
struct ci_senssion_connect * ci_get_senss_by_rscid(struct ci_senssion_layer *senss, int rscid, int slot)
{
    int i;
    struct ci_senssion_connect *sc;

    for (i = 0; i < CI_MAX_SENSSCON_NUM; i++)
    {
        sc = &(senss->connect[i]);
        if (sc->tc && (sc->tc->slot == slot) && (sc->rscid == rscid))
        {
            return sc;
        }
    }
    return NULL;
}

