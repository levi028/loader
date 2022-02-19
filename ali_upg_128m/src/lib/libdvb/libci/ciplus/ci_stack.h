/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_stack.h
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack.
*    History:
*             Date                 Athor         Version          Reason
*      =========== ===    ====   ======   =============================
*    1.  Nov.10.2004      Justin Wu      Ver 0.1       Create file.
*    2.  Dec.16.2005      Justin Wu      Ver 0.2       Support timeout event.
*    3.  Dec.20.2005      Justin Wu      Ver 0.3       Support message protect.
*                                                   Spit head file.
*    4.    Feb.23.2010      Steven         Ver 0.4        Add new resources for CI+
*
*****************************************************************************/
#include <types.h>
#include <osal/osal.h>
#include <api/libci/ci_plus.h>

#ifndef __CI_STACK_H__
#define __CI_STACK_H__

#define CIPLUS_DEBUG_INFO_LINK        0
#define CIPLUS_DEBUG_INFO_TRANS    0
#define CIPLUS_DEBUG_INFO_SENSS    0
#define CIPLUS_DEBUG_INFO_APP        0
#define CIPLUS_DEBUG_INFO_IFX        0

#define LINK_PRINTF        PRINTF
#define TRAN_PRINTF        PRINTF
#define SENS_PRINTF        PRINTF
#define APPL_PRINTF        PRINTF
#define INFX_PRINTF        PRINTF


#define CI_DURATION_SENSS_PROCESS        100        /* Stack TS Polling Cycle */
#define CI_DURATION_CAM_MONITOR        300        /* CAM Exist Polling Cycle */
#define CI_DURATION_CAS_MONITOR        (CI_DURATION_SENSS_PROCESS * 40)    /* DSC Fail-Resend CA_PMT Cycle */
/* SMIT's irdeto cam card */
//change to 2s for Digisat's Digiturk CAM card
#define CI_DURATION_CAS_GUARD        (CI_DURATION_SENSS_PROCESS * 20)    /* Auto OK_SCRM Cycle */
#define CI_DURATION_CAS_RETRY        (CI_DURATION_SENSS_PROCESS * 5)    /* Com Fail-Resend CA_PMT Cycle */
#define CI_DURATION_MMI_ENTER        (CI_DURATION_SENSS_PROCESS * 60)    /* Enter Menu Timeout */
#define CI_DURATION_MMI_MONITOR    (CI_DURATION_SENSS_PROCESS * 80)    /* Operate Menu Timeout */
#define CI_DURATION_MMI_RETRY        (CI_DURATION_SENSS_PROCESS * 20)    /* Retry MMI Cycle */
#define CI_DURATION_MMI_GUARD        (CI_DURATION_SENSS_PROCESS * 5)    /* Exit MMI Delay */

#define CI_CPCI_QUERY_INTERVAL        2000    /* Multi-slot QUERY cmd interval */


#define CI_URI_MONITOR        10000            /* CI+ URI monitor */
#define CI_DURATION_CAMUP_GUARD     60000   /* CAM Upgrade monitor */


/* SMIT's irdeto CAM Card Need Always Resend CA_PMT */
#define CI_CAS_MONITOR_TIMERS        0x7FFFFFFF

/* APPMMI: CI+ Browser File */
//20100903, pls change this buffer size according Browser File Data Size
#define CI_MAX_FILE_DATA_LEN            0x10000//0x7000


/***************************** Section for Link Layer ***********************/
/* PCMCIA CIS Tuple Code */
enum pcmcia_cis_tuple_code
{
    CISTPL_NULL                 = 0x00,
    CISTPL_DEVICE                 = 0x01,
    CISTPL_LONGLINK_CB         = 0x02,
    CISTPL_INDIRECT             = 0x03,
    CISTPL_CONFIG_CB            = 0x04,
    CISTPL_CFTABLE_ENTRY_CB     = 0x05,
    CISTPL_LONGLINK_MFC         = 0x06,
    CISTPL_BAR                          = 0x07,
    CISTPL_PWR_MGMNT            = 0x08,
    CISTPL_EXTDEVICE                = 0x09,
    /* 0ah - 0fh reseved */
    CISTPL_CHECKSUM                 = 0x10,
    CISTPL_LONGLINK_A               = 0x11,
    CISTPL_LONGLINK_C            = 0x12,
    CISTPL_LINKTARGET               = 0x13,
    CISTPL_NO_LINK                  = 0x14,
    CISTPL_VERS_1                   = 0x15,
    CISTPL_ALTSTR                = 0x16,
    CISTPL_DEVICE_A                 = 0x17,
    CISTPL_JEDEC_C                  = 0x18,
    CISTPL_JEDEC_A                  = 0x19,
    CISTPL_CONFIG                = 0x1a,
    CISTPL_CFTABLE_ENTRY        = 0x1b,
    CISTPL_DEVICE_OC                = 0x1c,
    CISTPL_DEVICE_OA                = 0x1d,
    CISTPL_DEVICEGEO            = 0x1e,
    CISTPL_DEVICEGEO_A              = 0x1f,
    CISTPL_MANFID                   = 0x20,
    CISTPL_FUNCID                   = 0x21,
    CISTPL_FUNCE                = 0x22,
    CISTPL_SWIL                         = 0x23,
    /* 24 - 3fh reseved */
    CISTPL_VERS_2                   = 0x40,
    CISTPL_FORMAT                   = 0x41,
    CISTPL_GEOMETRY            = 0x42,
    CISTPL_BYTEORDER                = 0x43,
    CISTPL_DATE                         = 0x44,
    CISTPL_BATTERY                  = 0x45,
    CISTPL_ORG                    = 0x46,
    CISTPL_FORMAT_A                 = 0x47,
    /* 80h - 8fh Vendor unique tuples */
    CISTPL_CPCL                         = 0x90,
    /* 90h - feh Reseved */
    CISTPL_END                          = 0xff
};

enum pcmcia_cis_sub_tuple_code
{
    CCSTPL_CIF                = 0xc0
};

/* Define for Device Type Code Field */
enum pcmcia_cis_device_type_code
{
    DTYPE_NULL                = 0x00,
    DTYPE_ROM                = 0x01,
    DTYPE_OPTION            = 0x02,
    DTYPE_EPROM            = 0x03,
    DTYPE_EEPROM            = 0x04,
    DTYPE_FLASH            = 0x05,
    DTYPE_SRAM            = 0x06,
    DTYPE_DRAM            = 0x07,
    DTYPE_FUNCSPEC          = 0x0d,
    DTYPE_EXTEND            = 0x0e
};

/* Define for Device Speed Codes */
enum pcmcia_cis_device_speed_code
{
    DSPEED_NULL            = 0x00,
    DSPEED_250NS            = 0x01,
    DSPEED_200NS            = 0x02,
    DSPEED_150NS            = 0x03,
    DSPEED_100NS            = 0x04,
    DSPEED_EXT                = 0x07
};

struct cistpl_tuple
{
        UINT8                tuple_code;
        UINT8                tuple_link;
        UINT8                tuple_data_max;
        UINT8                tuple_data_len;
        UINT8                *tuple_data;
};

/* Structure for Other Device Information */
#define CISTPL_MAX_DEVICES            4

struct cistpl_device
{
        UINT8    ndev;
        struct
    {
        UINT8                type;
        UINT8                wp;
        UINT32                speed;
        UINT32                size;
        } dev[CISTPL_MAX_DEVICES];
};

struct cistpl_device_o
{
    UINT8                    mwait;
    UINT8                    vcc_used;
        struct cistpl_device        device;
};

/* Structure for Level 1 Version Information */
#define CISTPL_VERS_1_MAX_PROD_STRINGS    4

struct cistpl_vers_1
{
        UINT8    major;
        UINT8    minor;
        UINT8    ns;
        UINT8    ofs[CISTPL_VERS_1_MAX_PROD_STRINGS];
        char        str[254];
        UINT8    compatible;
};

/* Structure for Manufactory ID Information */
struct cistpl_manfid
{
        UINT16    manf;
        UINT16    card;
};

/* Structure for Config Information */
enum stci_ifn_code
{
    IFN_ZOOMVIDEO        = 0x0141,
    IFN_DVBCI            = 0x0241,
    IFN_OPENCABLEPOD    = 0x0341
};

#define CCSTPL_CIF_MAX_PROD_STRINGS        8

struct ccstpl_cif
{
    UINT32    ifn;
        UINT8    ns;
        UINT8    ofs[CCSTPL_CIF_MAX_PROD_STRINGS];
    char        str[128];
};

struct cistpl_config
{
        UINT8                last_idx;
        UINT32                base;
        UINT32                rmask[4];
        UINT8                subtuples;
        struct ccstpl_cif        cif_infor;
};

/* Structure for Config Table Entry Information */
#define CISTPL_POWER_VNOM        0
#define CISTPL_POWER_VMIN        1
#define CISTPL_POWER_VMAX        2
#define CISTPL_POWER_ISTATIC    3
#define CISTPL_POWER_IAVG        4
#define CISTPL_POWER_IPEAK        5
#define CISTPL_POWER_IDOWN    6

#define CISTPL_POWER_HIGHZ_OK        0x01
#define CISTPL_POWER_HIGHZ_REQ    0x02

struct cistpl_power
{
        UINT8    present;
        UINT8    flags;
        UINT32    param[7];
};

struct cistpl_timing
{
    UINT32    wait, waitscale;
    UINT32    ready, rdyscale;
    UINT32    reserved, rsvscale;
};

#define CISTPL_IO_LINES_MASK    0x1f
#define CISTPL_IO_8BIT            0x20
#define CISTPL_IO_16BIT            0x40
#define CISTPL_IO_RANGE            0x80

#define CISTPL_IO_MAX_WIN        16

struct cistpl_io
{
        UINT8    flags;
        UINT8    nwin;
        struct
    {
        UINT32    base;
        UINT32    len;
        } win[CISTPL_IO_MAX_WIN];
};

struct cistpl_irq
{
        UINT32    irqinfo1;
        UINT32    irqinfo2;
};

#define CISTPL_MEM_MAX_WIN    8

struct cistpl_mem
{
        UINT8    flags;
        UINT8    nwin;
        struct
    {
        UINT32    len;
        UINT32    card_addr;
        UINT32    host_addr;
        } win[CISTPL_MEM_MAX_WIN];
};

#define CISTPL_CFTABLE_DEFAULT        0x0001
#define CISTPL_CFTABLE_BVDS        0x0002
#define CISTPL_CFTABLE_WP            0x0004
#define CISTPL_CFTABLE_RDYBSY        0x0008
#define CISTPL_CFTABLE_MWAIT        0x0010
#define CISTPL_CFTABLE_AUDIO        0x0800
#define CISTPL_CFTABLE_READONLY    0x1000
#define CISTPL_CFTABLE_PWRDOWN    0x2000

struct cistpl_cftable_entry
{
        UINT8                    index;
        UINT16                    flags;
        UINT8                    interface;
        struct cistpl_power        vcc, vpp1, vpp2;
        struct cistpl_timing        timing;
        struct cistpl_io            io;
        struct cistpl_irq            irq;
        struct cistpl_mem            mem;
        UINT8                    subtuples;
};

//CI: 256
//#define CI_LINK_BUFFER_SIZE        0x100
//CI+: 1k ~64k, SMiT - 1k, SmarDTV - 512
#ifdef CI_PLUS_SUPPORT
#define CI_LINK_BUFFER_SIZE        0x400
#else
#define CI_LINK_BUFFER_SIZE        0x100
#endif
#define CI_LINK_M_FLAG            0x80

/* Slot */
struct ci_slot
{
    struct cic_device         *cic_dev;                    /* Driver Handle */

    enum ci_slot_state     state;                        /* Card IRQ Detect */

    UINT16                 buffer_size;                    /* Transfer Buffer Size */
    UINT8                 buffer[CI_LINK_BUFFER_SIZE];    /* Transfer Buffer */

    OSAL_ID             flag_id;                        /* Flags ID for this Slot */
    UINT8                 cam_present_status;            /* Used for Detect CAM Status in slot */
    int                     ca_pmt_cnt;                    /* CA_PMT Counter */

    /* Card Information Structure */
    struct cistpl_device_o        device_infor;
    struct cistpl_vers_1        version_infor;
    struct cistpl_manfid        manid_infor;
    struct cistpl_config        config_infor;
    struct cistpl_cftable_entry    cfgtable_infor;

    enum ci_cam_type cam_type;        /* CICAM_DVB_CI or CICAM_CI_PLUS */
    //int host_control_tune;            /* Host Control Tune Status */
    UINT8 akh_idx;                    /* index of AKH stored in flash */
    UINT32 app_info_type;            /* RSC_ID_APPLICATION_INFORMATION_V1 or RSC_ID_APPLICATION_INFORMATION_V3 */
};

/* Link Layer */
struct ci_link_layer
{
    struct ci_slot slot[CI_MAX_SLOT_NUM];    /* CI slot */
};

/************************** Section for Transport Layer *********************/
#define CI_MAX_TRANSCON_NUM        4
#ifdef CI_PLUS_SUPPORT
#define CI_MAX_TPDU_SIZE            CI_MAX_FILE_DATA_LEN    //APPMMI need 5.6k
#else
#define CI_MAX_TPDU_SIZE            0x800
#endif
#define CI_MAX_TPDU_DATA_SIZE        CI_MAX_TPDU_SIZE - 4

#define CI_DATA_INDICATOR            0x80

#define CI_MAX_CONNECT_RETRIES    2
#define CI_MAX_POLLING_RETRIES        2

enum ci_transport_tags        /* From CI Spec. */
{
    TRANS_TAG_SB                = 0x80,
    TRANS_TAG_RCV                = 0x81,
    TRANS_TAG_CREAT_TC        = 0x82,
    TRANS_TAG_CTC_REPLY        = 0x83,
    TRANS_TAG_DELETE_TC        = 0x84,
    TRANS_TAG_DTC_REPLY        = 0x85,
    TRANS_TAG_REQUEST_TC        = 0x86,
    TRANS_TAG_NEW_TC            = 0x87,
    TRANS_TAG_TC_ERROR        = 0x88,

    TRANS_TAG_DATA_LAST        = 0xa0,
    TRANS_TAG_DATA_MORE        = 0xa1
};

/* Transport layer state */
enum ci_transport_state        /* From CI Spec. */
{
    TRANS_STATE_IDLE,
    TRANS_STATE_CREATION,
    TRANS_STATE_ACTIVE,
    TRANS_STATE_DELETION
};

struct ci_tpdu
{
    int        size;
    union
    {
        UINT8    data[CI_MAX_TPDU_SIZE];
        struct
        {
            UINT8    slot;
            UINT8    tcid;
            UINT8    tag;
        } info;
    } tpdu;
};

/* Transport connect */
struct ci_transport_connect
{
    struct ci_link_layer *link;

    int slot;                        /* Current Slot */
    UINT8 tcid;                    /* Current Transport Connect ID */
    enum ci_transport_state state;    /* Current Transport Connect State */
    struct ci_tpdu tpdu;            /* Current TPDU */
    int last_response;            /* Current Response Tag or State */
    int data_available;            /* Current Data Available Flag */
    int trans_polling_cnt;            /* Current Transport Connect Polling Cnt */
    int trans_polling_fail_cnt;        /* Current Transport Connect Polling Fail Cnt */
};

/* Transport layer */
struct ci_transport_layer
{
    struct ci_link_layer         link;
    struct ci_transport_connect    connect[CI_MAX_TRANSCON_NUM];
};

/************************** Section for Session Layer **********************/
#define CI_MAX_SENSSCON_NUM    16
#ifdef CI_PLUS_SUPPORT
#define CI_MAX_SPDU_SIZE        CI_MAX_FILE_DATA_LEN    /* CI+ APPMMI need 5.6k */
#else
#define CI_MAX_SPDU_SIZE        0x400    /* This Size Affect CI Task Stack! */
#endif

enum ci_senssion_tags                /* From CI Spec. */
{
    SENSS_TAG_SESSION_NUMBER            = 0x90,
    SENSS_TAG_OPEN_SESSION_REQUEST        = 0x91,
    SENSS_TAG_OPEN_SESSION_RESPONSE    = 0x92,
    SENSS_TAG_CREATE_SESSION            = 0x93,
    SENSS_TAG_CREATE_SESSION_RESPONSE    = 0x94,
    SENSS_TAG_CLOSE_SESSION_REQUEST    = 0x95,
    SENSS_TAG_CLOSE_SESSION_RESPONSE    = 0x96
};

enum ci_senssion_status
{
    SENSS_STATUS_OK                        = 0x00,
    SENSS_STATUS_NOT_ALLOCATED            = 0xF0,
    SENSS_STATUS_UNAVAILABLE            = 0xF1,
    SENSS_STATUS_VERSION_LOW            = 0xF2,
    SENSS_STATUS_BUSY                    = 0xF3
};

/* Senssion connect */
struct ci_senssion_connect
{
    int scnb;
    int rscid;
    struct ci_transport_connect *tc;
    /* Application Private Data */
    void *appdata;
    /* Application Detach Callback */
    void (*appdelete)(struct ci_senssion_connect *sc);
    /* Application Object Callback */
    INT32 (*callback)(struct ci_senssion_connect *sc, int len, UINT8 *data);
};

/* Senssion layer */
struct ci_senssion_layer
{
    struct ci_transport_connect *tc;                    /* Current Process TC */
    struct ci_transport_layer    trans;
    struct ci_senssion_connect    connect[CI_MAX_SENSSCON_NUM];
};

/*********************** Section for Application Layer **********************/

enum ci_resource_identifier        /* From CI Spec. */
{
    RSC_ID_RESOURCE_MANAGER_V1            = 0x00010041,
    RSC_ID_RESOURCE_MANAGER_V2            = 0x00010042,
    RSC_ID_RESOURCE_MANAGER                = RSC_ID_RESOURCE_MANAGER_V1,
    RSC_ID_APPLICATION_INFORMATION_V1    = 0x00020041,
    RSC_ID_APPLICATION_INFORMATION_V2    = 0x00020042,
    RSC_ID_APPLICATION_INFORMATION_V3    = 0x00020043,    /*CI+*/
//    RSC_ID_APPLICATION_INFORMATION        = RSC_ID_APPLICATION_INFORMATION_V3,
    RSC_ID_CONDITIONAL_ACCESS_SUPPORT    = 0x00030041,
    RSC_ID_HD_PLUS                        = 0x00100041,
    RSC_ID_HOST_CONTROL                    = 0x00200041,
    RSC_ID_DATE_TIME                    = 0x00240041,
    RSC_ID_MMI                            = 0x00400041,
    RSC_ID_APPLICATION_MMI                = 0x00410041,    /*CI+*/
    RSC_ID_SMART_CARD_READER            = 0x00700041,
    RSC_ID_CONTENT_CONTROL                = 0x008C1001,    /*CI+*/
    RSC_ID_HOST_LANGUAGE_COUNTRY        = 0x008D1001,    /*CI+*/
    RSC_ID_CAM_UPGRADE                    = 0x008E1001,    /*CI+*/
    RSC_ID_CA_PVR                        = 0x00971001    /*CI+*/
};

/* Resource Management */
struct ci_resource_manager
{
    int state;
};

/* Application Information */
#define CI_TIME_BEFORE_ENTER_MENU  0    /* mS */

enum ci_application_type                /* From CI spec */
{
    APP_TYPE_NULL                        = 0x00,
    APP_TYPE_CA                        = 0x01,
    APP_TYPE_EPG                        = 0x02
};

struct ci_application_information
{
    int         state;
    UINT32     creation_time;
    UINT8     application_type;
    UINT16     application_manufacturer;
    UINT16     manufacturer_code;
    char     *menu_string;
};

/* CA PMT */
#define CI_MAX_CAS_NUM        64

enum ci_ca_enable
{
    CI_CE_DSCR_POSSIBLE                    = 0x01,
    CI_CE_DSCR_POSSIBLE_PD                = 0x02,
    CI_CE_DSCR_POSSIBLE_TD                = 0x03,
    CI_CE_DSCR_NOPOSSIBLE_NE                = 0x71,
    CI_CE_DSCR_NOPOSSIBLE_TR                = 0x73
};

struct UIR_DATA
{
    enum URI_STATE state;

    struct ci_uri_message uri_msg;
    UINT32 slot;
};

/* Conditional Access */
struct ci_conditional_access
{
    int                 state;
    int                 cas_num;
    unsigned short     cas_id[CI_MAX_CAS_NUM + 1]; /* List is Zero Terminated! */
};

/*HD Plus*/
struct ci_hd_plus
{
    UINT16 auth_protocol_id;
    UINT8 rsa_key_index;
    unsigned char session_key_seed_cicam[10];    //80
    unsigned char nonce[8];                        //64
    unsigned char copyright_cicam[195];            //264
};

/* Host Control */
#define CI_MAX_REPLACE_NUM    256
struct ci_host_control
{
    int         state;
    UINT16     old_nid;
    UINT16     old_onid;
    UINT16     old_tsid;
    UINT16     old_sid;
    UINT16     old_pid[CI_MAX_REPLACE_NUM];
};

/* Date and Time */
struct ci_date_time
{
    UINT8 interval;
};

enum ci_application_object_tags    /* From CI Spec. */
{
    APPOBJ_NONE                            = 0x000000,
    APPOBJ_TAG_PROFILE_ENQ                = 0x9F8010,
    APPOBJ_TAG_PROFILE                    = 0x9F8011,
    APPOBJ_TAG_PROFILE_CHANGE            = 0x9F8012,
    APPOBJ_TAG_APPLICATION_INFO_ENQ        = 0x9F8020,
    APPOBJ_TAG_APPLICATION_INFO            = 0x9F8021,
    APPOBJ_TAG_ENTER_MENU                = 0x9F8022,
    APPOBJ_TAG_REQUEST_CICAM_RESET        = 0x9F8023,    /*CI+*/
    APPOBJ_TAG_DATA_RATE_INFO            = 0x9F8024,    /*CI+*/
    APPOBJ_TAG_CA_INFO_ENQ                = 0x9F8030,
    APPOBJ_TAG_CA_INFO                    = 0x9F8031,
    APPOBJ_TAG_CA_PMT                    = 0x9F8032,
    APPOBJ_TAG_CA_PMT_REPLY                = 0x9F8033,
    APPOBJ_TAG_HDPLUS_AUTH_REQ            = 0x9F8200,
    APPOBJ_TAG_HDPLUS_AUTH_RESP            = 0x9F8201,
    APPOBJ_TAG_TUNE                        = 0x9F8400,
    APPOBJ_TAG_REPLACE                    = 0x9F8401,
    APPOBJ_TAG_CLEAR_REPLACE                = 0x9F8402,
    APPOBJ_TAG_ASK_RELEASE                = 0x9F8403,
    APPOBJ_TAG_DATE_TIME_ENQ            = 0x9F8440,
    APPOBJ_TAG_DATE_TIME                    = 0x9F8441,
    APPOBJ_TAG_CLOSE_MMI                    = 0x9F8800,
    APPOBJ_TAG_DISPLAY_CONTROL            = 0x9F8801,
    APPOBJ_TAG_DISPLAY_REPLY                = 0x9F8802,
    APPOBJ_TAG_TEXT_LAST                    = 0x9F8803,
    APPOBJ_TAG_TEXT_MORE                    = 0x9F8804,
    APPOBJ_TAG_KEYPAD_CONTROL            = 0x9F8805,
    APPOBJ_TAG_KEYPRESS                    = 0x9F8806,
    APPOBJ_TAG_ENQ                        = 0x9F8807,
    APPOBJ_TAG_ANSW                        = 0x9F8808,
    APPOBJ_TAG_MENU_LAST                    = 0x9F8809,
    APPOBJ_TAG_MENU_MORE                = 0x9F880A,
    APPOBJ_TAG_MENU_ANSW                = 0x9F880B,
    APPOBJ_TAG_LIST_LAST                    = 0x9F880C,
    APPOBJ_TAG_LIST_MORE                    = 0x9F880D,
    APPOBJ_TAG_SUBTITLE_SEGMENT_LAST    = 0x9F880E,
    APPOBJ_TAG_SUBTITLE_SEGMENT_MORE    = 0x9F880F,
    APPOBJ_TAG_DISPLAY_MESSAGE            = 0x9F8810,
    APPOBJ_TAG_SCENE_END_MARK            = 0x9F8811,
    APPOBJ_TAG_SCENE_DONE                = 0x9F8812,
    APPOBJ_TAG_SCENE_CONTROL            = 0x9F8813,
    APPOBJ_TAG_SUBTITLE_DOWNLOAD_LAST    = 0x9F8814,
    APPOBJ_TAG_SUBTITLE_DOWNLOAD_MORE    = 0x9F8815,
    APPOBJ_TAG_FLUSH_DOWNLOAD            = 0x9F8816,
    APPOBJ_TAG_DOWNLOAD_REPLY            = 0x9F8817,
    APPOBJ_TAG_COMMS_CMD                = 0x9F8C00,
    APPOBJ_TAG_CONNECTION_DESCRIPTOR    = 0x9F8C01,
    APPOBJ_TAG_COMMS_REPLY                = 0x9F8C02,
    APPOBJ_TAG_COMMS_SEND_LAST            = 0x9F8C03,
    APPOBJ_TAG_COMMS_SEND_MORE            = 0x9F8C04,
    APPOBJ_TAG_COMMS_RCV_LAST            = 0x9F8C05,
    APPOBJ_TAG_COMMS_RCV_MORE            = 0x9F8C06,
    APPOBJ_TAG_SMART_CARD_CMD            = 0x9F8E00,
    APPOBJ_TAG_SMART_CARD_REPLY            = 0x9F8E01,
    APPOBJ_TAG_SMART_CARD_SEND            = 0x9F8E02,
    APPOBJ_TAG_SMART_CARD_REV            = 0x9F8E03,

    APPOBJ_TAG_HOST_COUNTRY_ENQ            = 0x9F8100,    /* CI+: Host Language & Country */
    APPOBJ_TAG_HOST_COUNTRY                = 0x9F8101,
    APPOBJ_TAG_HOST_LANGUAGE_ENQ        = 0x9F8110,
    APPOBJ_TAG_HOST_LANGUAGE            = 0x9F8111,

    APPOBJ_TAG_CAM_FIRMWARE_UPGRADE        = 0x9F9D01,    /* CI+: CAM_Upgrade */
    APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_REPLY     = 0x9F9D02,
    APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_PROGRESS = 0x9F9D03,
    APPOBJ_TAG_CAM_FIRMWARE_UPGRADE_COMPLETE = 0x9F9D04,

    APPOBJ_TAG_REQUEST_START            = 0x9F8000,    /* Application MMI */
    APPOBJ_TAG_REQUEST_START_ACK        = 0x9F8001,
    APPOBJ_TAG_FILE_REQUEST                = 0x9F8002,
    APPOBJ_TAG_FILE_ACKNOWLEDGE            = 0x9F8003,
    APPOBJ_TAG_APP_ABORT_REQUEST        = 0x9F8004,
    APPOBJ_TAG_APP_ABORT_ACK            = 0x9F8005,

    APPOBJ_TAG_CC_OPEN_REQ                = 0x9F9001,    /* CI+: Content Control */
    APPOBJ_TAG_CC_OPEN_CNF                = 0x9F9002,
    APPOBJ_TAG_CC_DATA_REQ                = 0x9F9003,
    APPOBJ_TAG_CC_DATA_CNF                = 0x9F9004,
    APPOBJ_TAG_CC_SYNC_REQ                = 0x9F9005,
    APPOBJ_TAG_CC_SYNC_CNF                = 0x9F9006,
    APPOBJ_TAG_CC_SAC_DATA_REQ            = 0x9F9007,
    APPOBJ_TAG_CC_SAC_DATA_CNF            = 0x9F9008,
    APPOBJ_TAG_CC_SAC_SYNC_REQ            = 0x9F9009,
    APPOBJ_TAG_CC_SAC_SYNC_CNF            = 0x9F9010
};

/* Man Machine Interface */
enum ci_mmi_close_mmi_id
{
    CI_CMI_IMMEDIATE                                = 0x00,
    CI_CMI_DELAY                                    = 0x01
};

enum ci_mmi_display_control_command
{
    CI_DCC_SET_MMI_MODE                                = 0x01,
    CI_DCC_DISPLAY_CHARACTER_TABLE_LIST                = 0x02,
    CI_DCC_INPUT_CHARACTER_TABLE_LIST                = 0x03,
    CI_DCC_OVERLAY_GRAPHICS_CHARACTERISTICS        = 0x04,
    CI_DCC_FULL_SCREEN_GRAPHICS_CHARACTERISTICS    = 0x05
};

enum ci_mmi_mode
{
    CI_MM_HIGH_LEVEL                                    = 0x01,
    CI_MM_LOW_LEVEL_OVERLAY_GRAPHICS                = 0x02,
    CI_MM_LOW_LEVEL_FULL_SCREEN_GRAPHICS            = 0x03
};

enum ci_mmi_display_reply_id
{
    CI_DRI_MMI_MODE_ACK                                    = 0x01,
    CI_DRI_LIST_DISPLAY_CHARACTER_TABLES                = 0x02,
    CI_DRI_LIST_INPUT_CHARACTER_TABLES                    = 0x03,
    CI_DRI_LIST_GRAPHIC_OVERLAY_CHARACTERISTICS        = 0x04,
    CI_DRI_LIST_FULL_SCREEN_GRAPHIC_CHARACTERISTICS    = 0x05,
    CI_DRI_UNKNOWN_DISPLAY_CONTROL_CMD                = 0xF0,
    CI_DRI_UNKNOWN_MMI_MODE                            = 0xF1,
    CI_DRI_UNKNOWN_CHARACTER_TABLE                        = 0xF2
};

#define CI_ENQUIRY_FLAGS_BLIND                        0x01

#define CI_MMI_ANSWER_ID_CANCEL                    0x00
#define CI_MMI_ANSWER_ID_ANSWER                    0x01
#define CI_MMI_MENU_LEVEL_MAX                        1

struct ci_mmi
{
    struct ci_senssion_connect *sc;                            /* Point to App SC */
    struct ci_menu *menu_list[CI_MMI_MENU_LEVEL_MAX];    /* Menu Path List */
    int cur_menu;                                        /* Current Menu Number*/
    struct ci_enquiry *enquiry;                                /* Current Enquiry */
};

/* Host Control */
struct ci_host_tp_param{
    UINT16 network_id;
    UINT16 org_network_id;
    UINT16 ts_id;
    UINT16 service_id;
};

/* APP MMI */
enum ci_appmmi_request_type
{
    CI_APPMMI_REQ_TYPE_FILE                = 0x00,
    CI_APPMMI_REQ_TYPE_DATA                = 0x01,
};

//#define CI_MAX_FILE_DATA_LEN            0x7000    //APPMMI need 5.6k
struct ci_appmmi
{
    //RequestStart message
    UINT8 app_domain_identifier_length;
    UINT8 app_domain_identifier[256];
    UINT8 initial_object_length;
    UINT8 initial_object[256];

    //File
    UINT8 file_name_length;
    UINT8 file_name[256];

    UINT32 data_length;                    //FileDataLength
    UINT8 data[CI_MAX_FILE_DATA_LEN];    //FileData
};

struct ci_file_request
{
    UINT8 requesttype;
    int   len;
    UINT8 *data;
};

/**************************** Section for stack main ************************/


/* Stack Timer */
typedef void (* ci_timer_handler)(void *arg1, void *arg2);

struct ci_timer
{
      struct ci_timer *next;
      UINT32 time;
      ci_timer_handler h;
      void *arg1;
      void *arg2;
};

struct ci_timers
{
      struct ci_timer *next;
};

/* API Sync Flag */
enum api_ci_msg_flag_mask
{
    API_FLAG_APTYPE            = 0x001, /* Message Sync Flag for Get APTYPE */
    API_FLAG_APMANUFACTURER = 0x002, /* Message Sync Flag for Get APMANUFACTURER */
    API_FLAG_MENUSTRING        = 0x004, /* Message Sync Flag for Get MENUSTRING */
    API_FLAG_CASIDS            = 0x008, /* Message Sync Flag for Get CASIDS */
    API_FLAG_MENU                = 0x010, /* Message Sync Flag for Get MENU */
    API_FLAG_ENQUIRY            = 0x020, /* Message Sync Flag for Get ENQUIRY */
    API_FLAG_ENQUIRYRPLY        = 0x040, /* Message Sync Flag for Enquiry Replay */
    API_FLAG_SENDCAPMT        = 0x080,    /* Message Sync Flag for Send CA PMT */

    API_FLAG_INITIAL_OBJECT = 0x100,     /* Message Sync Flag for APP MMI get initial object */
    API_FLAG_FILE_REQUEST   = 0x200,     /* Message Sync Flag for APP MMI send file request cmd */
    API_FLAG_FILE_DATA      = 0x400,     /* Message Sync Flag for APP MMI get file data */

    API_FLAG_URI_MSG      = 0x800,         /* Message Sync Flag for get URI data */
    API_FLAG_CAMBID            = 0x1000,    /* Message Sync Flag for Get CICAM Brand ID */
    API_FLAG_SET_URI_MSG    = 0x2000    /* Message Sync Flag for set URI data */
};

/* API Message */
#define CI_MAX_MESSAGE_TIMEOUT                    1000

struct api_ci_msg
{
    enum api_ci_msg_type type;
    int         slot;
    UINT32     param;
};

#define CAM_NAME_MAX_LEN 256

/* Common structure */
struct ci_stack_st
{
    OSAL_ID thread_id;                /* CI stack task ID */
    OSAL_ID msgque_id;                /* CI stack message queue ID */
    OSAL_ID mutex_id;                /* CI stack function mutex ID, for CA_PMT */
    UINT32 cam_hw_active_flag;
    struct ci_timers ci_stack_timers;
    struct ci_timer ci_stack_timer[CI_MAX_TIMER_NUM];

    void (*callback)(int slot, enum api_ci_msg_type type);

    struct ci_senssion_layer senss;

    char cam_name[CI_MAX_SLOT_NUM][CAM_NAME_MAX_LEN];
};

#define CI_STATE_START    0x00000001
#define CI_STATE_STOP    0x0
struct ci_slot_control
{
    OSAL_ID ci_app_mutex;
    enum ci_slot_mode mode;

    struct ci_slot_info
    {
        // Device
        struct nim_device* nim_dev;
        struct dmx_device* dmx_dev;

        // CA_PMT
        UINT32 m_id;        /* SI Monitor ID */
        struct ci_ca_pmt ca_pmt;

        UINT8 audio_count;
        UINT8 ttx_count;
        UINT8 subt_count;
    }slot_info[CI_MAX_SLOT_NUM];

    OSAL_ID state;    //20101025 add for state sync between PUB & Stack

};

#define CI_NO_PROG_PASS -1
#ifdef __cplusplus
extern "C"
{
#endif

/* Link Layer Interface */
INT32 ci_link_attach(struct ci_link_layer *link);
INT32 ci_link_detach(struct ci_link_layer *link);
INT32 ci_link_cam_detect(struct ci_link_layer *link, int slot);
INT32 ci_link_cam_attach(struct ci_link_layer *link, int slot);
INT32 ci_link_cam_detach(struct ci_link_layer *link, int slot);
INT32 ci_link_negotiation(struct ci_link_layer *link, int slot);
INT32 ci_link_send_data(struct ci_link_layer *link, int slot, int len, UINT8 *data);
INT32 ci_link_recv_data(struct ci_link_layer *link, int slot, int *len, UINT8 *data);

/* Transport Layer Interface */
INT32 ci_trans_attach(struct ci_transport_layer *trans);
INT32 ci_trans_detach(struct ci_transport_layer *trans);
struct ci_transport_connect * ci_trans_process(struct ci_transport_layer *trans, int slot);
struct ci_transport_connect * ci_trans_open_conn(struct ci_transport_layer *trans, int slot);
INT32 ci_trans_close_conn(struct ci_transport_layer *trans, int tcid);
int   ci_trans_close_all_conn(struct ci_transport_layer *trans, int slot);
INT32 ci_trans_send_spdu(struct ci_transport_connect *tc, int len, UINT8 *data);
UINT8 *ci_trans_get_spdu(struct ci_transport_connect *tc, int *len);

/* Senssion Layer Interface */
void  ci_senss_conn_init(struct ci_senssion_connect *sc, int scnb, int rscid, struct ci_transport_connect *tc);
INT32 ci_senss_attach(struct ci_senssion_layer *senss);
INT32 ci_senss_detach(struct ci_senssion_layer *senss);
INT32 ci_senss_send_apdu(struct ci_senssion_connect *sc, int tag, int len, UINT8 *data);
UINT8 *ci_senss_get_apdu(struct ci_senssion_connect *sc, int *len, UINT8 *data);
struct ci_senssion_connect * ci_get_senss_by_rscid(struct ci_senssion_layer *senss, int rscid, int slot);
void  ci_message_process(struct ci_senssion_layer *senss, struct api_ci_msg *msg);
void ci_cas_watchdog(void *dummy1, void *dummy2);

/* Application Layer Interface */
int ci_get_apdu_tag(int *len, UINT8 **data);
struct ci_senssion_connect *ci_rm_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect *ci_ai_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect *ci_cas_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect *ci_hc_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect *ci_dat_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect *ci_mmi_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect * ci_scr_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);
/*CI+*/
struct ci_senssion_connect* ci_hlc_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect* ci_camup_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect* ci_appmmi_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc);
struct ci_senssion_connect* ci_cc_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc);
/*HD+*/
struct ci_senssion_connect *ci_hdplus_create(struct ci_senssion_connect *sc,
         int scnb, struct ci_transport_connect *tc);

/* Application Information */
UINT8  ci_ai_get_application_type(struct ci_senssion_connect *sc);
UINT16 ci_ai_get_application_manufacturer(struct ci_senssion_connect *sc);
char * ci_ai_get_menu_string(void *buffer, int len, struct ci_senssion_connect *sc);
INT32  ci_ai_enter_menu(struct ci_senssion_connect *sc);
UINT16 ci_cc_get_cicam_brand_id(struct ci_senssion_connect *sc);
/* Conditional Access */
UINT16 *ci_cas_get_ids(struct ci_senssion_connect *sc);
int ci_cas_get_ids_num(struct ci_senssion_connect *sc);
INT32   ci_cas_send_pmt(struct ci_senssion_connect *sc, UINT8 ca_pmt_cmd_id);
/* Host Control */
INT32  ci_hc_ask_release(struct ci_senssion_connect *sc);
/* Date and Time */
/* Man Machine Interface */
INT32  ci_mmi_close_mmi(struct ci_senssion_connect *sc);
int    ci_mmi_pop_menu(struct ci_mmi *mmi);
struct ci_menu *ci_mmi_get_menu(void *buffer, int len, struct ci_senssion_connect *sc);
struct ci_enquiry *ci_mmi_get_enquiry(void *buffer, int len, struct ci_senssion_connect *sc);
/* MMI - Menu */
INT32 ci_menu_select(struct ci_menu *menu, int index);
INT32 ci_menu_cancel(struct ci_menu *menu);
/* MMI - Enquiry */
INT32 ci_enquiry_reply(struct ci_enquiry *enquiry, char *s);
INT32 ci_enquiry_cancel(struct ci_enquiry *enquiry);
/* APP MMI */
INT32 ci_appmmiget_initial_object(void *buffer, int len, struct ci_senssion_connect *sc);
INT32 ci_appmmi_request_start_ack(struct ci_senssion_connect *sc, UINT8 ack_code);
INT32 ci_appmmi_file_request(struct ci_senssion_connect *sc,
                            UINT8 request_type, UINT8 *data, int len);
INT32 ci_appmmi_get_file_data(void *buffer, int len, struct ci_senssion_connect *sc);
INT32 ci_appmmi_send_abort_request(struct ci_senssion_connect *sc, UINT8 abort_req_code);
INT32 ci_appmmi_send_abort_ack(struct ci_senssion_connect *sc, UINT8 abort_req_code);

/* Internal Common */
UINT8 *ci_set_length(UINT8 *p, int len);
UINT8 *ci_get_length(UINT8 *p, int *len);

void api_ci_cam_to_stack(int slot);
void api_ci_msg_to_stack(int slot, int msg);
void api_ci_msg_to_app(int slot, int msg);

INT32 ci_set_timer(UINT32 time, ci_timer_handler h, void *arg1, void *arg2);
INT32 ci_del_timer(ci_timer_handler h, void *arg1, void *arg2);
void ci_mmi_watchdog(struct ci_senssion_connect *sc, int slot);
void ci_set_state(UINT32 state);


INT8 ci_get_mmi_visualstatus(void);
BOOL ci_is_av_scrambled(UINT8 slot);
UINT8 ci_cas_nim_get_lock(UINT8 slot);
RET_CODE  api_ci_send_allpmt(int slot);
BOOL ci_is_vdec_running(int slot);
void ci_set_mmi_enablestatus(INT8 status);
RET_CODE ci_set_cam_name(void *cam_name, UINT8 len, UINT8 slot);
UINT16 ntohs(UINT8 *data);
UINT32 htonl(UINT32 n);
UINT16 htons(UINT16 n);
UINT32 ntohl(UINT8 *data);

#ifdef __cplusplus
}
#endif

#endif /* __CI_STACK_H__ */

