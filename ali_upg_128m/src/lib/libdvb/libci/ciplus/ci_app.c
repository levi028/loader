/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_app.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack - application layer system management
*                     resources.
*    History:
*               Date                Athor            Version          Reason
*        ==========   ========   =======    =========    =================
*    1.    Nov.15.2004       Justin Wu       Ver 0.1          Create file.
*    2.    Oct.20.2009   Steven      fix Cyfrowy Polsat CAM reply 0xF3 mosaic issue.
*
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/decv/decv.h>

#include <api/libtsi/db_3l.h>
#include "ci_stack.h"

#ifdef HD_PLUS_SUPPORT
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#endif
#define MAX_CHARACTER_TABLE_BYTE_LEN        256
#define MMI_DISPLAY_REPLY_BUF_LEN            1024

struct graphic_characteristics
{
    UINT16     display_horizontal_size;
    UINT16     display_vertical_size;
    UINT8     aspect_ratio_information        :4;
    UINT8     graphics_relation_to_video    :3;
    UINT8     multiple_depths                :1;
    UINT32     display_bytes                :12;
    UINT32     composition_buffer_bytes    :8;
    UINT32     object_cache_bytes            :8;
    UINT32     number_pixel_depths        :4;
    struct graphic_characteristics_dprr
    {
        UINT8 display_depth        :3;
        UINT8 pixels_per_byte    :3;
        UINT8 reserved            :2;
        UINT8 region_overhead;
    } dprr[512];
};

struct ci_mmi_display_reply
{
    UINT8 id;
    UINT8 mode;
    struct graphic_characteristics graphic_characteristics;
    UINT8 characer_table_byte[MAX_CHARACTER_TABLE_BYTE_LEN];
    UINT8 buffer[MMI_DISPLAY_REPLY_BUF_LEN];
};

/* Modules which don't have CAS session */
struct ci_cam_cas_info
{
    char *cam_name;
    UINT8 cas_present;
};

#define CI_CAM_NO_CAS_SENSS 10
static struct ci_cam_cas_info cas_info[CI_CAM_NO_CAS_SENSS] =
{
    {"Air CAM", 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0}
};

/* CA_PMT Buffer */
static struct ci_ca_pmt ca_pmt[CI_MAX_SLOT_NUM];

/* Service Function */
static char* ci_copy_string(int len, UINT8 *data)
///< Copy the String at Data.
///< Return a Pointer to a Newly Allocated String.
{
    char *s = osal_memory_allocate(len + 1);
    if (NULL == s)
        return NULL;
    MEMCPY(s, data, len);
    s[len] = 0;
    return s;
}

static char *ci_get_string(int *len, UINT8 **data)
///< Get String at Data.
///< Return a Pointer to a Newly Allocated String, or NULL in case of Error.
///< Upon Return Length and Data Represent the Remaining Data after the String has been Skipped.
{
    UINT8 *d;
    char *s;
    int l = 0;

    if (*len > 0 && data && *data)
    {
        d = ci_get_length(*data, &l);
        if (NULL == d)
            return NULL;
        s = ci_copy_string(l, d);
        *len -= d - *data + l;
        *data = d + l;
        return s;
    }
    return NULL;
}

//static int ci_get_apdu_tag(int *len, UINT8 **data)
int ci_get_apdu_tag(int *len, UINT8 **data)
///< Get Tag at Data.
///< Return the Actual Tag, or AOT_NONE in case of Error.
///< Upon Return Length and Data Represent Remaining Data after Tag has been Skipped.
{
    int t = 0, i;

    if (*len >= 3 && data && *data)
    {
        for (i = 0; i < 3; i++)
        {
            t = (t << 8) | *(*data)++;
        }
        *len -= 3;
        return t;
    }
    return APPOBJ_NONE;
}

/* Resource Manager Resource */
static void ci_rm_delete(struct ci_senssion_connect *sc);
static INT32 ci_rm_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

/* Create Resource Manager Session Connection */
struct ci_senssion_connect* ci_rm_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_resource_manager *tp;
    ci_senss_conn_init(sc, scnb, RSC_ID_RESOURCE_MANAGER, tc);
    sc->appdelete = ci_rm_delete;
    sc->callback = ci_rm_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_resource_manager))) == NULL)
    {
        APPL_PRINTF("ci_rm_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_resource_manager*)(sc->appdata);
    tp->state = 0;

    //set CICAM-type
    sc->tc->link->slot[sc->tc->slot].cam_type = CICAM_DVB_CI;

    return sc;
}

/* Delete Resource Manager Session Connection */
static void ci_rm_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

/* Process for Resource Manager Session */
static INT32 ci_rm_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_resource_manager *tp = (struct ci_resource_manager *)(sc->appdata);
    int resources[] = { htonl(RSC_ID_RESOURCE_MANAGER),
                        //htonl(RSC_ID_RESOURCE_MANAGER_V2),
                        //htonl(RSC_ID_APPLICATION_INFORMATION),
                        htonl(sc->tc->link->slot[sc->tc->slot].app_info_type),
                        //htonl(RSC_ID_APPLICATION_INFORMATION_V3),    /*CI+*/
                        htonl(RSC_ID_CONDITIONAL_ACCESS_SUPPORT),
                        htonl(RSC_ID_HOST_CONTROL),
                        htonl(RSC_ID_DATE_TIME),
                        htonl(RSC_ID_MMI),
#ifdef CI_PLUS_SUPPORT
                        htonl(RSC_ID_APPLICATION_MMI),
                        htonl(RSC_ID_CONTENT_CONTROL),    /*CI+*/
                        htonl(RSC_ID_HOST_LANGUAGE_COUNTRY),    /*CI+*/
                        htonl(RSC_ID_CAM_UPGRADE),    /*CI+*/
#endif
#ifdef HD_PLUS_SUPPORT
                        htonl(RSC_ID_HD_PLUS),/*HD+*/
#endif
                        };
    int tag, l, i;
    UINT8 *d;

#ifndef CI_PLUS_NO_COMPATIBLE_INFO
    if(sc->tc->link->slot[sc->tc->slot].version_infor.compatible == 1)
        sc->tc->link->slot[sc->tc->slot].app_info_type = RSC_ID_APPLICATION_INFORMATION_V3;
    else
        sc->tc->link->slot[sc->tc->slot].app_info_type = RSC_ID_APPLICATION_INFORMATION_V1;
#else
    sc->tc->link->slot[sc->tc->slot].app_info_type = RSC_ID_APPLICATION_INFORMATION_V3;
#endif
    resources[1] = htonl(sc->tc->link->slot[sc->tc->slot].app_info_type);

    if(NULL == tp)
    {
        APPL_PRINTF("ci_rm_process: Resource Manager Data Null --- Error!\n");
        return ERR_FAILUE;
    }

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_PROFILE_ENQ:        /* Profile Enquiry */
            APPL_PRINTF("ci_rm_process: %d: <== Profile Enquiry\n", sc->scnb);
            APPL_PRINTF("ci_rm_process: %d: ==> Profile\n", sc->scnb);
            // Profile Reply (Host -> CAM)
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_PROFILE, sizeof(resources), (UINT8 *)resources) != SUCCESS)
            {
                APPL_PRINTF("ci_rm_process: Send APPOBJ_TAG_PROFILE failue\n");
                return ERR_FAILUE;
            }
            tp->state = 3;
                    break;
        case APPOBJ_TAG_PROFILE:            /* Profile Reply */
            APPL_PRINTF("ci_rm_process: %d: <== Profile\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            for (i = (l>>2); i > 0; i--, d += 4)
            {
                APPL_PRINTF("ci_rm_process: resource id %08x\n", ntohl(d));
            }
            APPL_PRINTF("ci_rm_process: %d: ==> Profile Change\n", sc->scnb);
            // Profile Changed (Host -> CAM)
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_PROFILE_CHANGE, 0, NULL) != SUCCESS)
            {
                APPL_PRINTF("ci_rm_process: Send APPOBJ_TAG_PROFILE_CHANGE failue\n");
                return ERR_FAILUE;
            }
            tp->state = 2;
            break;
            case APPOBJ_TAG_PROFILE_CHANGE:    /* Profile Changed */
            APPL_PRINTF("ci_rm_process: %d: <== Profile Change\n", sc->scnb);
            APPL_PRINTF("ci_rm_process: %d: ==> Profile Enq\n", sc->scnb);
            // Profile Enquiry (Host -> CAM)
            if (ci_senss_send_apdu(sc, APPOBJ_TAG_PROFILE_ENQ, 0, NULL) != SUCCESS)
            {
                APPL_PRINTF("ci_rm_process: Send APPOBJ_TAG_PROFILE_ENQ failue\n");
                return ERR_FAILUE;
            }
            break;
        default:
            APPL_PRINTF("ci_rm_process: CI resource manager: unknown tag %06X\n", tag);
            return ERR_FAILUE;
           }
    }
    else if (tp->state == 0)
    {
        APPL_PRINTF("ci_rm_process: %d: ==> Profile Enq\n", sc->scnb);
        // Profile Enquiry (Host -> CAM)
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_PROFILE_ENQ, 0, NULL) != SUCCESS)
        {
            APPL_PRINTF("ci_rm_process: Send APPOBJ_TAG_PROFILE_ENQ failue\n");
            return ERR_FAILUE;
        }
        tp->state = 1;
    }
    return SUCCESS;
}

/* Application Information Resource */
static void ci_ai_delete(struct ci_senssion_connect *sc);
static INT32 ci_ai_process(struct ci_senssion_connect *sc, int len, UINT8 *data);
static void ci_ai_data_rate_info(struct ci_senssion_connect *sc, UINT8 data_rate);

/* Create Application Information Session Connection */
struct ci_senssion_connect * ci_ai_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_application_information *tp;

    ci_senss_conn_init(sc, scnb, tc->link->slot[tc->slot].app_info_type, tc);
    sc->appdelete = ci_ai_delete;
    sc->callback = ci_ai_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_application_information))) == NULL)
    {
        APPL_PRINTF("ci_ai_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_application_information *)(sc->appdata);
    tp->state = 0;
    tp->application_type = 0;
    tp->application_manufacturer = 0;
    tp->manufacturer_code = 0;
    tp->creation_time = osal_get_tick();
    tp->menu_string = NULL;
    return sc;
}

/* Delete Application Information Session Connection */
static void ci_ai_delete(struct ci_senssion_connect *sc)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);

    if(tp != NULL)
    {
        if (tp->menu_string)
        {
            osal_memory_free(tp->menu_string);
            tp->menu_string = NULL;
        }
    }
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

extern INT32 ci_reset_cam(struct ci_link_layer *link, int slot);
extern int _strncasecmp(const char *cs, const char *ct, int count);
/* Process for Application Information Session */
static INT32 ci_ai_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);
    int tag, l;
    UINT8 *d;
    UINT8 i;

    if(NULL == tp)
    {
        APPL_PRINTF("ci_ai_process: Application Information Data Null --- Error!\n");
        return ERR_FAILUE;
    }

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_APPLICATION_INFO:        /* Application Information */
            APPL_PRINTF("ci_ai_process: %d: <== Application Info\n", sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if ((l -= 1) < 0)
            {
                break;
            }
            tp->application_type = *d++;            /* Application Type */
            if ((l -= 2) < 0)
            {
                break;
            }
            tp->application_manufacturer = ntohs(d);    /* Application Manufacturer */
            d += 2;
            if ((l -= 2) < 0)
            {
                break;
            }
            tp->manufacturer_code = ntohs(d);        /* Manufacturer Code */
            d += 2;
            if (tp->menu_string)
            {
                osal_memory_free(tp->menu_string);
            }
            tp->menu_string = ci_get_string(&l, &d);    /* Menu String */
            if (NULL == tp->menu_string)
                break;
            APPL_PRINTF("ci_ai_process: %s, %02X, %04X, %04X\n", tp->menu_string, tp->application_type, tp->application_manufacturer, tp->manufacturer_code);//XXX make externally accessible!
            tp->state = 2;

            ci_set_cam_name(tp->menu_string, STRLEN(tp->menu_string), sc->tc->slot);

            /* CI+: data_rate_info */
            if (sc->tc->link->slot[sc->tc->slot].app_info_type == RSC_ID_APPLICATION_INFORMATION_V3)
                ci_ai_data_rate_info(sc, CI_DATA_RATE>72 ? 1 : 0);

#ifndef CI_PLUS_SUPPORT
            for (i = 0; i < CI_CAM_NO_CAS_SENSS; i++)
            {
                if (cas_info[i].cam_name
                    && 0 == _strncasecmp(cas_info[i].cam_name, tp->menu_string, STRLEN(cas_info[i].cam_name))
                    && !cas_info[i].cas_present)
                {
                    APPL_PRINTF("Special CAM: no CAS session available :(\n");
                    sc->tc->link->slot[sc->tc->slot].state = CAM_STACK_ATTACH;
                    api_ci_msg_to_app(sc->tc->slot, API_MSG_CAAPP_OK);
                }
            }
#endif

            //Are you sure this could be work ? I don't know, so I add the upper part
            //<<< for smit compunicate/Conax cards have no ca_info
            /* Notify Application of CAM Init Finished (Not According to CI Spec.) */
            sc->tc->link->slot[sc->tc->slot].state = CAM_STACK_ATTACH;
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CTC);
            //<<<

            break;
        case APPOBJ_TAG_REQUEST_CICAM_RESET:    /* CI+: request_cicam_reset command */
            APPL_PRINTF("ci_ai_process: request_cicam_reset command\n");
            //TO DO...
            if(ci_reset_cam(sc->tc->link, sc->tc->slot) != SUCCESS)
            {
                APPL_PRINTF("ci_ai_process: REQUEST_CICAM_RESET failed!\n");
            }
            break;
        default:
            APPL_PRINTF("ci_ai_process: CI application information: unknown tag %06X\n", tag);
            return ERR_FAILUE;
        }
    }
    else if (tp->state == 0)
    {
        // Application Info Enquiry (Host -> CAM)
        APPL_PRINTF("ci_ai_process: %d: ==> Application Info Enq\n", sc->scnb);
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_APPLICATION_INFO_ENQ, 0, NULL) != SUCCESS)
        {
            APPL_PRINTF("ci_ai_process: Send APPOBJ_TAG_APPLICATION_INFO_ENQ failue\n");
            return ERR_FAILUE;
        }
        tp->state = 1;
    }
    return SUCCESS;
}

/* Get Application Type */
UINT8 ci_ai_get_application_type(struct ci_senssion_connect *sc)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);

    return tp->application_type;
}

/* Get Application Manufacturer */
UINT16 ci_ai_get_application_manufacturer(struct ci_senssion_connect *sc)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);

    return tp->application_manufacturer;
}

/* Get Application Menu String */
char * ci_ai_get_menu_string(void *buffer, int len, struct ci_senssion_connect *sc)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);
    int l;

    if(NULL == tp)
    {
        APPL_PRINTF("ci_ai_get_menu_string: Application Information Data Null --- Error!\n");
        return NULL;
    }

    l = STRLEN(tp->menu_string) + 1;
    if (l < len)
    {
        MEMSET(buffer, 0, l);
        //STRCPY(buffer, tp->menu_string);
        strncpy(buffer, tp->menu_string,l-1);
        return buffer;
    }
    return NULL;
}

/* Enter Application Information Menu (Send Enter Menu APDU to CAM)*/
INT32 ci_ai_enter_menu(struct ci_senssion_connect *sc)
{
    struct ci_application_information *tp = (struct ci_application_information *)(sc->appdata);

    if(NULL == tp)
    {
        APPL_PRINTF("ci_ai_get_enter_menu: Application Information Data Null --- Error!\n");
        return ERR_FAILUE;
    }

/* The Irdeto AllCAM 4.7 (and maybe others, too) does not react on APPOBJ_TAG_ENTER_MENU
 * during the first few seconds of a newly established connection
 */
    if (tp->state == 2 && (osal_get_tick() - tp->creation_time > CI_TIME_BEFORE_ENTER_MENU))
    {
        // Send Enter Menu APDU to CAM
        APPL_PRINTF("ci_ai_enter_menu: %d: ==> Enter Menu\n", sc->scnb);
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_ENTER_MENU, 0, NULL) == SUCCESS)
        {
            APPL_PRINTF("ci_ai_enter_menu: Send APPOBJ_TAG_ENTER_MENU success! \n");
            return SUCCESS;
        }
        APPL_PRINTF("ci_ai_enter_menu: Send APPOBJ_TAG_ENTER_MENU failue\n");
    }
    return ERR_FAILUE;
}

/* CI+: data_rate_info */
/* data_rate: IN, 00 72 Mbit/s, 01 96 Mbit/s */
static void ci_ai_data_rate_info(struct ci_senssion_connect *sc,UINT8 data_rate)
{
    APPL_PRINTF("ci_ai_data_rate_info: data rate - %d\n",data_rate);
    if (ci_senss_send_apdu(sc, APPOBJ_TAG_DATA_RATE_INFO, 1, &data_rate) != SUCCESS)
    {
        APPL_PRINTF("ci_ai_data_rate_info: Send APPOBJ_TAG_DATA_RATE_INFO failue\n");
    }
}

/* Conditional Access Support Resource */
static void ci_cas_delete(struct ci_senssion_connect *sc);
static INT32 ci_cas_process(struct ci_senssion_connect *sc, int len, UINT8 *data);
void ci_cas_monitor(struct ci_senssion_connect *sc, int *times);

/* Create CA Support Session Connection */
struct ci_senssion_connect * ci_cas_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_conditional_access *tp;

    ci_senss_conn_init(sc, scnb, RSC_ID_CONDITIONAL_ACCESS_SUPPORT, tc);
    sc->appdelete = ci_cas_delete;
    sc->callback = ci_cas_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_conditional_access))) == NULL)
    {
        APPL_PRINTF("ci_cas_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_conditional_access*)(sc->appdata);
    tp->state = 0;
    tp->cas_num = 0;
    tp->cas_id[0] = 0;

    sc->tc->link->slot[sc->tc->slot].ca_pmt_cnt = 0;;

    return sc;
}

/* Delete CA Support Session Connection */
static void ci_cas_delete(struct ci_senssion_connect *sc)
{
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
    // Delete Timer of Send CA_PMT for Descramble
    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING));
    // Delete Timer of CA Support Monitor
    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
}

/* Process for CA Support Session */
static INT32 ci_cas_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_conditional_access *tp = (struct ci_conditional_access *)(sc->appdata);
    int tag, l;
    UINT16 id;
    UINT8 *d;
    UINT8 i;
    char module_name[33];

    if(NULL == tp)
    {
        APPL_PRINTF("ci_cas_process[%d]: %d: CAS Data Null --- Error!\n", sc->tc->slot, sc->scnb);
        return ERR_FAILUE;
    }

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_CA_INFO:        /* CA Info. */
            APPL_PRINTF("ci_cas_process[%d]: %d: <== Ca Info\n", sc->tc->slot, sc->scnb);
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            // Get CA System IDs
            while (l > 1)
            {
                id = ((UINT8)(*d) << 8) | *(d + 1);
                APPL_PRINTF("ci_cas_process: %04X\n", id);
                d += 2;
                l -= 2;
                if (tp->cas_num < CI_MAX_CAS_NUM)
                {
                    tp->cas_id[tp->cas_num++] = id;
                    tp->cas_id[tp->cas_num] = 0;
                }
                else
                {
                    APPL_PRINTF("ci_cas_process[%d]: too many CA system IDs!\n", sc->tc->slot);
                }
            }
            tp->state = 2;

            /* Fulan CYFRA+ cam Ca Info -> Application Info. 20090519 */
            sc->tc->link->slot[sc->tc->slot].state = CAM_STACK_ATTACH;
            /* Notify Application of CAM is Ready for CA_PMT */
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CAAPP_OK);

#ifndef CI_PLUS_SUPPORT
            // We can't invoke this function here
            // It may cause crash when get menu string
            api_ci_get_menu_string(module_name, 33, sc->tc->slot);

            for (i = 0; i < CI_CAM_NO_CAS_SENSS; i++)
            {
                if (cas_info[i].cam_name
                    && 0 == _strncasecmp(cas_info[i].cam_name, module_name, STRLEN(cas_info[i].cam_name))
                    && !cas_info[i].cas_present)
                    cas_info[i].cas_present = 1;
             }
#endif

            break;
        case APPOBJ_TAG_CA_PMT_REPLY:    /* CA PMT Reply */
            APPL_PRINTF("ci_cas_process[%d]: %d: <== CA_PMT reply\n", sc->tc->slot, sc->scnb);
            /* This CAM support reply, we remove sending OK_DESCRABLING timeout handler */
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 3)
            {
                int i;
                UINT16 program_number;
                int ca_enable_flag; /* Bit0: Program Level; Bit1: ES Level; Bit7: ES Level Exist Flag */
                UINT8 reply = 0;

                for (i = 0; i < l; i++)
                {
                    APPL_PRINTF("0x%02x ", d[i]);
                }
                APPL_PRINTF("\n");

                program_number = (d[0] << 8) | d[1];    /* Program Number */

                ca_enable_flag = 0;
                /* Check Program Level Flag */
//                if ((d[3] & 0x80) == 0x80)     /* for Digisat's Irdeto(& black Viaccess) CAM card, d[3] = 0xF1(0xF3) */
//                if ((d[3] & 0x80) == 0x80
//                    && (d[3] & 0x7f) != 0x71/*no entitlement*/)
                if ((d[3] & 0xf0) == 0x80)    /* for SKY Cyfrowy Polsat CAM card, d[3] = 0xF3, and no need send ca pmt, or it has mosaic */
                {
                    ca_enable_flag = 0x01;
                }
                reply = d[3] & 0x7f;
                l -= 4;
                d += 4;
                while (l > 2)    /* Check ES Level Flag, Higher Priority than Program Level */
                {
                    if (d[2] & 0x80)        /* CA Flag Exist or Not */
                    {
                        ca_enable_flag |= 0x80;
                    //}
                    //if ((d[2] & 0xf0) == 0x80) //for SMT SmartDTV Nagra vision cam sometime reply 0xF1
                    //{
                        ca_enable_flag |= 0x02;
                        break;
                    }
                    l -= 3;
                    d += 3;
                }
                /* Remove CA_PMT GUARD Timer */
                while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING));

                /* If ES Level Exist and OK, or ES Level Not Exist and Program Level OK, Send Descrambling Command */
                if ((ca_enable_flag & 0x02) || (ca_enable_flag & 0x01))
                {
                    //If Reply's Program Number is Current ca_pmt's Program Number, do Send ca_pmt
                    //else Ignore this Reply.
                    if (program_number == ((ca_pmt[sc->tc->slot].buffer[1]<<8)|ca_pmt[sc->tc->slot].buffer[2]))
                    {
                        ci_cas_send_pmt(sc, CI_CPCI_OK_DESCRAMBLING);
                    }
                    else
                    {
                        APPL_PRINTF("ci_cas_process[%d]: reply program number %X != current ca_pmt program number %X !!\n", sc->tc->slot,program_number,((ca_pmt[sc->tc->slot].buffer[1]<<8)|ca_pmt[sc->tc->slot].buffer[2]));
                    }
                }
                else
                {
                    /* for Digisat's Irdeto(& black Viaccess) CAM card, d[3] = 0xF1(0xF3) */
                    if (reply == 0x71 || reply == 0x73)
                    {
                        APPL_PRINTF("ci_cas_process[%d]: set Descramble timer for reply 0x%X\n", sc->tc->slot,reply);
                        //if CAM still no descramble program, do send ca pmt
                        ci_set_timer(CI_DURATION_CAS_GUARD, (ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING);
                    }
                    APPL_PRINTF("ci_cas_process[%d]: reply ca_enable_flag %X\n", sc->tc->slot,ca_enable_flag);
                }
            }
            else
            {
                APPL_PRINTF("ci_cas_process[%d]: l %d <= 3\n", sc->tc->slot,l);
            }
            break;
        default:
            APPL_PRINTF("ci_cas_process[%d]: CI conditional access support: unknown tag %06X\n", sc->tc->slot, tag);
            return ERR_FAILUE;
        }
    }
    else if (tp->state == 0)
    {
        // CA Info Enquiry (Host -> CAM)
        APPL_PRINTF("ci_cas_process[%d]: %d: ==> Ca Info Enq\n", sc->tc->slot, sc->scnb);
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_CA_INFO_ENQ, 0, NULL) != SUCCESS)
        {
            APPL_PRINTF("ci_cas_process[%d]: Send APPOBJ_TAG_CA_INFO_ENQ failue\n", sc->tc->slot);
            return ERR_FAILUE;
        }
        tp->state = 1;
    }
    return SUCCESS;
}

/* Get CA System IDs */
UINT16* ci_cas_get_ids(struct ci_senssion_connect *sc)
{
    struct ci_conditional_access *tp = (struct ci_conditional_access*)(sc->appdata);

    return tp->cas_id;
}

/* Get Number of CA System IDs */
int ci_cas_get_ids_num(struct ci_senssion_connect *sc)
{
    struct ci_conditional_access *tp = (struct ci_conditional_access*)(sc->appdata);

    return tp->cas_num;
}

/*for Viaccess issue*/
static BOOL ci_cam_check(int slot, char* patch_module)
{
    char module_name[256];

    api_ci_get_menu_string(module_name, 256, slot);

#ifndef _BUILD_OTA_E_
    if (_strncasecmp(module_name, patch_module, STRLEN(patch_module)) == 0)
    {
        return TRUE;
    }
#endif

    return FALSE;
}

/* CA System Monitor: if NIM Lock but AV still Scrambled, then Send CA_PMT to CAM again */
void ci_cas_monitor(struct ci_senssion_connect *sc, int *times)
{
    /* If MMI Visual Status Enable, then Set Timer for Next Monitor, not Check Stream Scrambled Status
      * and Send CA_PMT to CAM again */
    if(1 == ci_get_mmi_visualstatus() && 1 == api_ci_get_mmi_enablestatus())
    {
        APPL_PRINTF("ci_cas_monitor[%d]: not send ca pmt %d times because mmi_visual\n", sc->tc->slot, *times);
        ci_set_timer(CI_DURATION_CAS_MONITOR, (ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)times);

        /* Check Stream Scrambled Status */
        /* If NIM Lock but AV still Scrambled, then Send CA_PMT to CAM again */
        if (sc != NULL && sc->tc != NULL
            && ci_is_av_scrambled(sc->tc->slot)
            && ci_cas_nim_get_lock(sc->tc->slot))
        {
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CI_NOT_DESCRAMBLED);
        }
        return;
    }

    if (*times)
    {
        /* Check Stream Scrambled Status */
        /* If NIM Lock but AV still Scrambled, then Send CA_PMT to CAM again */
        if (sc != NULL && sc->tc != NULL
            && ci_is_av_scrambled(sc->tc->slot)
            && ci_cas_nim_get_lock(sc->tc->slot) && !ci_cam_check(sc->tc->slot,"DREcrypt MPEG4"))
        {
            APPL_PRINTF("ci_cas_monitor[%d]: send ca pmt %d\n",sc->tc->slot,*times);
            /* send not_descrambled msg to app */

            //ci+ do not resend ca_pmt 20100325
            //We may need to reset the dsc for this case
            //But currently, we don't have this issue
            //(can't descramble when CI Plus program)
            //So it should be considered when we come across
            //TO DO...
#ifdef CI_PLUS_SUPPORT
            if (CICAM_CI_PLUS == api_ci_get_cam_type(sc->tc->slot))    //CI+ CAM
                ci_set_timer(CI_DURATION_CAS_MONITOR, (ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)times);
            else    //CI CAM
                ci_cas_send_pmt(sc, CI_CPCI_OK_DESCRAMBLING);
#else
            ci_cas_send_pmt(sc, CI_CPCI_OK_DESCRAMBLING);
#endif
            api_ci_msg_to_app(sc->tc->slot, API_MSG_CI_NOT_DESCRAMBLED);
            (*times) --;
            return;    //ci_cas_send_PMT() will set timer for ci_cas_monitor()
        }

//<<<<<<< FIX BUG00388 for DREcrypt MPEG4 card(which is special for change program between DVB-S and DVB-S2)
        struct vdec_status_info cur_status;

        MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
        vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
        static UINT32 display_idx = 0xFFFFFFFF;
        if (sc != NULL && sc->tc != NULL
            && (display_idx == cur_status.display_idx || !cur_status.u_first_pic_showed)
            && ci_cas_nim_get_lock(sc->tc->slot) && ci_cam_check(sc->tc->slot,"DREcrypt MPEG4"))
            {
            ci_set_timer(0, (ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING);
            }
        display_idx = cur_status.display_idx;
        if(NULL == sc || NULL == sc->tc)
        {
            APPL_PRINTF("NULL Pointer!\n");
            return;
        }
        // Set Timer for Next Monitor
        if (ci_cam_check(sc->tc->slot,"DREcrypt MPEG4") && display_idx != 0xFFFFFFFF)
            ci_set_timer(1400,(ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)times);
//>>>>>>>>
        else
        ci_set_timer(CI_DURATION_CAS_MONITOR, (ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)times);
    }
    else
    {
        APPL_PRINTF("ci_cas_monitor[%d]: %d\n",sc->tc->slot,*times);
    }
}

#if CIPLUS_DEBUG_INFO_APP
/* Dump CA_PMT feild, from program_number to table end */
static void dump_ca_pmt(UINT8 *data, int len)
{
    UINT32 temp;
    UINT8 *p;
    int i, info_length, stream_info_length, descriptor_length;

    api_ci_mutex_lock();
    p = data;
    APPL_PRINTF("dump_CA_PMT: begin dump CA_PMT\n");
    temp = (p[0] << 8) | p[1];
    APPL_PRINTF("program_number\t= 0x%04x\n", temp);
    temp = (p[2] >> 6) & 0x3;
    APPL_PRINTF("reserved      \t= 0x%01x\n", temp);
    temp = (p[2] >> 1) & 0x1f;
    APPL_PRINTF("version_number\t= 0x%02x\n", temp);
    temp = p[2] & 0x1;
    APPL_PRINTF("current_next_ind\t= 0x%01x\n", temp);
    temp = (p[3] >> 4) & 0xf;
    APPL_PRINTF("reserved      \t= 0x%01x\n", temp);
    info_length = ((p[3] & 0xf) << 8) | p[4];
    stream_info_length  = len - info_length - 6;
    APPL_PRINTF("progr_info_length\t= 0x%03x\n", info_length);
    p = &(data[5]);
    if (info_length > 0)
    {
        APPL_PRINTF("{\n");
        APPL_PRINTF("\tca_pmt_cmd_id\t= 0x%02x\n", *p++);
        info_length--;
        while (info_length > 0)
        {
            APPL_PRINTF("\t{\n");
            APPL_PRINTF("\t\tdescriptor_tag\t= 0x%02x\n", *p++);
            descriptor_length = *p++;
            APPL_PRINTF("\t\tdescriptor_len\t= 0x%02x\n", descriptor_length);
            APPL_PRINTF("\t\tdescriptor_data\t=");
            for (i = 0; i < descriptor_length; i++)
                APPL_PRINTF(" 0x%02x", *p++);
            APPL_PRINTF("\n");
            APPL_PRINTF("\t}\n");
            info_length -= (descriptor_length + 2);
        }
        APPL_PRINTF("}\n");
    }
    while (stream_info_length > 0)
    {
        APPL_PRINTF("{\n");
        APPL_PRINTF("\tstream_type\t= 0x%02x\n", *p++);
        temp = (p[0] >> 5) & 0x7;
        APPL_PRINTF("\treserved\t= 0x%01x\n", temp);
        temp = ((p[0] & 0x1f) << 8) | p[1];
        APPL_PRINTF("\telement_PID\t= 0x%02x\n", temp);
        p += 2;
        temp = (p[0] >> 4) & 0xf;
        APPL_PRINTF("\treserved\t= 0x%01x\n", temp);
        info_length = ((p[0] & 0x0f) << 8) | p[1];
        APPL_PRINTF("\tES_info_len\t= 0x%02x\n", info_length);
        p += 2;
        stream_info_length -= 5;
        stream_info_length -= info_length;
        if (info_length > 0)
        {
            APPL_PRINTF("\t{\n");
            APPL_PRINTF("\t\tca_pmt_cmd_id\t= 0x%02x\n", *p++);
            info_length--;
            while (info_length > 0)
            {
                APPL_PRINTF("\t\t{\n");
                APPL_PRINTF("\t\t\tdescriptor_tag\t= 0x%02x\n", *p++);
                descriptor_length = *p++;
                APPL_PRINTF("\t\t\tdescriptor_len\t= 0x%02x\n", descriptor_length);
                APPL_PRINTF("\t\t\tdescriptor_data\t=");
                for (i = 0; i < descriptor_length; i++)
                    APPL_PRINTF(" 0x%02x", *p++);
                APPL_PRINTF("\n");
                APPL_PRINTF("\n");
                info_length -= (descriptor_length + 2);
                APPL_PRINTF("\t\t}\n");
            }
            APPL_PRINTF("\t}\n");
        }
        APPL_PRINTF("}\n");
    }
    api_ci_mutex_unlock();
}
#endif

/* Copy Received CA_PMT to Local Variable */
void ci_cas_cp_pmt(struct ci_ca_pmt* l_ca_pmt, int slot)
{
    APPL_PRINTF("ci_cas_cp_PMT[%d]: ca_pmt 0x%X\n", slot,l_ca_pmt);

    if (l_ca_pmt != NULL)
    {
        ca_pmt[slot].length = l_ca_pmt->length;
        MEMCPY(ca_pmt[slot].buffer, l_ca_pmt->buffer, CA_PMT_BUF_LEN);
        ca_pmt[slot].program_id = l_ca_pmt->program_id;
        ca_pmt[slot].ca_pmt_list_management = l_ca_pmt->ca_pmt_list_management;
        ca_pmt[slot].force_resend = l_ca_pmt->force_resend;
        ca_pmt[slot].pmt_crc= l_ca_pmt->pmt_crc;
        MEMCPY(&ca_pmt[slot].es_pid, &l_ca_pmt->es_pid, sizeof(struct ci_ca_es_pid_management));
    }
}

/* Filter Local CA_PMT for Send it to CAM
 * return:
 * CA_PMT_INVALID_CA_SYSTEM  0
 * CA_PMT_VALID_CA_SYSTEM  1
 * CA_PMT_CLEAR_STREAM     2
 */
#define CA_PMT_INVALID_CA_SYSTEM 0
#define CA_PMT_VALID_CA_SYSTEM 1
#define CA_PMT_CLEAR_STREAM     2
static INT32 ci_cas_filt_pmt(struct ci_senssion_connect *sc, struct ci_ca_pmt *out, UINT8 ca_pmt_cmd_id)
{
    UINT16 *cas_id;
    int cas_id_num;
    UINT8 ca_system_id_hi;     /* High Byte of CA System ID */
    UINT8 __MAYBE_UNUSED__ ca_system_id_lo;     /* Low Byte of CA System ID */
    short info_length, info_length_new, stream_info_length;
    UINT8 *curr_pt, *curr_pt_new, *base_pt_new;
    UINT8 ca_descriptor_len, valid_flag;
    INT32 result = 0, clear_flag = 0;
    int i;
    UINT16 pid;
    int slot = sc->tc->slot;
    UINT8 use_prog_ca = 0;
    UINT8 use_prog_ca_patch = 0;
    UINT8 __MAYBE_UNUSED__ stream_type;

    MEMSET(out, 0, sizeof(struct ci_ca_pmt));
    if (ca_pmt[slot].length == 0)
    {
        result = CA_PMT_INVALID_CA_SYSTEM;
        return result;
    }

    if(RET_SUCCESS == api_ci_send_allpmt(slot))
    {
        use_prog_ca_patch = 1;
    }

    cas_id = ci_cas_get_ids(sc);                    /* CA System IDs */
    cas_id_num = ci_cas_get_ids_num(sc);            /* Number of CA System IDs */
    api_ci_mutex_lock();
    MEMCPY(out->buffer, ca_pmt[slot].buffer, 4);        /* Copy Program Head */
    out->buffer[0] = out->ca_pmt_list_management = ca_pmt[slot].ca_pmt_list_management;    /* Buffer[0]:CA_PMT_List_Management */
    out->program_id = ca_pmt[slot].program_id;            /* Program ID */
    out->force_resend = ca_pmt[slot].force_resend;        /* Force to Resend CA_PMT or not */
    curr_pt = &(ca_pmt[slot].buffer[4]);                    /* Seek to Program_Info_Length */
    curr_pt_new = &(out->buffer[4]);
    info_length = ((curr_pt[0] & 0x0f) << 8) | curr_pt[1];    /* Program_Info_Length */
    info_length_new = 0;
    stream_info_length = ca_pmt[slot].length - info_length - 6;    /* ES Info Length */
    curr_pt += 2;                            /* Seek to Stream Level or Prog Info */
    curr_pt_new += 2;

    /* Program Level */
    if (info_length > 0)
    {
        curr_pt_new[0] = ca_pmt_cmd_id;    /* Buffer[6]: CA_PMT_Cmd_ID */
        info_length_new += 1;
        info_length -= 1;                    /* Jump over CA_PMT_Cmd_ID */
        curr_pt += 1;                        /* Seek to CA_Descriptor */
        curr_pt_new += 1;
        valid_flag = 0;                    /* Pre-Set Valid Flag to False */
        clear_flag++;
        while (info_length > 0)
        {
            ca_descriptor_len = curr_pt[1];
            ca_system_id_hi = curr_pt[2];
            ca_system_id_lo = curr_pt[3];

            /* Search the CA System IDs List and Copy to New CA PMT */
            for (i = 0; i < cas_id_num; i++)
            {
                // Only Compare CA System ID High Byte
                if ((cas_id[i] >> 8) == ca_system_id_hi
#ifdef CI_PLUS_SUPPORT
                    && (cas_id[i] & 0xFF) == ca_system_id_lo
#endif
                    )
                {
                    result = CA_PMT_VALID_CA_SYSTEM;                /* Set Return Flag */
                    MEMCPY(curr_pt_new, curr_pt, ca_descriptor_len + 2);    /* Copy CA_Descriptor */
                    info_length_new += (ca_descriptor_len + 2);
                    curr_pt_new += (ca_descriptor_len + 2);                /* Seek to Next CA_Descriptor */
                    valid_flag = 1;            /* Set Valid Flag */
                    break;
                }
            }
            curr_pt += (ca_descriptor_len + 2);
            info_length -= (ca_descriptor_len + 2);
        }
        if (valid_flag == 0)
        {
            info_length_new -= 1;
            curr_pt_new -= 1;
        }
    }

    out->buffer[4] = (0xf0 | (info_length_new >> 8));    /* Buffer[4]: Program_Info_Length High Byte */
    out->buffer[5] = (info_length_new & 0xff);        /* Buffer[5]: Program_Info_Length Low Byte */

    /* ES Info */
    while (stream_info_length > 0)
    {
        //if out->es_pid_num == 0, Mean Support All ES
        stream_type = curr_pt[0];
        pid = (((curr_pt[1] & 0x1F)) << 8) | curr_pt[2];        /* ES PID */
        if (ca_pmt[slot].es_pid.pid_num > 0)
        {
            for (i = 0; i < ca_pmt[slot].es_pid.pid_num; i++)
            {
                // Compare ES PID
                if ((pid == (ca_pmt[slot].es_pid.pid[i] & 0x1fff))
                    || (1==use_prog_ca))
                {
                    break;
                }
            }
            //If This ES PID is Not Within ES PID List, do Ignore it!
            if (i >= ca_pmt[slot].es_pid.pid_num)
            {
                info_length = ((curr_pt[3] & 0x0f) << 8) | curr_pt[4];    /* ES_Info_Length */
                stream_info_length -= (5 + info_length);
                curr_pt += (5 + info_length);                        /* Seek to Next ES_Info */
                continue;
            }
        }
        APPL_PRINTF("ci_cas_filt_PMT[%d]: es pid 0x%X\n", slot,pid);

        MEMCPY(curr_pt_new, curr_pt, 3);                            /* Copy ES Info Head */
        curr_pt += 3;                                                /* Seek to ES_Info_Length */
        curr_pt_new += 3;
        base_pt_new = curr_pt_new;
        info_length = ((curr_pt[0] & 0x0f) << 8) | curr_pt[1];            /* ES_Info_Length */
        info_length_new = 0;
        stream_info_length -= 5;
        stream_info_length -= info_length;
        curr_pt += 2;                                                /* Seek to Next ES Info */
        curr_pt_new += 2;

        if((pid == (ca_pmt[slot].es_pid.pid[CI_VIDEO_INDEX] & 0x1fff))&&(1==use_prog_ca_patch))
        {
            use_prog_ca = 1;
        }

        /* ES Level */
        if (info_length > 0)
        {
            curr_pt_new[0] = ca_pmt_cmd_id;    /* Copy CA_PMT_Cmd_ID */
            info_length_new += 1;                /* Jump Over CA_PMT_Cmd_ID */
            info_length -= 1;
            curr_pt += 1;                        /* Seek to CA_Descriptor */
            curr_pt_new += 1;
            valid_flag = 0;                    /* Pre-Set Valid Flag to False */
            clear_flag++;
            while (info_length > 0)
            {
                ca_descriptor_len = curr_pt[1];
                ca_system_id_hi = curr_pt[2];
                ca_system_id_lo = curr_pt[3];

                /* Search the CA System IDs List and Copy to New CA PMT */
                for (i = 0; i < cas_id_num; i++)
                {
                    // Only Compare CA System ID High Byte
                    if ((cas_id[i]>>8) == ca_system_id_hi
#ifdef CI_PLUS_SUPPORT
                        && (cas_id[i] & 0xFF) == ca_system_id_lo
#endif
                     )
                    {
                        result = CA_PMT_VALID_CA_SYSTEM;                                /* Set Return Flag */
                        MEMCPY(curr_pt_new, curr_pt, ca_descriptor_len + 2);
                        info_length_new += (ca_descriptor_len + 2);
                        curr_pt_new += (ca_descriptor_len + 2);        /* Seek to Next CA_Descriptor */
                        valid_flag = 1;                            /* Set Valid Flag */
                        break;
                    }
                }
                curr_pt += (ca_descriptor_len + 2);                    /* Seek to Next CA_Descriptor */
                info_length -= (ca_descriptor_len + 2);
            }
            if (valid_flag == 0)
            {
                info_length_new -= 1;
                curr_pt_new -= 1;
            }
        }
        base_pt_new[0] = (0xf0 | (info_length_new >> 8));        /* Copy ES_Info_Length */
        base_pt_new[1] = (info_length_new & 0xff);
    }
    out->length = curr_pt_new - out->buffer;
    api_ci_mutex_unlock();

    if (clear_flag==0)
        result = CA_PMT_CLEAR_STREAM;
    return result;
}

extern UINT32 ci_get_cur_program_id(int slot);
BOOL api_ci_cas_absent(int slot)
{
#ifndef CI_PLUS_SUPPORT
    char module_name[33];
    int i;

    api_ci_get_menu_string(module_name, 33, slot);

    for (i=0; i<CI_CAM_NO_CAS_SENSS; i++)
    {
#ifndef _BUILD_OTA_E_
        if (cas_info[i].cam_name &&
          _strncasecmp(module_name, cas_info[i].cam_name, STRLEN(cas_info[i].cam_name)) == 0
          && !cas_info[i].cas_present)
        {
            APPL_PRINTF("Special CAM: no CAS session available :(\n");
            return TRUE;
        }
#endif
    }
#endif

    return FALSE;
}

/* query cmd patch: some cam cards can not support query cmd! */
/* Question: we have some CAM with the same name, how could we differentiate them? */
static BOOL ci_patch_by_name(int slot)
{
//    return TRUE;    //for test, SMiT not support CI_CPCI_QUERY cmd
   
#ifdef CI_DESCRAMBLE_DIRECT
    //if cam do not support CI_CPCI_QUERY cmd,
    //send CI_CPCI_OK_DESCRAMBLING directly.
    return TRUE;
#else
    char patch_module[][33] = {"Conax ITI CIPLUS CAM","TSD Crypt Beta2","CryptoWorks","Viaccess Access","POLSAT","Irdeto Access","MODULE RETAIL","DREcrypt MPEG4","DRE NKE"};//
    char module_name[33];
    int i;

    api_ci_get_menu_string(module_name, 33, slot);

    for (i=0; i<9; i++)
    {
#ifndef _BUILD_OTA_E_
        if (_strncasecmp(module_name, patch_module[i], STRLEN(patch_module[i])) == 0)
            return TRUE;
#endif
    }

    return FALSE;
#endif
}

#ifdef CI_PLUS_TEST_CASE
extern UINT32 ciplus_case_idx_spdif;
extern BOOL api_ciplus_test_spdif(UINT32 case_idx);
#endif

extern UINT32 ci_current_prog_id;
extern UINT32 ci_prog_id_bak;

#ifdef CI_PLUS_SUPPORT
extern UINT8 dis_analog_out;
extern void ci_notify_uri(int slot, struct ci_uri_message *uri_msg);
extern void uri_enable_analog_output();
#endif
/* Send CA_PMT to CAM Card */
INT32 ci_cas_send_pmt(struct ci_senssion_connect *sc, UINT8 ca_pmt_cmd_id)
{
    int i;
    static UINT32 pre_ca_pmt_crc = 0;
    static UINT16 pre_program_number = -1;
    static UINT16 last_program_number = -1;
    static UINT32 last_tick = 0;
    static int last_slot = -1;
    UINT32 tick;
    UINT16 program_number;
    INT32 ca_valid;                /* ca system valid */
    struct ci_conditional_access *tp = NULL;
    struct ci_ca_pmt cam_ca_pmt;
    UINT8 bk_ca_pmt_cmd_id = ca_pmt_cmd_id;

    MEMSET(&cam_ca_pmt, 0, sizeof(struct ci_ca_pmt));
    //When Card Plug Out, some Timers of ci_cas_send_PMT still Work, so need Check here
    if (sc == NULL)
    {
        APPL_PRINTF("ci_cas_send_PMT: sc[%X] error!!!\n", sc);
        return ERR_FAILUE;
    }

    tp = (struct ci_conditional_access *)(sc->appdata);
    if (tp == NULL || sc->tc == NULL || sc->tc->link == NULL
        || sc->tc->slot >= CI_MAX_SLOT_NUM)
    {
        APPL_PRINTF("ci_cas_send_PMT: sc[%X] error!!!\n", sc);
        return ERR_FAILUE;
    }

    /* Whatever the State(lock or not), Decrease the ca_pmt count.*/
    osal_task_dispatch_off();
    if (--sc->tc->link->slot[sc->tc->slot].ca_pmt_cnt < 0)
    {
        sc->tc->link->slot[sc->tc->slot].ca_pmt_cnt = 0;
    }
    osal_task_dispatch_on();

    /* query cmd patch: some cam cards can not support query cmd! */
    if (ca_pmt_cmd_id == CI_CPCI_QUERY && ci_patch_by_name(sc->tc->slot))
    {
        ca_pmt_cmd_id = CI_CPCI_OK_DESCRAMBLING;
    }

    /* Setup CA PMT for the CAM */
    ca_valid = ci_cas_filt_pmt(sc, &cam_ca_pmt, ca_pmt_cmd_id);
    if ( ca_valid == CA_PMT_INVALID_CA_SYSTEM)
    {
        ca_pmt_cmd_id = CI_CPCI_NOT_SELECTED;
        APPL_PRINTF("ci_cas_send_PMT[%d]: ca system invalid!\n", sc->tc->slot);
    }
    else if ( ca_valid == CA_PMT_CLEAR_STREAM)
    {
        ca_pmt_cmd_id = CI_CPCI_OK_DESCRAMBLING;
#ifdef CI_PLUS_SUPPORT
        if (CICAM_CI_PLUS == api_ci_get_cam_type(sc->tc->slot))
        {
            uri_enable_analog_output();//resume analog output if it was disabled
        }
#endif
        APPL_PRINTF("ci_cas_send_PMT[%d]: It must be clear stream,not send pmt!\n", sc->tc->slot);
        return SUCCESS;
    }

#if CIPLUS_DEBUG_INFO_APP
    dump_ca_pmt(&cam_ca_pmt.buffer[1], cam_ca_pmt.length - 1);
#endif

    /* Then, Send CA PMT */
    if (cam_ca_pmt.length == 0)
    {
        APPL_PRINTF("ci_cas_send_PMT[%d]: CA PMT NULL or don't lock! return\n", sc->tc->slot);
        return SUCCESS;
    }
    APPL_PRINTF("ci_cas_send_PMT[%d]: Send CA PMT\n", sc->tc->slot);
    for (i = 0; i < cam_ca_pmt.length; i++)
    {
        APPL_PRINTF(" %02x", cam_ca_pmt.buffer[i]);
    }
    APPL_PRINTF("\n");

    program_number = (cam_ca_pmt.buffer[1]<<8) | cam_ca_pmt.buffer[2];    /* Program Number to Descramble */

    if (tp->state == 2)
    {
        if (sc->tc->link->slot[sc->tc->slot].ca_pmt_cnt != 0)
        {
            APPL_PRINTF("Drop ca_pmt for create more times before send out!\n");
            return SUCCESS;
        }

        if (cam_ca_pmt.program_id != ci_get_cur_program_id(sc->tc->slot))
        {
            APPL_PRINTF("Drop ca_pmt for program changed!\n");
            return SUCCESS;
        }

        //<<< fix BUG08757
        if (ca_pmt_cmd_id == CI_CPCI_QUERY
            && program_number == last_program_number
            && sc->tc->slot != last_slot)
        {
            tick = osal_get_tick()-last_tick;
            if (tick < CI_CPCI_QUERY_INTERVAL)
            {
                while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_QUERY));
                ci_set_timer(CI_CPCI_QUERY_INTERVAL-tick, (ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_QUERY);
                APPL_PRINTF("ci_cas_send_PMT[%d]: CI_CPCI_QUERY delay %d ms\n", sc->tc->slot, CI_CPCI_QUERY_INTERVAL - tick);
                return SUCCESS;
            }
        }
        //>>>

        api_ci_mutex_lock();

        switch (ca_pmt_cmd_id)
        {
        case CI_CPCI_QUERY:                /* Query Command */
            // Delete Last Old Timers
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING));
            /* Create Timeout Handle to Send OK_DESCRAMBLING Command for some CAM don't support reply */
            ci_set_timer(CI_DURATION_CAS_GUARD, (ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING);
            ca_pmt[sc->tc->slot].ca_monitor_times = CI_CAS_MONITOR_TIMERS;
            //fix BUG ???
            //播放SmiT Irdeto 122的免费台，插入卡后当提示E07/E04时，免费台会停止播放2秒
            ca_pmt[sc->tc->slot].force_resend = FALSE;
            APPL_PRINTF("ci_cas_send_PMT[%d]: CI_CPCI_QUERY\n", sc->tc->slot);
            break;
        case CI_CPCI_OK_DESCRAMBLING:    /* Ok_Descramble Command */
            // Delete Last Old Timers
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING));
            /* If has Send OK_DESCRAMBLING Command, Create Monitor */
            if (ca_pmt[sc->tc->slot].ca_monitor_times == 0)
                ca_pmt[sc->tc->slot].ca_monitor_times = CI_CAS_MONITOR_TIMERS;
//<<< fix bug 00388
            struct vdec_status_info cur_status;
            vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
            if(ci_cam_check(sc->tc->slot,"DREcrypt MPEG4") && !cur_status.u_first_pic_showed)
                ci_set_timer(800, (ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)&ca_pmt[sc->tc->slot].ca_monitor_times);
//>>>>
            else
            ci_set_timer(CI_DURATION_CAS_MONITOR, (ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)&ca_pmt[sc->tc->slot].ca_monitor_times);

            /* If CI+ CAM, start URI timer 20100401 */
#ifdef CI_PLUS_SUPPORT
            //20100922, rem it to avoid DSC re-setting issue
            //ci_set_uri_timer(sc->tc->slot, program_number);
            // ci_set_uri_cam_match(sc->tc->slot);
            if (CICAM_CI_PLUS == api_ci_get_cam_type(sc->tc->slot)
               && (1 == ci_cas_nim_get_lock(sc->tc->slot)))
            {
                APPL_PRINTF("%s: program number = %d, pre program number = %d\n",
                            __FUNCTION__, program_number, pre_program_number);
                if ((cam_ca_pmt.pmt_crc== pre_ca_pmt_crc)
                    && (program_number == pre_program_number))
                {
                    //When from same channel, we just use the old URI
                    uri_set_received();
                    api_uri_bak_resume();

                    state = uri_get_msg(uri_msg);
                    if(uri_msg->uri_from_pvr == 0)
                    {
                        ci_set_timer(500, (ci_timer_handler)ci_notify_uri, (void *)sc->tc->slot, (void *)(uri_msg));
                    }
                }
                else
                {
                    //When channel changing, we reset all config firstly
                    api_ci_camup_answer_setstatus(TRUE);
                    api_uri_clear();
                }
            }
#endif

            if (bk_ca_pmt_cmd_id == CI_CPCI_QUERY)
            {
                ca_pmt[sc->tc->slot].ca_monitor_times = CI_CAS_MONITOR_TIMERS;
                //fix BUG ???
                //播放SmiT Irdeto 122的免费台，插入卡后当提示E07/E04时，免费台会停止播放2秒
                ca_pmt[sc->tc->slot].force_resend = FALSE;
            }
            APPL_PRINTF("ci_cas_send_PMT[%d]: CI_CPCI_OK_DESCRAMBLING\n", sc->tc->slot);
            break;
        case CI_CPCI_NOT_SELECTED:
            // Delete Last Old Timers
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_cas_send_pmt, (void *)sc, (void *)CI_CPCI_OK_DESCRAMBLING));
            APPL_PRINTF("ci_cas_send_PMT[%d]: CI_CPCI_NOT_SELECTED\n", sc->tc->slot);

#ifdef CI_PLUS_SUPPORT
            notify_uri(sc->tc->slot, URI_CLEAR);
            if (CICAM_CI_PLUS == api_ci_get_cam_type(sc->tc->slot))
            {
                uri_enable_analog_output();
            }
#endif
            break;
        case CI_CPCI_OK_MMI:            /* Other Commands */
        default:
            break;
        }
        /* If is fake crypted program, just return success for avoid "No right" pop up menu */
        //for black Viaccess card, need send ca_pmt
#if (!defined CI_PLUS_SUPPORT) && (!defined HD_PLUS_SUPPORT)
        if ((cam_ca_pmt.pmt_crc== pre_ca_pmt_crc)
            && (program_number == pre_program_number)
            && (ca_pmt_cmd_id == CI_CPCI_OK_DESCRAMBLING || ca_pmt_cmd_id == CI_CPCI_QUERY)
            && cam_ca_pmt.ca_pmt_list_management != CI_CPLM_UPDATE
            && !cam_ca_pmt.force_resend
            && ci_is_vdec_running(sc->tc->slot)
            && !ci_cam_check(sc->tc->slot,"Viaccess")
            && !ci_cam_check(sc->tc->slot,"DREcrypt MPEG4"))
        {
            /* If Lock and is Clear Program, Don't Send OK_DESCRAMBLING for "No right" Pop Menu Issue */
            if (!ci_is_av_scrambled(sc->tc->slot)
                || (0 == ci_cas_nim_get_lock(sc->tc->slot)))
            {
                APPL_PRINTF("ci_cas_send_PMT[%d]: fake crypted program or no signal return success\n", sc->tc->slot);
                api_ci_mutex_unlock();
                return SUCCESS;
            }
        }
#endif
        // Send CA PMT in APDU
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_CA_PMT, cam_ca_pmt.length, cam_ca_pmt.buffer) == SUCCESS)
        {
            // If Command is CI_CPCI_OK_DESCRAMBLING or CI_CPCI_OK_MMI, then Enable MMI
                    if(CI_CPCI_OK_DESCRAMBLING == ca_pmt_cmd_id || CI_CPCI_OK_MMI == ca_pmt_cmd_id)
                    {
                        ci_set_mmi_enablestatus(1);
                    }
                    else
                    {
                        ci_set_mmi_enablestatus(0);
                    }

            //<<< fix BUG08757
            if (ca_pmt_cmd_id == CI_CPCI_QUERY)
            {
                last_tick = osal_get_tick();
                last_program_number = program_number;
                last_slot = sc->tc->slot;
            }
            //>>>
#ifdef CI_SEND_CAPMT_OK_TO_APP
            if(ca_pmt_cmd_id == CI_CPCI_NOT_SELECTED)
            {
                //if CI_CPCI_NOT_SELECTED, it means: this CI card don't support ca_system of TS.
                //so don't record ca_pmt_length and program_number. Because these variable is shared
                //by two CI slot operation.
                APPL_PRINTF("%s : ca_pmt_cmd_id == %d\n", __FUNCTION__, ca_pmt_cmd_id);
            }
            else
            {
                pre_ca_pmt_crc    = cam_ca_pmt.pmt_crc;
                pre_program_number = program_number;
            }
            if (ca_valid!=0)
            {
#ifdef CI_PLUS_SUPPORT
                if(CI_CPCI_OK_DESCRAMBLING == ca_pmt_cmd_id)    //20100427
#endif
                    api_ci_msg_to_app(sc->tc->slot, API_MSG_CAPMT_OK);
            }
            else
            {
                api_ci_msg_to_app(sc->tc->slot, API_MSG_CAPMT_CASYSTEM_INVALID);
            }

#ifdef CI_PLUS_TEST_CASE
    ci_set_timer(0, (ci_timer_handler)api_ciplus_test_spdif, ciplus_case_idx_spdif, NULL);
#endif

#else
            pre_ca_pmt_crc    = cam_ca_pmt.pmt_crc;
            pre_program_number = program_number;
#endif
            APPL_PRINTF("ci_cas_send_PMT[%d]: Send APPOBJ_TAG_CA_PMT success\n", sc->tc->slot);
            api_ci_mutex_unlock();
            return SUCCESS;
        }
        api_ci_mutex_unlock();
        APPL_PRINTF("ci_cas_send_PMT[%d]: Send APPOBJ_TAG_CA_PMT failue\n", sc->tc->slot);
    }
    return ERR_FAILUE;
}

/* DVB Host Control Resource */
static void ci_hc_delete(struct ci_senssion_connect *sc);
static INT32 ci_hc_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

/* Create Host Control Session Connection */
struct ci_senssion_connect* ci_hc_create(struct ci_senssion_connect *sc,
    int scnb, struct ci_transport_connect *tc)
{
    struct ci_host_control *tp;

    if (sc == NULL)
    {
        APPL_PRINTF("ci_hc_create: input sc null\n");
        return NULL;
    }

    ci_senss_conn_init(sc, scnb, RSC_ID_HOST_CONTROL, tc);
    sc->appdelete = ci_hc_delete;
    sc->callback = ci_hc_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_host_control))) == NULL)
    {
        APPL_PRINTF("ci_hc_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_host_control*)(sc->appdata);
    tp->state = 0;
    tp->old_nid = 0;
    tp->old_onid = 0;
    tp->old_tsid = 0;
    tp->old_sid = 0;
    MEMSET(&(tp->old_pid[0]), 0, CI_MAX_REPLACE_NUM << 1);
    return sc;
}

/* Delete Host Control Session Connection */
static void ci_hc_delete(struct ci_senssion_connect *sc)
{
    if (sc == NULL)
    {
        APPL_PRINTF("ci_hc_delete: input session connection not exist\n");
        return;
    }

    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

extern INT32 ci_tune_service_by_ft_type(UINT8 slot, UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id);
/* Process for Host Control Session */
static INT32 ci_hc_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_host_control *tp = (struct ci_host_control *)(sc->appdata);
    int tag, l;
    UINT8 *d;
    struct ci_host_tp_param param;

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_TUNE:                /* Tune */
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l == 8)
            {
                APPL_PRINTF("ci_hc_process: Change channel\n");
                APPL_PRINTF("\tnetwork_id = %04x\n", ntohs(d));        /* Network ID */
                param.network_id = ntohs(d);
                d += 2;
                APPL_PRINTF("\to_network_id = %04x\n", ntohs(d));    /* Original Network ID */
                param.org_network_id = ntohs(d);
                d += 2;
                APPL_PRINTF("\tts_id = %04x\n", ntohs(d));            /* Transport Stream ID */
                param.ts_id = ntohs(d);
                d += 2;
                APPL_PRINTF("\tservice_id = %04x\n", ntohs(d));        /* Service ID */
                param.service_id = ntohs(d);

                //TO DO...
                /* Channel change... */
                /*Function for TuneService  2012/04/22
                  *Old function: ci_tune_service
                  *New function: ci_tune_service_by_ft_type
                  *For CI+ CamUpgrade issue
                  */
                //ci_tune_service(sc->tc->slot, param.network_id, param.org_network_id, param.ts_id, param.service_id);
                ci_tune_service_by_ft_type(sc->tc->slot, param.network_id, param.org_network_id, param.ts_id, param.service_id);
            }
            tp->state = 1;
            break;
        case APPOBJ_TAG_REPLACE:            /* Replace */
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l == 5)
            {
                APPL_PRINTF("ci_hc_process: Replace PID\n");
                APPL_PRINTF("\trep_ref = %d\n", *d);                    /* Replacement Reference */
                d++;
                APPL_PRINTF("\told_PID = %04x\n", 0x1fff & ntohs(d));    /* Replaced PID */
                d += 2;
                APPL_PRINTF("\trep_PID = %04x\n", 0x1fff & ntohs(d));    /* Replacement PID */
                if (tp->old_pid[*d] == 0)
                {
                    //TO DO...
                    /* Change PID... */
                }
            }
            tp->state = 1;
            break;
        case APPOBJ_TAG_CLEAR_REPLACE:        /* Clear Replace */
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l == 1)
            {
                APPL_PRINTF("ci_hc_process: Clear replace PID\n");
                APPL_PRINTF("\trep_ref = %d\n", *d);    /* Repacement Reference */
                if (tp->old_pid[*d] != 0)
                {
                    /* Recover PID... */
                    tp->old_pid[*d] = 0;
                }
            }
            break;
        default:
            APPL_PRINTF("ci_hc_process: Unknow tag!\n");
            break;
        }
    }

    return SUCCESS;
}

/* Send Ask Release Object to CAM Card for Host Request to Regain Control */
INT32 ci_hc_ask_release(struct ci_senssion_connect *sc)
{
    struct ci_host_control *tp = (struct ci_host_control *)(sc->appdata);

    if (tp->state == 1)
    {
        if (ci_senss_send_apdu(sc, APPOBJ_TAG_ASK_RELEASE, 0, NULL) == SUCCESS)
        {
            return SUCCESS;
        }
        APPL_PRINTF("ci_hc_ask_release: Send ask release failue\n");
    }
    return ERR_FAILUE;
}

/* Date and Time Resource */
static void ci_dat_delete(struct ci_senssion_connect *sc);
static void ci_dat_monitor(struct ci_senssion_connect *sc, UINT32 cycle);
static INT32 ci_dat_send_date_time(struct ci_senssion_connect *sc);
static INT32 ci_dat_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

/* Create Date and Time Session Connection */
struct ci_senssion_connect * ci_dat_create(struct ci_senssion_connect *sc,
  int scnb, struct ci_transport_connect *tc)
{
    struct ci_date_time *tp;

    ci_senss_conn_init(sc, scnb, RSC_ID_DATE_TIME, tc);
    sc->appdelete = ci_dat_delete;
    sc->callback = ci_dat_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_date_time))) == NULL)
    {
        APPL_PRINTF("ci_dat_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_date_time*)(sc->appdata);
    tp->interval = 0;

    return sc;
}

/* Delete Date and Time Session Connection */
static void ci_dat_delete(struct ci_senssion_connect *sc)
{
    // Delete Session Connection
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
    // Delete Date and Time Monitor Timer
    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_dat_monitor, (void*)sc, (void*)CI_TIMER_PARAM_ANY));
}

/* Monitor for Send Date and Time to CAM Card Periodically */
static void ci_dat_monitor(struct ci_senssion_connect *sc, UINT32 cycle)
{
    // Send Date and Time to CAM Card
    ci_dat_send_date_time(sc);
    // Set Date and Time Monitor Timer
    if (cycle)
    {
        ci_set_timer(cycle, (ci_timer_handler)ci_dat_monitor, (void*)sc, (void *)cycle);
    }
}

#include <api/libsi/si_tdt.h>
#define DEC2BCD(d) (((d / 10) << 4) + (d % 10))

/* Send Date and Time to CAM Card */
static INT32 ci_dat_send_date_time(struct ci_senssion_connect *sc)
{
    //struct ci_date_time *tp = (struct ci_date_time*)(sc->appdata);
    UINT32 mjd;
    date_time tm_gmt;
    UINT8 tm_utc[5];

    get_utc(&tm_gmt);

#ifdef CI_TEST
//    if (tm_gmt.year < 2009)
//        tm_gmt.year = 2010;        //for CI+ Test Certificate
#endif

    mjd = ymd_to_mjd(tm_gmt.year, tm_gmt.month, tm_gmt.day);    /* Convert Y/M/D to MJD Code */

    *(UINT16*)tm_utc = htons(mjd);
    tm_utc[2] = DEC2BCD(tm_gmt.hour);
    tm_utc[3] = DEC2BCD(tm_gmt.min);
    tm_utc[4] = DEC2BCD(tm_gmt.sec);
    APPL_PRINTF("%d: ==> Date Time %d %d/%d/%d\n", sc->scnb,mjd,tm_gmt.year,tm_gmt.month,tm_gmt.day);

    return ci_senss_send_apdu(sc, APPOBJ_TAG_DATE_TIME, 5, tm_utc);
}

/* Process for Date and Time Session */
static INT32 ci_dat_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_date_time *tp = (struct ci_date_time*)(sc->appdata);
    int tag, l;
    UINT8 *d;

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_DATE_TIME_ENQ:    /* Date Time Enquiry */
            tp->interval = 0;
            l = 0;
            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 0)
            {
                tp->interval = *d;            /* Response Interval */
            }
            APPL_PRINTF("ci_dat_process: %d: <== Date Time Enq, interval = %d\n", sc->scnb, tp->interval);
            // Delete Last Date and Time Monitor Timer
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_dat_monitor, (void *)sc, (void *)CI_TIMER_PARAM_ANY));
            // Set New Date and Time Monitor
            ci_dat_monitor(sc, tp->interval * 500);
            break;
        default:
            APPL_PRINTF("ci_dat_process: CI date time: unknown tag %06X\n", tag);
            return ERR_FAILUE;
           }
    }
    return SUCCESS;
}

/* Man-Machine Interface Resource */
static void ci_mmi_delete(struct ci_senssion_connect *sc);
static INT32 ci_mmi_process(struct ci_senssion_connect *sc, int len, UINT8 *data);
static INT32 ci_menu_add_entry(struct ci_menu *menu, char *s);

static struct ci_menu* ci_menu_create(struct ci_mmi *mmi, int selectable);
static void ci_menu_delete(struct ci_menu *menu);
static struct ci_enquiry* ci_enquiry_create(struct ci_mmi *mmi);
static void ci_enquiry_delete(struct ci_enquiry *enquiry);

/* Create MMI Session Connection */
struct ci_senssion_connect* ci_mmi_create(struct ci_senssion_connect *sc,
  int scnb, struct ci_transport_connect *tc)
{
    struct ci_mmi *tp;
    int i;

    ci_senss_conn_init(sc, scnb, RSC_ID_MMI, tc);
    sc->appdelete = ci_mmi_delete;
    sc->callback = ci_mmi_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_mmi))) == NULL)
    {
        APPL_PRINTF("ci_mmi_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_mmi*)(sc->appdata);
    tp->sc = sc;

    for (i = 0; i < CI_MMI_MENU_LEVEL_MAX; i++)
    {
        tp->menu_list[i] = NULL;
    }
    tp->cur_menu = -1;
    tp->enquiry = NULL;
    return sc;
}

/* Delete MMI Session Connection */
static void ci_mmi_delete(struct ci_senssion_connect *sc)
{
    struct ci_mmi *tp = (struct ci_mmi*)(sc->appdata);
    int i;

    /* Remove all Related Timers if MMI Closed */
    while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, CI_TIMER_PARAM_ANY, (void *)(sc->tc->slot)));
    while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(sc->tc->slot), (void *)API_MSG_ENTER_MENU));

    tp->sc = NULL;

    // Clear Menu List
    for (i = 0; i < CI_MMI_MENU_LEVEL_MAX && tp->menu_list[i] != NULL; i++)
    {
        ci_menu_delete(tp->menu_list[i]);
        tp->menu_list[i] = NULL;
    }
    tp->cur_menu = -1;

    // Clear Enquiry and Notify App of Exit Enquiry
    if (tp->enquiry)
    {
        osal_memory_free(tp->enquiry);
        tp->enquiry = NULL;
            api_ci_msg_to_app(sc->tc->slot, API_MSG_ENQUIRY_EXIT);
    }

    // Clear Others
    sc->appdelete = NULL;
    sc->callback = NULL;
    if (sc->appdata)
    {
        osal_memory_free(sc->appdata);
        sc->appdata = NULL;
    }
}

/* Get Text From APDU */
static char * ci_mmi_get_text(struct ci_senssion_connect *sc, int *len, UINT8 **data)
///< Get Text at Data.
///< Return a Pointer to a Newly Allocated String, or NULL in case of Error.
///< Upon Return Length and Data Represent the Remaining Data after Text has been Skipped.
{
    char *s;
    int tag = ci_get_apdu_tag(len, data);

    if (tag == APPOBJ_TAG_TEXT_LAST)
    {
        s = ci_get_string(len, data);
        APPL_PRINTF("ci_mmi_get_text: %d: <== Text Last '%s'\n", sc->scnb, s);
        return s;
    }
    else
    {
        APPL_PRINTF("ci_mmi_get_text: unexpected text tag: %06X\n", tag);
    }
    return NULL;
}

/* Push Menu to Menu List */
static void ci_mmi_push_menu(struct ci_mmi *mmi, struct ci_menu *menu)
{
    int i, j;
    struct ci_menu *tp_menu;

    /* Search Menu List, if is Same as Upper Layer Menu, Roll Back */
    for (i = 0; i < CI_MMI_MENU_LEVEL_MAX && mmi->menu_list[i] != NULL; i++)
    {
        tp_menu = mmi->menu_list[i];
        if (tp_menu->mmi == menu->mmi &&
                tp_menu->selectable == menu->selectable &&
                STRCMP(tp_menu->title_text, menu->title_text) == 0 &&
                STRCMP(tp_menu->subtitle_text, menu->subtitle_text) == 0 &&
                STRCMP(tp_menu->bottom_text, menu->bottom_text) == 0)
        {
            /* It seems same,     Free all Lower Menus */
            for (j = i;
                     j < CI_MMI_MENU_LEVEL_MAX && mmi->menu_list[j] != NULL;
                     j++)
            {
                ci_menu_delete(mmi->menu_list[j]);
                mmi->menu_list[j] = NULL;
            }
            break;
        }
    }
    /* Add New Menu */
    if (CI_MMI_MENU_LEVEL_MAX == i)
    {
        i = CI_MMI_MENU_LEVEL_MAX - 1;
        ci_menu_delete(mmi->menu_list[i]);
    }
    mmi->menu_list[i] = menu;
    mmi->cur_menu = i;
}

/* Pop Menu from Menu List (Return Upper Level Menu) */
int ci_mmi_pop_menu(struct ci_mmi *mmi)
{
    if (mmi->cur_menu >= 0)
    {
        ci_menu_delete(mmi->menu_list[mmi->cur_menu]);
        mmi->menu_list[mmi->cur_menu] = NULL;
        return    --(mmi->cur_menu);
    }

    return -1;
}

extern INT32 ci_senss_close_conn_requ(struct ci_senssion_connect *sc);
/* Process for MMI Session */
static INT32 ci_mmi_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_mmi *tp = (struct ci_mmi*)(sc->appdata);
    struct ci_menu *menu;
    int tag, l = 0, i;
    UINT8 *d, blind;
    char *s;
    struct ci_mmi_display_reply dr;

    if (data)
    {
        // Get Tag of APDU
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_CLOSE_MMI:            /* Close MMI */
                    APPL_PRINTF("ci_mmi_process: %d: <== Close MMI\n", sc->scnb);
            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 0)
            {
                if(NULL == sc->tc)
                {
                    ASSERT(0);
                    return ERR_FAILUE;
                }
                // Process Close_MMI_Cmd_ID
                switch (*d)
                {
                case CI_CMI_IMMEDIATE:    /* Close MMI Immediately*/
                    APPL_PRINTF("ci_mmi_process: %02X: <== Close MMI immediate\n", *d);
                    // Notify App of Exit Top Menu
                    api_ci_msg_to_app(sc->tc->slot, API_MSG_EXIT_MENU);
                    // Send Request for Close MMI Session Connection
                    ci_senss_close_conn_requ(sc);
                    break;
                case CI_CMI_DELAY:        /* Close MMI after Delay some time */
                    APPL_PRINTF("ci_mmi_process: %02X: <== Close MMI delay\n", *d);
                    // Set Timer for Notify App of Exit Top Menu after some time
                    ci_set_timer((*(++d) * 500), (ci_timer_handler)api_ci_msg_to_app, (void *)sc->tc->slot, (void *)API_MSG_EXIT_MENU);
                    // Send Request for Close MMI Session Connection
                    ci_senss_close_conn_requ(sc);
                    break;
                default:
                    APPL_PRINTF("ci_mmi_process: unsupported close MMI id %02X\n", *d);
                    return ERR_FAILUE;
                }
                /* Remove all related Timers if CAM Feedback */
                while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, CI_TIMER_PARAM_ANY, (void*)(sc->tc->slot)));
                while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(sc->tc->slot), (void*)API_MSG_ENTER_MENU));
            }
            break;
        case APPOBJ_TAG_DISPLAY_CONTROL:    /**/

            APPL_PRINTF("ci_mmi_process: %d: <== Display Control\n", sc->scnb);
            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 0)
            {
                MEMSET(&dr, 0, sizeof(struct ci_mmi_display_reply));
                switch (*d)
                {
                case CI_DCC_SET_MMI_MODE:
                    if (l == 2 && *++d == CI_MM_HIGH_LEVEL)
                    {
                        dr.id = CI_DRI_MMI_MODE_ACK;
                        dr.mode = CI_MM_HIGH_LEVEL;
                        dr.buffer[0] = dr.id;
                        dr.buffer[1] = dr.mode;
                        APPL_PRINTF("ci_mmi_process: %d: ==> Display Reply (MMI_Mode_Ack)\n", sc->scnb);
                        //ci_senss_send_APDU(sc, APPOBJ_TAG_DISPLAY_REPLY, 2,(UINT8 *)dr.buffer);
                        if(ci_senss_send_apdu(sc, APPOBJ_TAG_DISPLAY_REPLY, 2, (UINT8 *)dr.buffer) != SUCCESS)
                        {
                            APPL_PRINTF("ci_senss_send_APDU() failed!\n");
                        }
                    }
                    break;
                case CI_DCC_DISPLAY_CHARACTER_TABLE_LIST:
                    if (l == 1)
                    {
                        dr.id = CI_DRI_LIST_DISPLAY_CHARACTER_TABLES;
                        for(i = 0; i < 0x60 /* 0x80 - 0x20 */; i++)
                            dr.characer_table_byte[i] = 0x20 + i;
                        dr.buffer[0] = dr.id;
                        MEMCPY(&dr.buffer[1], dr.characer_table_byte, i - 1);
                        APPL_PRINTF("ci_mmi_process: %d: ==> Display Reply (List_Display_Character_Tables)\n", sc->scnb);
                        //ci_senss_send_APDU(sc, APPOBJ_TAG_DISPLAY_REPLY, i, (UINT8 *)dr.buffer);
                        if(ci_senss_send_apdu(sc, APPOBJ_TAG_DISPLAY_REPLY, i, (UINT8 *)dr.buffer) != SUCCESS)
                        {
                             APPL_PRINTF("ci_senss_send_APDU() failed!\n");
                        }
                    }
                    break;
                case CI_DCC_INPUT_CHARACTER_TABLE_LIST:
                    if (l == 1)
                    {
                        dr.id = CI_DRI_LIST_INPUT_CHARACTER_TABLES;
                        for(i = 0; i < 0x60 /* 0x80 - 0x20 */; i++)
                            dr.characer_table_byte[i] = 0x20 + i;
                        dr.buffer[0] = dr.id;
                        MEMCPY(&dr.buffer[1], dr.characer_table_byte, i - 1);
                        APPL_PRINTF("ci_mmi_process: %d: ==> Display Reply (List_Input_Character_Tables)\n", sc->scnb);
                        //ci_senss_send_APDU(sc, APPOBJ_TAG_DISPLAY_REPLY, i, (UINT8 *)dr.buffer);
                        if(ci_senss_send_apdu(sc, APPOBJ_TAG_DISPLAY_REPLY, i, (UINT8 *)dr.buffer) != SUCCESS)
                        {
                            APPL_PRINTF("ci_senss_send_APDU() failed!\n");
                        }
                    }
                    break;
                default:
                    APPL_PRINTF("ci_mmi_process: unsupported display control command %02X\n", *d);
                    return ERR_FAILUE;
                }
            }
            break;
        case APPOBJ_TAG_LIST_LAST:
        case APPOBJ_TAG_MENU_LAST:
            APPL_PRINTF("ci_mmi_process: %d: <== Menu Last\n", sc->scnb);
            if ((menu = ci_menu_create(tp, (tag == APPOBJ_TAG_MENU_LAST ? 1 : 0))) == NULL)
            {
                APPL_PRINTF("ci_mmi_process: create menu faile\n");
                return ERR_FAILUE;
            }

            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 0)
            {
                /* Since the specification allows choiceNb to be undefined
                   it is useless, so let's just skip it. */
                d++;
                l--;
                if (l > 0)
                    menu->title_text = ci_mmi_get_text(sc, &l, &d);
                if (l > 0)
                    menu->subtitle_text = ci_mmi_get_text(sc, &l, &d);
                if (l > 0)
                    menu->bottom_text = ci_mmi_get_text(sc, &l, &d);
                while (l > 0)
                {
                    s = ci_mmi_get_text(sc, &l, &d);
                    if (s)
                    {
                        if (ci_menu_add_entry(menu, s) != SUCCESS)
                           osal_memory_free(s);
                    } else
                        break;
                }
            }
            if (menu->title_text != NULL
                && menu->subtitle_text != NULL
                && menu->bottom_text != NULL)
            {
                ci_mmi_push_menu(tp, menu);
            }
            else
            {
                APPL_PRINTF("ci_mmi_process: menu is NULL!\n");
            }
            /* Remove timer if CAM feedback */
            if(NULL == sc->tc)
            {
                ASSERT(0);
                return ERR_FAILUE;
            }
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, CI_TIMER_PARAM_ANY, (void *)(sc->tc->slot)));
            while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(sc->tc->slot), (void *)API_MSG_ENTER_MENU));

            api_ci_msg_to_app(sc->tc->slot, API_MSG_MENU_UPDATE);
            break;
        case APPOBJ_TAG_ENQ:
            APPL_PRINTF("ci_mmi_process: %d: <== Enq\n", sc->scnb);
            ci_enquiry_delete(tp->enquiry);
            if ((tp->enquiry = ci_enquiry_create(tp)) == NULL)
            {
                APPL_PRINTF("ci_mmi_process: create enquiry faile\n");
                return ERR_FAILUE;
            }
            d = ci_senss_get_apdu(sc, &l, data);
            if (l > 0)
            {
                blind = *d++;
                // XXX GetByte()???
                l--;
                tp->enquiry->blind = (blind & CI_ENQUIRY_FLAGS_BLIND);
                tp->enquiry->expected_length = *d++;
                l--;
                /* I really wonder why there is no text length field here... */
                tp->enquiry->text = ci_copy_string(l, d);
                APPL_PRINTF("ci_mmi_process: string \"%s\"\n", tp->enquiry->text);
            }
            /* Remove timer if CAM feedback */
            if(NULL == sc->tc)
            {
                ASSERT(0);
                return ERR_FAILUE;
            }
            while (SUCCESS == ci_del_timer((ci_timer_handler)ci_mmi_watchdog, CI_TIMER_PARAM_ANY, (void *)(sc->tc->slot)));
            while (SUCCESS == ci_del_timer((ci_timer_handler)api_ci_msg_to_stack, (void *)(sc->tc->slot), (void *)API_MSG_ENTER_MENU));
            api_ci_msg_to_app(sc->tc->slot, API_MSG_ENQUIRY_UPDATE);
            break;
        default:
            APPL_PRINTF("ci_mmi_process: unknown tag %06X\n", tag);
            return ERR_FAILUE;
        }
    }
    return SUCCESS;
}

INT32 ci_mmi_close_mmi(struct ci_senssion_connect *sc)
{
    UINT8 id = CI_CMI_IMMEDIATE;

    APPL_PRINTF("ci_mmi_close_mmi: %d: ==> Close MMI\n", sc->scnb);
    if (ci_senss_send_apdu(sc, APPOBJ_TAG_CLOSE_MMI, 1, &id) == SUCCESS)
    {
        return SUCCESS;
    }
    APPL_PRINTF("ci_mmi_close_mmi: Send APPOBJ_TAG_CLOSE_MMI failue\n");
    return ERR_FAILUE;
}

static INT32 ci_mmi_send_menu_answer(struct ci_senssion_connect *sc, UINT8 selection)
{
    APPL_PRINTF("ci_mmi_send_menu_answer: %d: ==> Menu Answ\n", sc->scnb);
    return ci_senss_send_apdu(sc, APPOBJ_TAG_MENU_ANSW, 1, &selection);
}

static INT32 ci_mmi_send_answer(struct ci_senssion_connect *sc, char *text, int answ)
{
    APPL_PRINTF("ci_mmi_send_answer: %d: ==> Answ\n", sc->scnb);
    struct ci_mmi_answer {
        UINT8 id;
        char text[256];
    } answer;

       MEMSET(&answer, 0, sizeof(struct ci_mmi_answer));
    answer.id = answ == 1 ? CI_MMI_ANSWER_ID_ANSWER : CI_MMI_ANSWER_ID_CANCEL;
    if (text)
    {
        //STRCPY(answer.text, text);
        strncpy(answer.text, text, 255);
        answer.text[255] = 0;
    }
    return ci_senss_send_apdu(sc, APPOBJ_TAG_ANSW, text ? STRLEN(text) + 1 : 1,
      (UINT8 *)&answer);
}

struct ci_menu *ci_mmi_get_menu(void *buffer, int len, struct ci_senssion_connect *sc)
{
    struct ci_mmi *tp = (struct ci_mmi *)(sc->appdata);
    struct ci_menu *pd, *ps;
    UINT8 *p = (UINT8 *)buffer;
    int l, i;

    pd = (struct ci_menu *)buffer;
    if (NULL == pd)
        return NULL;

    if (NULL == tp)
        return NULL;

    if ((0 > tp->cur_menu) || (CI_MMI_MENU_LEVEL_MAX <= tp->cur_menu))
        return NULL;
    ps = tp->menu_list[tp->cur_menu];
    if (tp->cur_menu >= 0)
    {
        /* Check buffer length */
        l = sizeof(struct ci_menu) + STRLEN(ps->title_text) + 1 +
            STRLEN(ps->subtitle_text) + 1 + STRLEN(ps->bottom_text) + 1;
        for (i = 0; i < ps->num_entries; i++)
        {
            l = l + STRLEN(ps->entries[i]) + 1;
        }
        if (l > len)
        {
            APPL_PRINTF("ci_mmi_get_menu: menu %d > buffer %d\n",l, len);
            return NULL;
        }
        MEMSET(buffer, 0, l);
        /* Copy menu */
        pd->mmi = ps->mmi;
        pd->selectable = ps->selectable;
        pd->num_entries = ps->num_entries;

        l = sizeof(struct ci_menu);            /* Jump over menu structure head */
        pd->title_text = (char *)&(p[l]);        /* Menu title_text copy */
        //STRCPY(&(p[l]), ps->title_text);
        if((l + STRLEN(ps->title_text) + 1) > ((UINT32)len))
            return NULL;
        strncpy((char *)&(p[l]), ps->title_text, STRLEN(ps->title_text));
        l = l + STRLEN(ps->title_text) + 1;

        pd->subtitle_text = (char *)&(p[l]);    /* Menu subtitle_text copy */
        //STRCPY(&(p[l]), ps->subtitle_text);
        if((l + STRLEN(ps->subtitle_text) + 1) > ((UINT32)len))
            return NULL;
        strncpy((char *)&(p[l]), ps->subtitle_text, STRLEN(ps->subtitle_text));
        l = l + STRLEN(ps->subtitle_text) + 1;

        pd->bottom_text = (char *)&(p[l]);/* Menu bottom_text copy */
        //STRCPY(&(p[l]), ps->bottom_text);
        if((l + STRLEN(ps->bottom_text) + 1) > ((UINT32)len))
            return NULL;
        strncpy((char *)&(p[l]), ps->bottom_text, STRLEN(ps->bottom_text));
        l = l + STRLEN(ps->bottom_text) + 1;

        for (i = 0; i < ps->num_entries; i++)
        {
            pd->entries[i] = (char *)&(p[l]);    /* Menu entries_text copy */
            //STRCPY(&(p[l]), ps->entries[i]);
            if((l + STRLEN(ps->entries[i]) + 1) > ((UINT32)len))
                return NULL;
            strncpy((char *)&(p[l]), ps->entries[i], STRLEN(ps->entries[i]));
            l = l + STRLEN(ps->entries[i]) + 1;
        }
        return buffer;
    }
    return NULL;
}

struct ci_enquiry *ci_mmi_get_enquiry(void *buffer, int len, struct ci_senssion_connect *sc)
{
    struct ci_mmi *tp = (struct ci_mmi *)(sc->appdata);
    struct ci_enquiry *pd, *ps;
    UINT8 *p = (UINT8 *)buffer;
    int l;

    pd = (struct ci_enquiry *)buffer;
    ps = tp->enquiry;
    if (tp->enquiry != NULL)
    {
        /* Check buffer length */
        l = sizeof(struct ci_enquiry) + STRLEN(ps->text) + 1;
        if (l > len)
        {
            return NULL;
        }
        MEMSET(buffer, 0, l);
        /* Copy enquiry */
        pd->mmi = ps->mmi;
        pd->blind = ps->blind;
        pd->expected_length = ps->expected_length;

        l = sizeof(struct ci_enquiry);            /* Jump over enquiry structure head */
        pd->text = (char *)&(p[l]);                /* Enquiry text copy */
        //STRCPY(&(p[l]), ps->text);
        strncpy((char *)&(p[l]), ps->text, STRLEN(ps->text));

        return buffer;
    }
    return NULL;
}

/* MMI Menu */
static struct ci_menu * ci_menu_create(struct ci_mmi *mmi, int selectable)
{
    struct ci_menu *menu;

    if ((menu = osal_memory_allocate(sizeof(struct ci_menu))) == NULL)
    {
        APPL_PRINTF("ci_menu_create: malloc faile\n");
        return NULL;
    }
    menu->mmi = mmi;
    menu->selectable = selectable;
    menu->title_text = menu->subtitle_text = menu->bottom_text = NULL;
    menu->num_entries = 0;
    return menu;
}

static void ci_menu_delete(struct ci_menu *menu)
{
    int i;

    if (menu)
    {
        if (menu->title_text)
        {
            osal_memory_free(menu->title_text);
        }
        if (menu->subtitle_text)
        {
            osal_memory_free(menu->subtitle_text);
        }
        if (menu->bottom_text)
        {
            osal_memory_free(menu->bottom_text);
        }
        for (i = 0; i < menu->num_entries; i++)
        {
            if (menu->entries[i])
            {
                osal_memory_free(menu->entries[i]);
            }
        }
        osal_memory_free(menu);
    }
}

static INT32 ci_menu_add_entry(struct ci_menu *menu, char *s)
{
    if (menu->num_entries < MAX_MENU_ENTRIES_NUM)
    {
        menu->entries[menu->num_entries++] = s;
        return SUCCESS;
    }
    return ERR_FAILUE;
}

INT32 ci_menu_select(struct ci_menu *menu, int index)
{
    if (menu->mmi && -1 <= index && index < menu->num_entries)
        return ci_mmi_send_menu_answer(menu->mmi->sc, index + 1);
    return ERR_FAILUE;
}

INT32 ci_menu_cancel(struct ci_menu *menu)
{
    return ci_menu_select(menu, -1);
}

char * ci_menu_get_title_text(struct ci_menu *menu)
{
    return menu->title_text;
}

char * ci_menu_get_subtitle_text(struct ci_menu *menu)
{
    return menu->subtitle_text;
}

char * ci_menu_get_bottom_text(struct ci_menu *menu)
{
    return menu->bottom_text;
}

char * ci_menu_get_entry(struct ci_menu *menu, int n)
{
    return n < menu->num_entries ? menu->entries[n] : NULL;
}

int ci_menu_get_entries_num(struct ci_menu *menu)
{
    return menu->num_entries;
}

int ci_menu_get_selectable(struct ci_menu *menu)
{
    return menu->selectable;
}

/* MMI Enquity */
static struct ci_enquiry * ci_enquiry_create(struct ci_mmi *mmi)
{
    struct ci_enquiry *enquiry;

    if ((enquiry = osal_memory_allocate(sizeof(struct ci_enquiry))) == NULL)
    {
        APPL_PRINTF("ci_enquiry_create: malloc faile\n");
        return NULL;
    }
    enquiry->mmi = mmi;
    enquiry->text = NULL;
    enquiry->blind = 0;;
    enquiry->expected_length = 0;;
    return enquiry;
}

static void ci_enquiry_delete(struct ci_enquiry *enquiry)
{
    if (enquiry)
    {
        if (enquiry->text)
        {
            osal_memory_free(enquiry->text);
        }
        osal_memory_free(enquiry);
    }
}

INT32 ci_enquiry_reply(struct ci_enquiry *enquiry, char *s)
{
    return enquiry->mmi ? ci_mmi_send_answer(enquiry->mmi->sc, s, 1) : ERR_FAILUE;
}

INT32 ci_enquiry_cancel(struct ci_enquiry *enquiry)
{
    return enquiry->mmi ? ci_mmi_send_answer(enquiry->mmi->sc, NULL, 0) : ERR_FAILUE;
}

char * ci_enquiry_get_text(struct ci_enquiry *enquiry)
{
    return enquiry->text;
}

int ci_enquiry_get_blind(struct ci_enquiry *enquiry)
{
    return enquiry->blind;
}

int ci_enquiry_get_expected_length(struct ci_enquiry *enquiry)
{
    return enquiry->expected_length;
}

static INT32 ci_scr_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

struct ci_senssion_connect * ci_scr_create(struct ci_senssion_connect *sc,
  int scnb, struct ci_transport_connect *tc)
{
    ci_senss_conn_init(sc, scnb, RSC_ID_SMART_CARD_READER, tc);
    sc->appdelete = NULL;
    sc->callback = ci_scr_process;
    sc->appdata = NULL;
    return sc;
}

static INT32 ci_scr_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    int tag;

    if (data)
    {
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
        case APPOBJ_TAG_SMART_CARD_CMD:
            APPL_PRINTF("ci_scr_process: %d: <== Smart Card Cmd\n", sc->scnb);
            break;
        case APPOBJ_TAG_SMART_CARD_REPLY:
            APPL_PRINTF("ci_scr_process: %d: <== Smart Card Reply\n", sc->scnb);
            break;
        case APPOBJ_TAG_SMART_CARD_SEND:
            APPL_PRINTF("ci_scr_process: %d: <== Smart Card Send\n", sc->scnb);
            break;
        case APPOBJ_TAG_SMART_CARD_REV:
            APPL_PRINTF("ci_scr_process: %d: <== Smart Card Rev\n", sc->scnb);
            break;
        default:
            APPL_PRINTF("ci_scr_process: unknown tag %06X\n", tag);
            break;
        }
    }
    return RET_SUCCESS;
}

#ifdef HD_PLUS_SUPPORT
//static void ci_hdplus_delete(struct ci_senssion_connect *sc);
static INT32 ci_hdplus_process(struct ci_senssion_connect *sc, int len, UINT8 *data);

/* Create HD Plus Session Connection */
struct ci_senssion_connect * ci_hdplus_create(struct ci_senssion_connect *sc, int scnb, struct ci_transport_connect *tc)
{
    struct ci_hd_plus *tp;

    ci_senss_conn_init(sc, scnb, RSC_ID_HD_PLUS, tc);
    sc->callback = ci_hdplus_process;
    if ((sc->appdata = osal_memory_allocate(sizeof(struct ci_hd_plus))) == NULL)
    {
        APPL_PRINTF("ci_hdplus_create: malloc faile\n");
        return NULL;
    }
    tp = (struct ci_hd_plus*)(sc->appdata);
    sc->tc->link->slot[sc->tc->slot].cam_type = CICAM_HD_PLUS;//HD+ Legacy Module
    tp->auth_protocol_id = 0;
    tp->rsa_key_index = 0;
    MEMSET(tp->session_key_seed_cicam, 0, 10);
    MEMSET(tp->nonce, 0, 8);
    MEMSET(tp->copyright_cicam, 0, 195);

    return sc;
}

/* Process for HD Plus Session */
static INT32 ci_hdplus_process(struct ci_senssion_connect *sc, int len, UINT8 *data)
{
    struct ci_hd_plus *tp = (struct ci_hd_plus *)(sc->appdata);
    UINT32 tag, l, i, clen, padding, flen;
    UINT8 *d;
    unsigned char reqfrom[64], reqto[64], respraw[64], respenc[64], respout[512];
    RSA *r;
    unsigned char session_key_seed_stb[10], copyright_stb[256], copyright_cicam[256];
    UINT16 stb_manufacturer_id;
    UINT32 len_cr_stb, len_cr_cam;

    if (data)
    {
        tag = ci_get_apdu_tag(&len, &data);
        switch (tag)
        {
            case APPOBJ_TAG_HDPLUS_AUTH_REQ:
                //Auth Request
                APPL_PRINTF("ci_hdplus_process: %d: <== HD Plus AUTH REQ\n", sc->scnb);
                l = 0;
                d = ci_senss_get_apdu(sc, &l, data);
                l = l+2;//261->263

                if ((l -= 2) < 0)
                {
                    break;
                }
                tp->auth_protocol_id = (d[0] << 8) | d[1];//auth_protocol_id is 0x1830
                d += 2;
                APPL_PRINTF("ci_hdplus_process: auth_protocol_id from CAM = 0x%x\n", tp->auth_protocol_id);

                l--;
                if(*d++ == 0x00)                        //version is 0x00
                {
                    l--;
                    tp->rsa_key_index = *d++;            //RSA_key_index is 0x00
                    APPL_PRINTF("ci_hdplus_process: RSA_key_index from CAM = 0x%x\n", tp->rsa_key_index);

                    //RSA decryption begin

                    #if 1//modulus size == 512
                    l-=64;
                    for(i=0;i<64;i++)
                    {
                        reqfrom[i] = *d++;
                    }
                    #else//modulus size == 256
                    l-=32;
                    for(i=0;i<32;i++)
                    {
                        reqfrom[i] = *d++;
                    }
                    //jump padding
                    l-=32;
                    d+=32;
                    #endif

                    r=RSA_new();
                    r->n=bn_new();
                    r->d=bn_new();
                    r->p=bn_new();
                    r->q=bn_new();
                    rsa_key_bin2bn(r->n,r->d,r->p,r->q);
                    padding = RSA_NO_PADDING;
                    //1.RSA_PKCS1_PADDING
                    //2.RSA_SSLV23_PADDING
                    //3.RSA_PKCS1_OAEP_PADDING
                    //4.RSA_NO_PADDING

                    //flen = BN_num_bytes(r->n);
                    flen = rsa_size(r);
                    APPL_PRINTF("ci_hdplus_process: RSA_private_decrypt: RSA size = %d, padding = %d\n", flen, padding);
                    clen = rsa_private_decrypt(flen,reqfrom,reqto,r,padding);

                    APPL_PRINTF("ci_hdplus_process: session_key_seed_CICAM from CAM:\n");
                    for(i=0;i<10;i++)
                    {
                        tp->session_key_seed_cicam[i] = reqto[i+1];        //session_key_seed_CICAM
                        APPL_PRINTF("%x ", tp->session_key_seed_cicam[i]);
                    }
                    APPL_PRINTF("\n");


                    APPL_PRINTF("ci_hdplus_process: nonce from CAM:\n");
                    for(i=0;i<8;i++)
                    {
                        tp->nonce[i] = reqto[i+11];                        //nonce
                        APPL_PRINTF("%x ", tp->nonce[i]);
                    }
                    APPL_PRINTF("\n");
                    //RSA decryption end

                    l-=195;
                    APPL_PRINTF("ci_hdplus_process: copyright_CICAM from CAM:\n");
                    for(i=0;i<195;i++)
                    {
                        tp->copyright_cicam[i] = *d++;                    //copyright_CICAM,"HD+ (eingetragene Marke) CA-Modul"
                        APPL_PRINTF("%x ", tp->copyright_cicam[i]);
                    }
                    APPL_PRINTF("\n");
                    //compare the received copyright_CICAM
                    sys_get_copyright(copyright_stb, &len_cr_stb, copyright_cicam, &len_cr_cam);
                    if(MEMCMP(tp->copyright_cicam, copyright_cicam, len_cr_cam) != 0)
                    {
                        APPL_PRINTF("ci_hdplus_process: %d: copyright_CICAM Mismatch!\n",sc->scnb);
                        return ERR_FAILUE;
                    }
                    APPL_PRINTF("ci_hdplus_process: %d: copyright_CICAM verify matched!\n",sc->scnb);
                    /*********************************************************/
                    APPL_PRINTF("ci_hdplus_process: Auth Response data configuring!\n");
                    //Auth Response
                    d = respout;
                    l = 0;

                    *d++ = ((tp->auth_protocol_id) & 0xff00) >> 8;
                    *d++ = (tp->auth_protocol_id) & 0xff;                //auth_protocol_id is 0x1830
                    l+=2;
                    APPL_PRINTF("ci_hdplus_process: auth_protocol_id to CAM = 0x%x\n", tp->auth_protocol_id);


                    *d++ = 0x00;                                        //version is 0x00
                    l++;

                    *d++ = tp->rsa_key_index;                            //RSA_key_index is 0x00
                    l++;

                    //encryption begin
                    respraw[0] = 0x00;

                    //session_key_seed_STB reserved for future use
                    if (rand_bytes(session_key_seed_stb, 10) <= 0)
                    {
                        APPL_PRINTF("ci_hdplus_process: %d: session_key_seed_STB rand error!\n",sc->scnb);
                    }
                    for(i=0;i<10;i++)
                    {
                        respraw[i+1] = session_key_seed_stb[i];            //session_key_seed_STB
                    }

                    APPL_PRINTF("ci_hdplus_process: nonce to CAM:\n");
                    for(i=0;i<8;i++)
                    {
                        respraw[i+11] = tp->nonce[i];                    //nonce
                        APPL_PRINTF("%x ", tp->nonce[i]);
                    }
                    APPL_PRINTF("\n");

                    //STB_manufacturer_id reserved for future use
                    stb_manufacturer_id = 0x0000;
                    respraw[19] = (stb_manufacturer_id & 0xff00) >> 8;
                    respraw[20] = stb_manufacturer_id & 0xff;            //STB_manufacturer_id

                    flen = rsa_size(r);
                    padding = RSA_NO_PADDING;
                    if(padding==RSA_PKCS1_PADDING)
                        flen-=11;
                    //else if(padding==RSA_NO_PADDING)
                    //    flen=flen;
                    APPL_PRINTF("ci_hdplus_process: RSA_private_encrypt: RSA size = %d, padding = %d\n", flen, padding);
                    clen=rsa_private_encrypt(flen,respraw,respenc,r,padding);

                    for(i=0;i<64;i++)
                    {
                        *d++ = respenc[i];
                    }
                    l+=64;

                    APPL_PRINTF("ci_hdplus_process: copyright_STB to CAM:\n");
                    for(i=0;i<len_cr_stb;i++)
                    {
                        *d++ = copyright_stb[i];
                        APPL_PRINTF("%x ", copyright_stb[i]);
                    }
                    APPL_PRINTF("\n");
                    l+=len_cr_stb;
                    //l = 2+1+1+64+195 = 263
      
                    //send response
                    if (ci_senss_send_apdu(sc, APPOBJ_TAG_HDPLUS_AUTH_RESP, l, respout) != SUCCESS)
                    {
                        APPL_PRINTF("ci_hdplus_process: %d: response send error!\n",sc->scnb);
                        return ERR_FAILUE;
                    }
                }
                else
                {
                    APPL_PRINTF("ci_hdplus_process: %d: version != 0x00, error!\n",sc->scnb);
                }

                break;
            default:
                APPL_PRINTF("ci_hdplus_process: CI hd plus: unknown tag %06X\n", tag);
                return ERR_FAILUE;
        }
    }
    return SUCCESS;
}
#endif

