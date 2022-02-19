/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_plus.h
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Nov.10.2004      Justin Wu      Ver 0.1    Create file.
*    2.  Dec.16.2005      Justin Wu      Ver 0.2    Support timeout event.
*    3.  Dec.20.2005         Justin Wu        Ver 0.3    Support message protect;
*                                                   Spit head file & rename.
*    4.    Jan.25.2008      Steven         Ver 0.4    Add CI platform interface.
*    5.    Mar.21.2008      Steven         Ver 0.5    Serial & parallel mode.
*    6.    2009.10.22       Steven                    Add IO Control API for compatibility.
*    7.    Feb.23.2010      Steven         Ver 0.6    Add new resources for CI+
*    8.    May.26.2010      Elliott        Ver 0.7    Add new CI+ interfaces.
*    9.    May.28.2010      Elliott        Ver 0.8    Add service type in uri message.
*    10.    Jun.09.2010      Elliott        Ver 0.9    Add ICT msg.
*       11.    Jun.29.2010      Elliott        Ver 1.0    Add API for mg query.
*****************************************************************************/
#include <types.h>
#include <mediatypes.h>

#ifndef __CI_PLUS_H__
#define __CI_PLUS_H__


#ifdef CONFIG_SLOT_NUM
#define CI_MAX_SLOT_NUM          CONFIG_SLOT_NUM  //   1
#else
#define CI_MAX_SLOT_NUM          2
#endif

//for descrambled program, bypass cam
//#define CI_BYPASS_ENABLE

//CI+: data_rate_info
#ifdef CI_PLUS_SUPPORT
#define CI_DATA_RATE    96    /*CI+*/
#else
#define CI_DATA_RATE    72    /*CI*/
#endif

//CI+: country code & language code
#define CI_ISO3166_COUNTRY_CODE        "GBR"
#define CI_ISO639_2_LANGUAGE_CODE    "eng"
//#define CI_ISO3166_COUNTRY_CODE        "CHN"
//#define CI_ISO639_2_LANGUAGE_CODE    "chi"

//CI tuned service status
#define CI_TUNED_SUCCESS 0
#define CI_TUNED_FAILURE  1

/* Slot state:
 *              (CAM In)->        CAM_ATTACH    (Stack Init)->
 * CAM_DETACH ----------------                  -------------- CAM_STACK_ATTACH
 *            <-(Stack unInit) CAM_STACK_DETACH  <-(CAM Out)
 */
enum ci_slot_state {
    CAM_DETACH                    = 0,    /* CAM card is detach state */
    CAM_ATTACH                    = 1,    /* CAM card is attach state */
    CAM_STACK_ATTACH            = 2,    /* CAM card stack attach state */
    CAM_STACK_DETACH            = 3        /* CAM card stack detach state */
};

enum ci_slot_mode {
    CI_MODE_NONE                = 0,    /* CAM card is serial link */
    CI_SERIAL_MODE                = 1,    /* CAM card is serial link */
    CI_PARALLEL_MODE            = 2        /* CAM card is parallel separately */
};

enum ci_pvr_mode
{
    CI_RE_ENCRYPT = 0,
    CI_NO_RE_ENCRYPT = 1,
    CI_NO_RECEIVED_URI = 2,
    CI_NO_MATCH_CAM = 3
    //CI_RE_ENCODED_ECM = 1,
    //CI_ORIGINAL_ECM = 2
};

/* device list for ci */
#define CI_MAX_DEV_NUM                32
struct ci_device_list
{
    void *nim_dev;
    void *dmx_dev;

    //total CI_MAX_DEV_NUM * UINT32
    UINT32 reserved[CI_MAX_DEV_NUM-2];
};

/* ci service information */
struct ci_service_info
{
    UINT32 prog_id;
    UINT32 monitor_id;

    UINT16 video_pid;
    UINT16 audio_pid;

    //091021 added,    and ci only support descramble 1 ttx, 1 subt
    UINT16 ttx_pid;
    UINT16 subt_pid;

    //total 128 * UINT32
    UINT32 reserved[128-4];
};

/* CA PMT */
enum ci_ca_pmt_cmd_id {
    CI_CPCI_OK_DESCRAMBLING        = 0x01,
    CI_CPCI_OK_MMI                = 0x02,
    CI_CPCI_QUERY                = 0x03,
    CI_CPCI_NOT_SELECTED        = 0x04
};

enum ci_ca_pmt_list_management {
    CI_CPLM_MORE                        = 0x00,
    CI_CPLM_FIRST                        = 0x01,
    CI_CPLM_LAST                        = 0x02,
    CI_CPLM_ONLY                        = 0x03,
    CI_CPLM_ADD                            = 0x04,
    CI_CPLM_UPDATE                        = 0x05
};

/* Stack timer */
#define CI_MAX_TIMER_NUM                        16
#define CI_TIMER_PARAM_ANY                    ((void *)0xffffffff)

/* CA PMT ES PID Management */
#define CI_VIDEO_INDEX                0
#define CI_AUDIO_INDEX                1
#define CI_TTX_INDEX                2
#define CI_SUBT_INDEX                3
#define CI_LAST_INDEX                3
#define CI_MAX_ES_NUM                32
struct ci_ca_es_pid_management
{
    UINT8 pid_num;
    UINT8 reserved[3];
    UINT16 pid[CI_MAX_ES_NUM];
};

/* CA PMT */
#define CA_PMT_BUF_LEN            1024
struct ci_ca_pmt {
    int length;
    UINT8 buffer[CA_PMT_BUF_LEN];            ///< XXX is there a specified maximum?
    int ca_descriptor;            /* Indicate the CA descriptor exist or not */

    struct ci_ca_es_pid_management es_pid;    //for filte the ES

    int ca_monitor_times;        //monitor retry times
    enum ci_ca_pmt_cmd_id cmd_id;

    enum ci_ca_pmt_list_management ca_pmt_list_management;

    UINT32 program_id;            // program id of the P_NODE

    BOOL force_resend;            //when video hold, can force resend ca_pmt
    UINT32 pmt_crc;
};

/* MMI Menu */
#define MAX_MENU_ENTRIES_NUM    64

struct ci_menu {
    struct ci_mmi *mmi;                    /* Point to MMI */
    int selectable;                        /* Menu list select-able */
    char *title_text;                    /* Title string */
    char *subtitle_text;                /* Subtitle string */
    char *bottom_text;                    /* Bottom string */
    char *entries[MAX_MENU_ENTRIES_NUM];/* Menu entries string list */
    int num_entries;                    /* Menu entries number */
};

/* MMI Enquiry */
struct ci_enquiry {
    struct ci_mmi *mmi;                    /* Point to MMI */
    char *text;                            /* Title string */
    int blind;                            /* 0: Clean; 1: Blind */
    int expected_length;                /* Input string length */
};

/* Service Type */
enum ci_service_type
{
    SERVICE_CI = 0,
    SERVICE_CI_PLUS = 1
};

/* uri message */
struct ci_uri_message
{
//    enum ci_service_type service_type;

    UINT16 program_number;
    UINT8 protocol_version;

    UINT8 aps_copy_control_info: 2;
    UINT8 emi_copy_control_info: 2;
    UINT8 ict_copy_control_info: 1;
    UINT8 rct_copy_control_info: 1;
    UINT8 reserved1: 2;

    UINT8 rl_copy_control_info: 6;
    UINT8 reserved2: 2;

    UINT8 reserved3[10];

    UINT8 uri_from_pvr: 1;
    UINT8 reserved4: 7;
/*    UINT8 uri_received: 1;
    UINT8 uri_cam_match: 1;
    UINT8 reserved4: 5;
*/
};    //total 16bytes

enum URI_STATE
{
    URI_CLEAR = 0,
    URI_NOT_RECEIVED = 1,
    URI_RECEIVED = 2,
};

/* Host Service Shunning */
enum ci_cam_type
{
    CICAM_DVB_NONE = 0,
    CICAM_DVB_CI = 1,
    CICAM_CI_PLUS = 2,
    CICAM_HD_PLUS = 3
};

enum ci_protect_mode
{
    CI_PROTECTED_IN_ACTIVE = 0,
    CI_PROTECTED_ACTIVE = 1
};

enum ci_host_key
{
    CIPLUS_KEY_AES_KEY = 0,
    CIPLUS_KEY_ROOT_CERT = 1,
    CIPLUS_KEY_BRAND_CERT = 2,
    CIPLUS_KEY_DEVICE_CERT = 3,
    CIPLUS_KEY_PRNG_SEED = 4,
    CIPLUS_KEY_PRNG_KEY_K = 5,
    CIPLUS_KEY_DH_P = 6,
    CIPLUS_KEY_DH_G = 7,
    CIPLUS_KEY_DH_Q = 8,
    CIPLUS_KEY_HDQ = 9,
    CIPLUS_KEY_SIV = 10,
    CIPLUS_KEY_SLK = 11,
    CIPLUS_KEY_CLK = 12,
    CIPLUS_KEY_AKH_DHSK = 13
};

/**************************** Section for stack main ************************/
enum api_ci_msg_type {
    /* Just for main thread work */
    API_MSG_NULL                = 0x00,    /* Nothing but make senssor process work */
    /* Followwing message for CI stack internal & notify APP (<Stack> -> APP) */
    API_MSG_CAMHW                = 0x11,    /* CAM status change(to CI) done */
    API_MSG_CAMIN                = 0x12,    /* CAM plug-in(to CI) stable(to app) */
    API_MSG_CAMOUT                = 0x13,    /* CAM pull-out(to CI) stable(to app) */
    API_MSG_CTC                    = 0x14,    /* Create TC(to CI) done(to app) */
    API_MSG_DTC                    = 0x15,    /* Delete TC(to CI) done(to app) */
    API_MSG_NEGO                = 0x16, /* Link layer negotiation(internal) */
    /* Followwing message only for API notify CI stack. (APP -> Stack) */
    API_MSG_CA_PMT                = 0x21,    /* Send CA PMT message */
    API_MSG_GET_APTYPE            = 0x22,    /* Get CAM application type */
    API_MSG_GET_APMANUFACTURER    = 0x23,    /* Get CAM application manufacturer ID */
    API_MSG_GET_MENUSTRING        = 0x24,    /* Get CAM menu string */
    API_MSG_GET_CASIDS            = 0x25,    /* Get CAM CAS IDs */
    API_MSG_ENTER_MENU            = 0x26,    /* Send a enter menu message */
    API_MSG_GET_MENU            = 0x27,    /* Get CAM menu structure */
    API_MSG_SELECT_MENU            = 0x28,    /* Select from current menu */
    API_MSG_CANCEL_MENU            = 0x29,    /* Cancel from current menu */
    API_MSG_GET_ENQUIRY            = 0x2A,    /* Get CAM enquiry structure */
    API_MSG_REPLY_ENQUIRY        = 0x2B,    /* Select from current enquiry */
    API_MSG_CANCEL_ENQUIRY        = 0x2C,    /* Cancel from current enquiry */

    API_MSG_GET_INITIAL_OBJECT  = 0x2D, /* APP MMI */
    API_MSG_REQUEST_START_ACK   = 0x2E, /* APP MMI */
    API_MSG_FILE_REQUEST        = 0x2F, /* APP MMI */
    API_MSG_GET_FILE_DATA       = 0x30, /* APP MMI */
    API_MSG_ABORT_REQUEST       = 0x31, /* APP MMI */
    API_MSG_ABORT_ACK           = 0x32, /* APP MMI */

    API_MSG_GET_URI_MSG         = 0x33, /* URI MSG */

    API_MSG_GET_CAMBID            = 0x34, /* Get CICAM Brand ID */
    API_MSG_SET_URI_MSG            = 0X35, /* Set URI MSG */

    /* Followwing message only for CI stack notify APP (Stack -> APP) */
    API_MSG_CAAPP_OK            = 0x41,    /* CAM is ready for CA_PMT(to app) */
    API_MSG_MENU_UPDATE            = 0x42,    /* New menu updated from CAM(to app) */
    API_MSG_ENQUIRY_UPDATE        = 0x43,    /* New enquiry updated from CAM(to app) */
    API_MSG_ENQUIRY_EXIT        = 0x44,    /* enquiry box exit(to app) */
    API_MSG_CAPMT_OK            = 0x45,    /* CAM PMT send OK(to app) */
    API_MSG_CAPMT_CASYSTEM_INVALID=0x46,/*CAM PMT send OK(to app) and ca_system_id is not support by this CI card */
    API_MSG_CI_NOT_DESCRAMBLED    = 0x47,    /* CAM descramble failed */

    API_MSG_CI_REQUEST_START    = 0x48,    /* APP MMI RequestStart */
    API_MSG_CI_FILE_RECEIVED    = 0x49,    /* APP MMI FileAcknowledge */
    API_MSG_CI_APP_ABORT        = 0x4A,    /* APP MMI AppAbortRequest */

//    API_MSG_URI_RECEIVED        = 0x4B, /* URI MSG received from CAM */

    API_MSG_SERVICE_SHUNNING_UPDATE    = 0x4C, /* SDT Service Shunning update */
    API_MSG_CI_ICT_ENABLE        = 0x4D, /* ICT Set TV Mode */
    API_MSG_CI_TUNE_SERVICE        = 0x4E, /* Tune Service for CAM Upgrade */
    API_MSG_ENABLE_ANALOG_OUTPUT= 0x4F, /* Enable analog output for Mxvgdby */
    API_MSG_TUNED_SEARCH_END    = 0x50, /* For CAMUP tp not in DB, search end notice */
    API_MSG_CAMUP_END_PLAY      = 0x51, /* For CAMUP end, then notice play */
    API_MSG_CAMUP_OK            = 0x52, /* For CAMUP OK, notice to app */
    API_MSG_ENABLE_YUV_OUTPUT   = 0x53, /* When no MG signal, enable YUV */
    API_MSG_MGCGMS_SET_VPO      = 0x54, /* Set VPO according to MG and CGMS-A */
    API_MSG_SET_MG_INFO= 0x55, /* Set VPO according to MG */
    API_MSG_SET_CGMS_INFO       = 0x56, /* Set VPO according to CGMS info */
    API_MSG_DISABLE_ANALOG_OUTPUT = 0x57, /* Disable analog output according to URI */
    API_MSG_DISABLE_YUV_OUTPUT  = 0x58, /* Disable YUV when have MG signal */
    API_MSG_RESUME_TV_MODE      = 0x59, /* Resume system TV mode when CAM plug out */
    API_MSG_CIPLUS_DSC_RESET    = 0x60,
    API_MSG_CI_FILE_OK           = 0x61,    /* APP MMI File Available and OK */
    API_MSG_CIPLUS_CC_CREATE    = 0X62, /* We know it's a CI Plus CAM now */

    /* Followwing message for between CI stack and APP (Stack <-> APP) */
    API_MSG_EXIT_MENU            = 0x81,    /* Exited the top level menu (MMI close) */
    API_MSG_CI_NOT_SELECTED        = 0x82    /* Not select CI card again */
};

#ifdef CI_PLUS_SUPPORT
#define CATEGORY_CODE_EUROPE 0x18/* URI default category code, ref "IEC 60958-3-2006: Table7" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Export function about CI/CAM */
INT32 api_ci_register(char *cic_name,
                      void (*cic_callback)(int slot, enum api_ci_msg_type type));
INT32 api_ci_unregister(char *cic_name);
int   api_ci_check_status(int slot);
int   api_ci_get_cam_type(int slot);
INT32 api_ci_register_callback(char *cic_name, void (*callback)(int percent));/* callback for CICAM upgrade progress */
INT32 api_ci_register_playch_callback(char *cic_name, void (*callback)(UINT32 prog_id)); /* Callback for channel play */
void api_ci_register_dac_reg(char *cic_name, void (*callback)());
void api_ci_register_tv_mode(char *cic_name, void (*callback)(enum tvsystem tvsys, BOOL b_progressive));
//void api_ci_plus_write_des_data();/* write CI+ data supporting DES in flash */
//void api_ci_plus_write_aes_data();/* write CI+ data supporting AES in flash */

/* Export function about CAM tune service */
UINT32 api_ci_get_tuned_prog (void);

/* Export function about CAM AI */
UINT8  api_ci_get_application_type(int slot);
UINT16 api_ci_get_application_manufacturer(int slot);
char  *api_ci_get_menu_string(void *buffer, int len, int slot);
UINT16 api_ci_get_cicam_brand_id(int slot);

/* Export function about CAM CAS */
UINT16 *api_ci_get_cas_ids(void *buffer, int len, int slot);
void    api_ci_send_pmt(int slot, UINT32 cmd);
void api_ci_not_selected(int slot);

/* Export function about PMT */
//void ci_pmt_create(UINT8 ca_pmt_cmd_id,UINT8 *ca_pmt_buff);
//void ci_pmt_create(UINT8 ca_pmt_cmd_id, UINT8 *ca_pmt_buff, struct ci_ca_pmt* pmt);

/* Export function about MMI */
void api_ci_enter_menu(int slot);
void api_ci_exit_menu(int slot);

/* Export function about menu */
struct ci_menu    *api_ci_get_menu(void *buffer, int len, int slot);
void   api_ci_menu_select(int slot, int n);
void   api_ci_menu_cancel(int slot);
char  *ci_menu_get_title_text(struct ci_menu *menu);
char  *ci_menu_get_subtitle_text(struct ci_menu *menu);
char  *ci_menu_get_bottom_text(struct ci_menu *menu);
char  *ci_menu_get_entry(struct ci_menu *menu, int n);
int    ci_menu_get_entries_num(struct ci_menu *menu);
int    ci_menu_get_selectable(struct ci_menu *menu);
void api_ci_set_mmi_visualstatus(INT8 status);
INT8 api_ci_get_mmi_enablestatus();

/* Export function about enquiry */
struct ci_enquiry *api_ci_get_enquiry(void *buffer, int len, int slot);
void   api_ci_enquiry_reply(int slot, char *s);
void   api_ci_enquiry_cancel(int slot);
char  *ci_enquiry_get_text(struct ci_enquiry *enquiry);
int    ci_enquiry_get_blind(struct ci_enquiry *enquiry);
int    ci_enquiry_get_expected_length(struct ci_enquiry *enquiry);

#ifdef CI_PLUS_SUPPORT

/* Export function about app mmi */
//buffer: OUT, AppDomainIdentifierLength[1] + AppDomainIdentifier[n] + InitialObjectLength[1] + InitialObject[n]
void *api_ci_get_initial_object(void *buffer, int len, int slot);

//ackcode: IN,
//0x01 OK
//0x02 Wrong API
//0x03 API busy
//0x80 to 0xFF Domain specific API busy
void api_ci_request_start_ack(UINT8 ackcode, int slot);

//requesttype: IN,
//File 0x00
//Data 0x01
void api_ci_file_request(UINT8 requesttype, UINT8 *data, int len, int slot);

//buffer: OUT, FileNameLength[1] + FileName[n] + FileDataLength[4] + FileData[n]
void *api_ci_get_file_data(void *buffer, int len, int slot);

//abortreqcode: IN,
//0x01 User Cancel
//0x02 System Cancel
void api_ci_app_abort_request(UINT8 abortreqcode, int slot);
void api_ci_app_abort_ack(UINT8 abortackcode, int slot);

//URI
//void *api_ci_get_uri_msg(void *buffer, int len, int slot);
//INT32 api_ci_set_uri_msg(void *buffer, int len, int slot);

//Service Shunning
#ifdef CI_PLUS_SERVICESHUNNING
enum ci_protect_mode ci_service_shunning(UINT32 tp_id, UINT16 service_id);
INT32 ci_start_sdt_filter(UINT32 tp_id);
INT32 ci_stop_sdt_filter();
UINT32 api_ci_get_sdt_service_count();
INT32 api_ci_get_sdt_service_list(UINT16 *service_list, UINT16 len);

#endif

//ICT Check
BOOL api_ict_control();
void api_uri_clear();
void api_uri_bak();
void api_uri_bak_resume();
void api_uri_bak_clear();
BOOL api_analog_output_enable();
//MG Check
BOOL api_mg_control();
void api_mg_cgms_reset();

//Descrambler functions
#ifdef CI_PLUS_NEW_CC_LIB
void ci_cc_dev_init(UINT8 slot);
RET_CODE ciplus_dsc_start(UINT8 slot);
RET_CODE ciplus_dsc_stop(UINT8 slot);
RET_CODE ciplus_dsc_close(UINT8 slot);
#else
RET_CODE ciplus_dsc_start();
RET_CODE ciplus_dsc_stop();
RET_CODE ciplus_dsc_close();
#endif

#endif

/* Export function about platform misc interface */
//when channel change, ci plus should not send any ca_pmt.
//so channel change need this mutex too.
void api_ci_mutex_lock();
void api_ci_mutex_unlock();

//1. mode, devices, ts route
void api_ci_set_mode(UINT8 mode);
void api_ci_set_dev(void *nim_dev, void *dmx_dev, UINT8 slot);
void api_set_nim_ts_type(UINT8 type, UINT8 slot_mask);
/*
 *  We may have some case: we need to bypass or pass the CAM
 *  directly, but we don't have CI device constructed, so we
 *  can't use api_set_nim_ts_type. This API could be a substitution
 *  of API api_set_nim_ts_type
 */
void api_set_ts_pass_cam(UINT8 pass, UINT8 slot_mask);

//2. cmd for descrambling
void api_send_ca_pmt(UINT8 slot);
void api_send_ca_pmt_ext(UINT8 slot, enum ci_ca_pmt_cmd_id cmd_id);
void cc_send_ca_pmt(UINT32 prog_id);

void api_ci_start_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask);
void api_ci_switch_aid(UINT32 prog_id, UINT16 pid);
void api_ci_stop_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask);


/*Function: search TP in DB
 *when tune service happened in CI+ CAM Upgrade Progress, we should find the given TP whether in DB
 *however the DB structure of DVB-S is different to DVB-C
 *so, we should search the TP in DB according to ft_type
 *api_ci_tuned_in_db() is not work anymore. we keep it just for compatibility consideration.
 *api_ci_tuned_in_db_by_ft_type() is provided for a replacement. We must use it when needed in the future.
*/
BOOL api_ci_tuned_in_db();
UINT32 api_ci_tuned_in_db_by_ft_type();


void api_ci_get_hc_id(UINT8 *slot, UINT16 *network_id, UINT16 *org_network_id, UINT16 *ts_id, UINT16 *service_id);

//io control cmd & api
enum ci_io_ctrl_cmd
{
    CI_IO_CTRL_NULL                     = 0,
    CI_IO_CTRL_PASS_CAM                    = 1,
    CI_IO_CTRL_BYPASS_CAM                = 2,
    CI_IO_CTRL_RESEND_CA_PMT             = 3,
    CI_IO_CTRL_SWITCH_AID                = 4,
    CI_IO_CTRL_SWITCH_TTX_PID             = 5,
    CI_IO_CTRL_SWITCH_SUBT_PID             = 6,
};
RET_CODE api_ci_io_ctrl(int slot, enum ci_io_ctrl_cmd cmd, UINT32 param);


//3. when video hold, send ca_pmt directly(for dm)
UINT32 api_ci_get_match_tp();
BOOL api_ci_camup_timeout();
BOOL api_ci_camup_in_progress();
UINT32 api_ci_set_cur_progid(int slot, UINT32 prog_id);

/*Function: search service in DB
 *when tune service happened in CI+ CAM Upgrade Progress, we should find the given service whether in DB
 *however the DB structure of DVB-S is different to DVB-C
 *so, we should search the service in DB according to ft_type
 *api_ci_get_prog_id() is not work anymore. we keep it just for compatibility consideration.
 *api_ci_get_prog_id_by_ft_type() is provided for a replacement. We must use it when needed in the future.
*/
UINT32 api_ci_get_prog_id(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id);
UINT32 api_ci_get_prog_id_by_ft_type(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id, UINT16 ft_type);



void api_ci_camup_answer_setstatus(BOOL can_answer);
BOOL api_ci_camup_answer_getstatus();
void api_ci_get_cgms_info(UINT8 *cgms, UINT8 *aps);
UINT8 api_ci_get_mg_info();
BOOL api_ci_get_dscpid_lst(UINT8 slot_mask, UINT16 *pid_lst, UINT8 *pid_num, UINT8 max_num);

/*
 *  @This API is for the CAM without ca info
 */
BOOL api_ci_cas_absent(int slot);

#ifdef __cplusplus
}
#endif

#endif /* __CI_PLUS_H__ */

