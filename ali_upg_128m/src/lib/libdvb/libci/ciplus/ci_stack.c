/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_stack.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack.
*    History:
*               Date                Athor         Version          Reason
*      ===========    ========   =====    ==========================
*    1.  Nov.10.2004       Justin Wu      Ver 0.1     Create file.
*    2.  Dec.16.2005       Justin Wu      Ver 0.2     Support timeout event.
*    3.  Dec.20.2005      Justin Wu    Ver 0.3     Support message protect.
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
#ifdef CI_SUPPORT

static void api_ci_main_thread(UINT32 param1, UINT32 param2);
static struct ci_timer* ci_timer_alloc();
static void ci_timer_free(struct ci_timer *timer);

static struct ci_stack_st ci_stack;

/*****************************************************************************
* INT32 api_ci_register(char *nim_name)
*
* Register a CI Stack Library (including Mutex, Message Buffer, CI Thread)
*
* Arguments:
*      Parameter1: char *cic_name
*
* Return Value: INT32
*****************************************************************************/
INT32 api_ci_register(char *cic_name, void (*cic_callback)(int slot, enum api_ci_msg_type type))
{
        OSAL_T_CTSK t_ctsk;
        OSAL_T_CMBF t_cmbf;
        OSAL_T_CTIM t_ctim;

    ci_control_init();

    MEMSET(&ci_stack, 0, sizeof(struct ci_stack_st));
    ci_stack.ci_stack_timers.next = NULL;
    ci_stack.cam_hw_active_flag = 0;
    ci_stack.callback = cic_callback;
    /* Create CI Mutex */
        ci_stack.mutex_id = osal_mutex_create();
        if (OSAL_INVALID_ID == ci_stack.mutex_id)
        {
            INFX_PRINTF("Can not create CI mutex!!!\n");
        ASSERT(0);
        }
    /* Create CI Message Buffer */
        t_cmbf.bufsz = 0x800;
        t_cmbf.maxmsz = 16;
        ci_stack.msgque_id = osal_msgqueue_create(&t_cmbf);
        if (OSAL_INVALID_ID == ci_stack.msgque_id)
        {
            INFX_PRINTF("Can not create CI message buffer!!!\n");
        ASSERT(0);
        }
        /* Attach Lower Stack Component */
        if (ci_senss_attach(&(ci_stack.senss)) != SUCCESS)
        {
        ASSERT(0);
        }
    /* Create CI Thread */
#ifdef CI_PLUS_SUPPORT
        t_ctsk.stksz = 0x4000;    //CI+
#else
        t_ctsk.stksz = 0x1800;
#endif
        t_ctsk.quantum = 5;
        t_ctsk.task = api_ci_main_thread;
        t_ctsk.itskpri = OSAL_PRI_NORMAL;
        ci_stack.thread_id = osal_task_create(&t_ctsk);
        if (OSAL_INVALID_ID == ci_stack.thread_id)
        {
            INFX_PRINTF("Can not create CI main thread!!!\n");
        ASSERT(0);
        }

        return SUCCESS;
}

/*****************************************************************************
* INT32 api_ci_unregister(char *cic_name)
*
* Unregister a CI Stack Library
*
* Arguments:
*      Parameter1: char *cic_name
*
* Return Value: INT32
*****************************************************************************/
INT32 api_ci_unregister(char *cic_name)
{
    /* Delete CI Thread */
    if (ci_stack.thread_id != OSAL_INVALID_ID)
    {
        osal_task_delete(ci_stack.thread_id);
    }
    /* Delete CI Message Buffer */
    if (ci_stack.msgque_id != OSAL_INVALID_ID)
    {
        osal_msgqueue_delete(ci_stack.msgque_id);
    }
    /* Delete CI Mutex */
    if (ci_stack.mutex_id != OSAL_INVALID_ID)
    {
        osal_mutex_delete(ci_stack.mutex_id);
    }

    /* Detach Lower Stack Component */
    ci_senss_detach(&(ci_stack.senss));

    MEMSET(&ci_stack, 0, sizeof(struct ci_stack_st));

    return SUCCESS;
}

/* Get CI Module Status */
int api_ci_check_status(int slot)
{
    return ci_stack.senss.trans.link.slot[slot].state;
}

/* Get CICAM Type */
int api_ci_get_cam_type(int slot)
{
    return ci_stack.senss.trans.link.slot[slot].cam_type;
}

/* Init CICAM Type */
void api_ci_init_cam_type(int slot)
{
    ci_stack.senss.trans.link.slot[slot].cam_type = CICAM_DVB_NONE;
}

/* Get CICAM Brand ID */
UINT16 api_ci_get_cicam_brand_id(int slot)
{
#ifdef CI_PLUS_SUPPORT
    struct api_ci_msg msg_st;
    UINT16 cicam_brand_id;
    UINT32 fptn;

    msg_st.type = API_MSG_GET_CAMBID;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&cicam_brand_id;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return 0;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_CAMBID,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return cicam_brand_id;
#else
    return 0;
#endif
}

/* Get Application Type */
UINT8 api_ci_get_application_type(int slot)
{
    struct api_ci_msg msg_st;
    UINT8 type;
    UINT32 fptn;

    msg_st.type = API_MSG_GET_APTYPE;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&type;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return APP_TYPE_NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_APTYPE,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return type;
}

/* Get Application Manufacturer */
UINT16 api_ci_get_application_manufacturer(int slot)
{
    struct api_ci_msg msg_st;
    UINT16 id;
    UINT32 fptn;

    msg_st.type = API_MSG_GET_APMANUFACTURER;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&id;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return 0;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_APMANUFACTURER,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return id;
}

/* Get Application Menu String */
char * api_ci_get_menu_string(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_MENUSTRING;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;

    //ci stack need get module name too.
    //  We may have following case
    //  @ We want to get menu string in app task
    //  @ Then we want to get the menu in ci task
    //  @    The app requirement will be buffered
    //  @    The ci requirement will be executed firstly
    //  @    The app requirement will be cleared
    //  @        Crash due to the clear buffer parameter
    if (osal_task_get_current_id() == ci_stack.thread_id)
    {
        /* ci_message_process(&ci_stack.senss, &msg_st);
        osal_flag_clear(ci_stack.senss.trans.link.slot[slot].flag_id,
                        API_FLAG_MENUSTRING); */
        MEMSET(buffer, 0x00, len);
        MEMCPY(buffer, ci_stack.cam_name[slot], len);
    }
    else
    {
        if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
            return NULL;
        osal_flag_wait(&fptn,
                        ci_stack.senss.trans.link.slot[slot].flag_id,
                        API_FLAG_MENUSTRING,
                        OSAL_TWF_ORW | OSAL_TWF_CLR,
                        OSAL_WAIT_FOREVER_TIME);
    }

    return buf;
}

/* Enter Menu */
void api_ci_enter_menu(int slot)
{
    api_ci_msg_to_stack(slot, API_MSG_ENTER_MENU);
}

/* Exit Menu */
void api_ci_exit_menu(int slot)
{
    api_ci_msg_to_stack(slot, API_MSG_EXIT_MENU);
}

/* Get CA System IDs */
UINT16 * api_ci_get_cas_ids(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_CASIDS;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_CASIDS,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}

/* Send CA PMT */
void api_ci_send_pmt(int slot, UINT32 cmd)
{
    struct api_ci_msg msg_st;
    UINT32 fptn;

    /* Counter for Filter Unwanted CA_PMT Sending Action */
    osal_task_dispatch_off();
    ci_stack.senss.trans.link.slot[slot].ca_pmt_cnt++;
    osal_task_dispatch_on();

    msg_st.type = API_MSG_CA_PMT;
    msg_st.slot = slot;
    msg_st.param = cmd;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_SENDCAPMT,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);
}

/* Get Menu in MMI */
struct ci_menu * api_ci_get_menu(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_MENU;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_MENU,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}

/* Get Enquiry in MMI */
struct ci_enquiry * api_ci_get_enquiry(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_ENQUIRY;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_ENQUIRY,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}

/* Select Current Level Menu in MMI */
void api_ci_menu_select(int slot, int n)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_SELECT_MENU;
    msg_st.slot = slot;
    msg_st.param = n;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

/* Cancel Current Level Menu in MMI */
void api_ci_menu_cancel(int slot)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_CANCEL_MENU;
    msg_st.slot = slot;
    msg_st.param = -1;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

/* CI Card Not Select */
void api_ci_not_selected(int slot)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_CI_NOT_SELECTED;
    msg_st.slot = slot;
    msg_st.param = -1;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

RET_CODE  api_ci_send_allpmt(int slot)
{
    char name[][16] = {"TechniSat"};
    char patch_module[][16] = {"Videoguard CA"};
    char module_name[33];
    UINT32 i;

    for (i=0; i<1; i++)
    {
        if (_strncasecmp(&name[i][0],
                        &ci_stack.senss.trans.link.slot[slot].version_infor.str[ci_stack.senss.trans.link.slot[slot].version_infor.ofs[0]],
                        STRLEN(&ci_stack.senss.trans.link.slot[slot].version_infor.str[ci_stack.senss.trans.link.slot[slot].version_infor.ofs[0]])
                        ) == 0
            )
            return RET_SUCCESS;
    }
    api_ci_get_menu_string(module_name, 33, slot);

    for (i=0; i<1; i++)
    {
        if (_strncasecmp(module_name, patch_module[i], STRLEN(patch_module[i])) == 0)
            return RET_SUCCESS;
    }
    return !RET_SUCCESS;
}
/*******************************************************************************
*   Function name: api_ci_set_mmi_visualstatus() & ci_get_mmi_visualstatus()
*   Description:
*       1. these two function are used to set and get mmi menu visual status; when UI display
*           ci_menu, set to 1;
*       2. because host send ca_pmt, then CI_CARD respond to it, and then if ci_card need pin code,
*           ci_menu is displayed to communicate with user. So during ci_menu display, don't send
*           ca_pmt again, because ci_card need pin code, not ca_pmt.
*       3. api_ci_set_mmi_visualstatus() is provided for UI layer. Its default value is 0. If UI layer
*           not set to 1, Middle layer will send ca_pmt every interval even if ci_menu is display,
*           just as before.
*   Time : 20080303
*******************************************************************************/
static INT8 mmimenu_visual_status = 0;

/* Set MMI Visual Status */
void api_ci_set_mmi_visualstatus(INT8 status)
{
        mmimenu_visual_status = status;
}

/* Get MMI Visual Status */
INT8 ci_get_mmi_visualstatus()
{
        return mmimenu_visual_status;
}

/*******************************************************************************
*   Function name: ci_set_mmi_enablestatus() & api_ci_get_mmi_enablestatus()
*   Description:
*       1. these two function are used to set and get mmi enable status.
*       2. According to spec_en_50221, Page31, only ca_pmt_cmd_id of ca_pmt  == 1 or 2,
*           mmi_menu can be start, so UI layer should decide if it can display ci_menu. This
*           function will tell it.
*   Time : 20080303
*******************************************************************************/
static INT8 mmi_enable_status = 0;

/* Set MMI Enable Status */
void ci_set_mmi_enablestatus(INT8 status)
{
        mmi_enable_status = status;
}

/* Get MMI Enable Status */
INT8 api_ci_get_mmi_enablestatus()
{
        return mmi_enable_status;
}

/* Reply Enquiry in MMI */
void api_ci_enquiry_reply(int slot, char *s)
{
    struct api_ci_msg msg_st;
    UINT32 fptn;

    msg_st.type = API_MSG_REPLY_ENQUIRY;
    msg_st.slot = slot;
    msg_st.param = (UINT32)s;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_ENQUIRYRPLY,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);
}

/* Cancel Enquiry in MMI */
void api_ci_enquiry_cancel(int slot)
{
    api_ci_msg_to_stack(slot, API_MSG_CANCEL_ENQUIRY);
}

/* APP MMI: get AppDomainIdentifier & InitialObject */
void *api_ci_get_initial_object(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_INITIAL_OBJECT;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_INITIAL_OBJECT,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}

void api_ci_request_start_ack(UINT8 ackcode, int slot)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_REQUEST_START_ACK;
    msg_st.slot = slot;
    msg_st.param = ackcode;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

void api_ci_file_request(UINT8 requesttype, UINT8 *data, int len, int slot)
{
    struct api_ci_msg msg_st;
    struct ci_file_request file_request;
    UINT32 fptn;

    file_request.requesttype = requesttype;
    file_request.len = len;
    file_request.data = data;
    msg_st.type = API_MSG_FILE_REQUEST;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&file_request;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_FILE_REQUEST,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);
}

void *api_ci_get_file_data(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_FILE_DATA;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_FILE_DATA,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}

void api_ci_app_abort_request(UINT8 abortreqcode, int slot)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_ABORT_REQUEST;
    msg_st.slot = slot;
    msg_st.param = abortreqcode;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

void api_ci_app_abort_ack(UINT8 abortackcode, int slot)
{
    struct api_ci_msg msg_st;

    msg_st.type = API_MSG_ABORT_ACK;
    msg_st.slot = slot;
    msg_st.param = abortackcode;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

#ifdef CI_PLUS_SUPPORT
/*void *api_ci_get_uri_msg(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    MEMSET(buffer, 0, len);
    //  *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_GET_URI_MSG;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_URI_MSG,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return buf;
}*/

void ci_uri_monitor(struct ci_senssion_connect *sc, UINT32 program_number);
void ci_set_uri_timer(int slot, UINT16 program_number)
{
    struct ci_senssion_connect *cc_sc;
    struct ci_uri_message *uri_msg;

    if ((cc_sc=ci_get_senss_by_rscid(&(ci_stack.senss), RSC_ID_CONTENT_CONTROL, slot)) != NULL)
    {
        //set URI default
        uri_msg = ci_cc_get_uri_buf(cc_sc->appdata, sizeof(struct ci_uri_message));
        set_uri_default(uri_msg);

        while (SUCCESS == ci_del_timer((ci_timer_handler)ci_uri_monitor, (void *)cc_sc, (void *)CI_TIMER_PARAM_ANY));

        ci_set_timer(CI_URI_MONITOR, (ci_timer_handler)ci_uri_monitor, (void *)cc_sc, (void*)program_number);
    }
}

void ci_set_uri_cam_match(int slot)
{
    struct ci_senssion_connect *cc_sc;
    if ((cc_sc=ci_get_senss_by_rscid(&(ci_stack.senss), RSC_ID_CONTENT_CONTROL, slot)) != NULL)
    {
        notify_uri(slot, URI_NOT_RECEIVED);
    }
    else
    {
        notify_uri(slot, URI_CLEAR);
    }
}

void ci_del_uri_monitor_timer(int slot)
{
    struct ci_senssion_connect *cc_sc;
    if ((cc_sc=ci_get_senss_by_rscid(&(ci_stack.senss), RSC_ID_CONTENT_CONTROL, slot)) != NULL)
    {
        while (SUCCESS == ci_del_timer((ci_timer_handler)ci_uri_monitor, (void *)cc_sc, (void *)CI_TIMER_PARAM_ANY));
    }
}
/*
INT32 api_ci_set_uri_msg(void *buffer, int len, int slot)
{
    struct api_ci_msg msg_st;
    void *buf;
    UINT32 fptn;

    //  *(int *)buffer = len;
    buf = buffer;
    msg_st.type = API_MSG_SET_URI_MSG;
    msg_st.slot = slot;
    msg_st.param = (UINT32)&buf;
    if (osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT) != OSAL_E_OK)
    {
        return NULL;
    }
    osal_flag_wait(&fptn,
                    ci_stack.senss.trans.link.slot[slot].flag_id,
                    API_FLAG_SET_URI_MSG,
                    OSAL_TWF_ORW | OSAL_TWF_CLR,
                    OSAL_WAIT_FOREVER_TIME);

    return SUCCESS;
}*/
#endif

/******************************************************************************/
#if 1
/* CI Main Thread */
static void api_ci_main_thread(UINT32 param1, UINT32 param2)
{
    struct api_ci_msg msg_st;
    UINT32 msg_len;
    UINT32 time, old_time, new_time;
    struct ci_timer *t;
    ci_timer_handler h;
    void *arg1, *arg2;

    while (1)
    {
        /* Process Message */
        ASSERT(ci_stack.ci_stack_timers.next != NULL)
        time = ci_stack.ci_stack_timers.next->time;
        msg_st.type = API_MSG_NULL;
        new_time = old_time = osal_get_tick();
        /* Receive and Process Messages */
        do
        {
            if (osal_msgqueue_receive(&msg_st, &msg_len, ci_stack.msgque_id, time - (new_time - old_time)) == OSAL_E_OK)
            {
                ci_message_process(&(ci_stack.senss), &msg_st);
            }
            new_time = osal_get_tick();
        } while (new_time - old_time < time);
        time = new_time - old_time;
        /* Check Timer and Handle Timer Event */
        do
        {
            old_time = osal_get_tick();
            for (t = ci_stack.ci_stack_timers.next; t != NULL && t->time <= time; t = t->next)
            {
                time -= t->time;
                ci_stack.ci_stack_timers.next = t->next;
                h = t->h;
                arg1 = t->arg1;
                arg2 = t->arg2;
                ci_timer_free(t);
                if (h != NULL)
                {
                    h(arg1, arg2);
                    break;        /* Break here for maybe operate timer in h() */
                }
            }
            new_time = osal_get_tick();
        } while (ci_stack.ci_stack_timers.next->time <= time);
        if (ci_stack.ci_stack_timers.next != NULL)
        {
            ci_stack.ci_stack_timers.next->time -= time;
        }
    }
}
#else
//Use Sys_Tick for Timer
static void api_ci_main_thread(UINT32 param1, UINT32 param2)
{
    struct api_ci_msg msg_st;
    UINT32 msg_len;
    UINT32 time;
#if CIPLUS_DEBUG_INFO_IFX
    UINT32 first_tick = -1;
#endif
    struct ci_timer *t;
    ci_timer_handler h;
    void *arg1, *arg2;

    while (1)
    {
        /* Process Message */
        ASSERT(ci_stack.ci_stack_timers.next != NULL)
        msg_st.type = API_MSG_NULL;
        time = osal_get_tick();
#if CIPLUS_DEBUG_INFO_IFX
        if (first_tick == -1)
        {
            first_tick = time;
            INFX_PRINTF("api_ci_main_thread: first_tick: %d\n",first_tick);
        }
#endif
        if (osal_msgqueue_receive(&msg_st, &msg_len, ci_stack.msgque_id, 0) == OSAL_E_OK)
        {
            ci_message_process(&(ci_stack.senss), &msg_st);
        }
        else
        {
            t = ci_stack.ci_stack_timers.next;
            if (t != NULL && t->time <= time)
            {
                ci_stack.ci_stack_timers.next = t->next;

                h = t->h;
                arg1 = t->arg1;
                arg2 = t->arg2;
                ci_timer_free(t);
                if (h != NULL)
                {
                    h(arg1, arg2);
                }
            }
            else
                osal_task_sleep(10);
        }
    }
}
#endif

static void dump_message(char *s, int slot, int msg)
{
    switch (msg)
    {
    case API_MSG_NULL:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_NULL)\n", s, slot, msg);
        break;
    /* Followwing message for CI stack internal & notify APP (<Stack> -> APP) */
    case API_MSG_CAMHW:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAMHW)\n", s, slot, msg);
        break;
    case API_MSG_CAMIN:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAMIN)\n", s, slot, msg);
        break;
    case API_MSG_CAMOUT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAMOUT)\n", s, slot, msg);
        break;
    case API_MSG_CTC:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CTC)\n", s, slot, msg);
        break;
    case API_MSG_DTC:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_DTC)\n", s, slot, msg);
        break;
    case API_MSG_NEGO:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_NEGO)\n", s, slot, msg);
        break;
    /* Followwing message only for API notify CI stack. (APP -> Stack) */
    case API_MSG_CA_PMT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CA_PMT)\n", s, slot, msg);
        break;
    case API_MSG_GET_APTYPE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_APTYPE)\n", s, slot, msg);
        break;
    case API_MSG_GET_APMANUFACTURER:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_APMANUFACTURER)\n", s, slot, msg);
        break;
    case API_MSG_GET_MENUSTRING:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_MENUSTRING)\n", s, slot, msg);
        break;
    case API_MSG_GET_CASIDS:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_CASIDS)\n", s, slot, msg);
        break;
    case API_MSG_ENTER_MENU:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_ENTER_MENU)\n", s, slot, msg);
        break;
    case API_MSG_GET_MENU:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_MENU)\n", s, slot, msg);
        break;
    case API_MSG_SELECT_MENU:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_SELECT_MENU)\n", s, slot, msg);
            break;
    case API_MSG_CANCEL_MENU:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CANCEL_MENU)\n", s, slot, msg);
        break;
    case API_MSG_GET_ENQUIRY:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_GET_ENQUIRY)\n", s, slot, msg);
        break;
    case API_MSG_REPLY_ENQUIRY:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_REPLY_ENQUIRY)\n", s, slot, msg);
        break;
    case API_MSG_CANCEL_ENQUIRY:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CANCEL_ENQUIRY)\n", s, slot, msg);
        break;
    /* Followwing message only for CI stack notify APP (Stack -> APP) */
    case API_MSG_CAAPP_OK:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAAPP_OK)\n", s, slot, msg);
        break;
    case API_MSG_MENU_UPDATE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_MENU_UPDATE)\n", s, slot, msg);
        break;
    case API_MSG_ENQUIRY_UPDATE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_ENQUIRY_UPDATE)\n", s, slot, msg);
        break;
    /* Followwing message for between CI stack and APP (Stack <-> APP) */

    case API_MSG_ENQUIRY_EXIT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_ENQUIRY_EXIT)\n", s, slot, msg);
        break;
    case API_MSG_CAPMT_OK:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAPMT_OK)\n", s, slot, msg);
        break;
    case API_MSG_CAPMT_CASYSTEM_INVALID:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAPMT_CASYSTEM_INVALID)\n", s, slot, msg);
        break;
    case API_MSG_CI_NOT_DESCRAMBLED:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_NOT_DESCRAMBLED)\n", s, slot, msg);
        break;

    case API_MSG_CI_REQUEST_START:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_REQUEST_START)\n", s, slot, msg);
        break;

    case API_MSG_CI_FILE_RECEIVED:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_FILE_RECEIVED)\n", s, slot, msg);
        break;

    case API_MSG_CI_APP_ABORT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_APP_ABORT)\n", s, slot, msg);
        break;

    case API_MSG_SERVICE_SHUNNING_UPDATE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_SERVICE_SHUNNING_UPDATE)\n", s, slot, msg);
        break;

    case API_MSG_CI_ICT_ENABLE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_ICT_ENABLE)\n", s, slot, msg);
        break;

    case API_MSG_CI_TUNE_SERVICE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_TUNE_SERVICE)\n", s, slot, msg);
        break;
    case API_MSG_TUNED_SEARCH_END:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_TUNED_SEARCH_END)\n", s, slot, msg);
        break;
    case API_MSG_CAMUP_END_PLAY:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CAMUP_END_PLAY)\n", s, slot, msg);
        break;
    case API_MSG_ENABLE_YUV_OUTPUT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_ENABLE_YUV_OUTPUT)\n", s, slot, msg);
        break;
    case API_MSG_MGCGMS_SET_VPO:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_MGCGMS_SET_VPO)\n", s, slot, msg);
        break;
    case API_MSG_SET_MG_INFO:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_SET_MG_INFO)\n", s, slot, msg);
        break;
    case API_MSG_SET_CGMS_INFO:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_SET_CGMS_INFO)\n", s, slot, msg);
        break;
    case API_MSG_DISABLE_ANALOG_OUTPUT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_DISABLE_ANALOG_OUTPUT)\n", s, slot, msg);
        break;
    case API_MSG_DISABLE_YUV_OUTPUT:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_DISABLE_YUV_OUTPUT)\n", s, slot, msg);
        break;
    case API_MSG_RESUME_TV_MODE:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_RESUME_TV_MODE)\n", s, slot, msg);
        break;
    case API_MSG_CIPLUS_DSC_RESET:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CIPLUS_DSC_RESET)\n", s, slot, msg);
        break;
    case API_MSG_CI_FILE_OK:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_FILE_OK)\n", s, slot, msg);
        break;
    case API_MSG_EXIT_MENU:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_EXIT_MENU)\n", s, slot, msg);
        break;
    case API_MSG_CI_NOT_SELECTED:
        INFX_PRINTF("%s: [slot %d], msg = %x (API_MSG_CI_NOT_SELECTED)\n", s, slot, msg);
        break;
    default:
        INFX_PRINTF("%s: [slot %d], msg = %x (Unknow)\n", s, slot, msg);
        break;
    }
}

/*  +---------- +  ----------+
 *    | Send Message to CI Stack |
 *   +---------- +  ----------+
 */
void api_ci_msg_to_stack(int slot, int msg)
{
    struct api_ci_msg msg_st;

#if CIPLUS_DEBUG_INFO_IFX
    dump_message("api_ci_msg_to_stack", slot, msg);
#else
    INFX_PRINTF("api_ci_msg_to_stack: [slot %d], msg = %x\n", slot, msg);
#endif
    msg_st.type = msg;
    msg_st.slot = slot;
    msg_st.param = 0;
    osal_msgqueue_send(ci_stack.msgque_id, &msg_st, sizeof(struct api_ci_msg), CI_MAX_MESSAGE_TIMEOUT);
}

/*  +----------+  Msg  +-----+
 *   | CI Stack |  <----- | CAM |
 *   +----------+         +-----+
 */
void api_ci_cam_to_stack(int slot)
{
    ci_stack.cam_hw_active_flag |= (1 << slot);
}

/*  +----------- +  -----------+
 *    | Send Message to Application |
 *   +----------- +  -----------+
 */
void api_ci_msg_to_app(int slot, int msg)
{
#if CIPLUS_DEBUG_INFO_IFX
    dump_message("api_ci_msg_to_app", slot, msg);
#else
    INFX_PRINTF("api_ci_msg_to_app: [slot %d], msg = %x\n", slot, msg);
#endif

    if(NULL != ci_stack.callback)
        ci_stack.callback(slot, msg);
}

/* Mutex Lock for Create CA PMT */
void api_ci_mutex_lock()
{
    osal_mutex_lock(ci_stack.mutex_id, OSAL_WAIT_FOREVER_TIME);
}

/* Mutex Unlock for Create CA PMT */
void api_ci_mutex_unlock()
{
    osal_mutex_unlock(ci_stack.mutex_id);
}

/******************************************************************************/
/* Allocate Timer */
static struct ci_timer* ci_timer_alloc()
{
    int i;

    for (i = 0; i < CI_MAX_TIMER_NUM; i++)
    {
        if (ci_stack.ci_stack_timer[i].h == NULL)
        {
            return &(ci_stack.ci_stack_timer[i]);
        }
    }

    return NULL;
}

/* Free Timer */
static void ci_timer_free(struct ci_timer *timer)
{
    timer->h = NULL;
}

#if 1
/* Set Timer */
INT32 ci_set_timer(UINT32 time, ci_timer_handler h, void *arg1, void *arg2)
{
    struct ci_timers *timers;
    struct ci_timer *timer, *t;

    // Set Timer Parameter
    timer = ci_timer_alloc();
    if (timer == NULL)
    {
        ASSERT(0);
        return ERR_FAILUE;
    }
    timer->next = NULL;
    timer->h = h;
    timer->arg1 = arg1;
    timer->arg2 = arg2;
    timer->time = time;
    timers = &(ci_stack.ci_stack_timers);

    // Insert Timer into Timer Queue
    if (timers->next == NULL)
    {
        timers->next = timer;
        return SUCCESS;
    }
    if (timers->next->time > time)
    {
        timers->next->time -= time;
        timer->next = timers->next;
        timers->next = timer;
    }
    else
    {
        for(t = timers->next; t != NULL; t = t->next)
        {
            timer->time -= t->time;
            if (t->next == NULL || t->next->time > timer->time)
            {
                if (t->next != NULL)
                {
                    t->next->time -= timer->time;
                }
                timer->next = t->next;
                t->next = timer;
                break;
            }
        }
    }
    return SUCCESS;
}

/* Delete Timer */
INT32 ci_del_timer(ci_timer_handler h, void *arg1, void *arg2)
{
    struct ci_timers *timers;
    struct ci_timer *prev_t, *t;

    timers = &(ci_stack.ci_stack_timers);

    if (timers->next == NULL)
    {
        return ERR_FAILUE;
    }

    for (t = timers->next, prev_t = NULL; t != NULL; prev_t = t, t = t->next)
    {
        if ((t->h == h) && (t->arg1 == arg1 || CI_TIMER_PARAM_ANY == arg1) &&
            (t->arg2 == arg2 || CI_TIMER_PARAM_ANY == arg2))
        {
            /* Match: Remove from Previous in List */
            if (prev_t == NULL)
            {
                timers->next = t->next;
            }
            else
            {
                prev_t->next = t->next;
            }
            /* If Not Last One, Add Time of This One Back to Next */
            if (t->next != NULL)
            {
                t->next->time += t->time;
            }

            INFX_PRINTF("ci_del_timer: %p h=%p arg1=%p arg2=%p\n",
                (void *)t, (void *)h, (void *)arg1, (void *)arg2);

            /* Here Free this Timer */
            ci_timer_free(t);
            return SUCCESS;
        }
    }
    return ERR_FAILUE;
}
#else
//use the sys_tick for timer
INT32 ci_set_timer(UINT32 time, ci_timer_handler h, void *arg1, void *arg2)
{
    struct ci_timers *timers;
    struct ci_timer *timer, *t;

    timer = ci_timer_alloc();
    if (timer == NULL)
    {
        ASSERT(0);
        return ERR_FAILUE;
    }
    timer->next = NULL;
    timer->h = h;
    timer->arg1 = arg1;
    timer->arg2 = arg2;
    time += osal_get_tick();
    timer->time = time;
    timers = &(ci_stack.ci_stack_timers);

    if (timers->next == NULL)
    {
        timers->next = timer;
        return SUCCESS;
    }
    if (timers->next->time > time) {
        timer->next = timers->next;
        timers->next = timer;
    } else
    {
        for(t = timers->next; t != NULL; t = t->next)
        {
            if (t->next == NULL || t->next->time > timer->time)
            {
                timer->next = t->next;
                t->next = timer;
                break;
            }
        }
    }
    return SUCCESS;
}

INT32 ci_del_timer(ci_timer_handler h, void *arg1, void *arg2)
{
    struct ci_timers *timers;
    struct ci_timer *prev_t, *t;

    timers = &(ci_stack.ci_stack_timers);

    if (timers->next == NULL)
        return ERR_FAILUE;

    for (t = timers->next, prev_t = NULL; t != NULL; prev_t = t, t = t->next)
    {
        if ((t->h == h) && (t->arg1 == arg1 || CI_TIMER_PARAM_ANY == arg1) &&
            (t->arg2 == arg2 || CI_TIMER_PARAM_ANY == arg2))
        {
            /* Match: remove from previous in list */
            if (prev_t == NULL)
                timers->next = t->next;
            else
                prev_t->next = t->next;

            INFX_PRINTF("ci_del_timer: %p h=%p arg1=%p arg2=%p\n",
                (void *)t, (void *)h, (void *)arg1, (void *)arg2);

            /* Here free this timer */
            ci_timer_free(t);
            return SUCCESS;
        }
    }
    return ERR_FAILUE;
}
#endif

/* Set CAM name in ci stack */
RET_CODE ci_set_cam_name(void *cam_name, UINT8 len, UINT8 slot)
{
    ASSERT(slot < CI_MAX_SLOT_NUM);

    if (len && cam_name)
        MEMCPY(ci_stack.cam_name[slot], cam_name, len);
    else
        MEMSET(ci_stack.cam_name[slot], 0x00, CAM_NAME_MAX_LEN);

    return RET_SUCCESS;
}

/* Set Length Field of PDU */
UINT8 *ci_set_length(UINT8 *p, int len)
{
    int i;

    if (len < 128)
    {
        *p++ = len;
    }
    else
    {
        for (i = 1; i < 4; i++)
        {
            if ((len & (0xff << (8 * i))) == 0)
            {
                break;
            }
        }
        for (*p++ = (0x80 | i); i > 0; i--)
        {
            *p++ = (len >> (8 * (i - 1)));
        }
    }

    return p;
}

/* Get Length Field of PDU */
UINT8 *ci_get_length(UINT8 *p, int *len)
{
    int i;

    if ((*p & 0x80) == 0)
    {
        *len = *p++;
    }
    else
    {
        for (i = (*p++ & 0x7f), *len = 0; i > 0; i--)
        {
            *len |= (*p++ << (8 * (i - 1)));
        }
    }

    return p;
}

UINT16 ntohs(UINT8 *data)
{
    return (data[0] << 8) | data[1];
}

UINT32 ntohl(UINT8 *data)
{
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

UINT16 htons(UINT16 n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

UINT32 htonl(UINT32 n)
{
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) |
           ((n & 0xff000000) >> 24);
}
#else    //CI_SUPPORT
INT32 api_ci_register(char *cic_name, void (*cic_callback)(int slot, enum api_ci_msg_type type))
{
       return SUCCESS;
}
INT32 api_ci_unregister(char *cic_name)
{
       return SUCCESS;
}
void api_ci_mutex_lock()
{
}
void api_ci_mutex_unlock()
{
}
char * api_ci_get_menu_string(void *buffer, int len, int slot)
{
    char *buf = (char*)buffer;

    if (buffer != NULL)
        buf[0] = '\0';

    return buffer;
}
#endif    //CI_SUPPORT

