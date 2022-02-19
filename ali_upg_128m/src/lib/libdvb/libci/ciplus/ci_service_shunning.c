/************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    ci_service_shunning.c
*
*    Description:    Implament the Host Service Shunning functions according CI+
*                  Spec v1.2 CH10.
*    History:
*     Date                Athor           Version            Reason
*       ==========    ======   =========    =========    =================
*    1.    2010.03.30         Steven          Ver 0.1         Create file.
*
*************************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <api/libci/ci_plus.h>

#include "ci_stack.h"

#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libsi/sie_monitor.h>

#ifdef CI_PLUS_SERVICESHUNNING

//CI+ Host Service Shunning
#define uimsbf16(data)    ((data[0]<<8)|data[1])
#define uimsbf32(data)    ((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3])


#define CI_SDT_MAX_PROGRAM_NUMBER    64    //128

#define SI_PRIVATE_DATA_SPECIFIER_DESC_TAG        0x5F
//defined in CI Plus Licensee Specification
//Supplementary CI Plus Specification: 4.2
#define CI_PRIVATE_DATA_SPECIFIER                0x00000040
#define CI_PROTECTION_DESC_TAG        0xCE
#define CI_MAX_CAM_BRAND_ID_COUNT    64


struct ci_protection_info
{
    //sdt
    UINT16 service_id;

    //ci_protection_descriptor
    UINT8 free_ci_mode_flag : 1;
    UINT8 match_brand_flag : 1;
    UINT8 number_of_entries;
    UINT16 cicam_brand_identifier[CI_MAX_CAM_BRAND_ID_COUNT];
};

//service shunning info
static struct ci_protection_info ci_service_shunning_info[CI_SDT_MAX_PROGRAM_NUMBER];
//MEMSET(&ci_service_shunning_info[CI_SDT_MAX_PROGRAM_NUMBER], 0, CI_SDT_MAX_PROGRAM_NUMBER * sizeof(struct ci_protection_info));

static int ci_service_shunning_cnt = 0;

static struct si_filter_t *ci_sdt_filter = NULL;
//SDT filter buffer
static UINT8 ciplus_sdt_buffer[SI_SHORT_SECTION_LENGTH+4];
static UINT32 ci_pre_tp_id = 0xFFFFFFFF;

UINT8 ciplus_service_shunning_timeout = 0;

//static UINT16 cicam_brand_id = 0;
#ifdef CI_PLUS_NO_SINGLE_SLOT
static int ci_slot = 1;//support 2 slot and slot 0 not ok
#else
static int ci_slot = 0;//single slot
#endif

//get private_data_specifier
static void ci_private_data_specifier_descriptor(UINT8 *data, UINT8 len, UINT32 *specifier)
{
    UINT8 descriptor_length;

    APPL_PRINTF("private_data_specifier_descriptor: %d\n",len);
    if (SI_PRIVATE_DATA_SPECIFIER_DESC_TAG != *data || len < 6)
    {
        APPL_PRINTF("private_data_specifier_descriptor: tag 0x%X error!\n",*data);
        return;
    }

    descriptor_length = data[1];
    if (len < descriptor_length + 2)
    {
        APPL_PRINTF("private_data_specifier_descriptor: bad descriptor len %d!\n",descriptor_length);
        return;
    }

    //private_data_specifier
    //The private data specifier value is defined in the
    //CI Plus Licensee Specification.
    //*specifier = uimsbf32(data);
    *specifier = (data[2]<<24) | (data[3]<<16) | (data[4]<<8) | data[5];

    APPL_PRINTF("private_data_specifier_descriptor: private_data_specifier 0x%X\n",*specifier);
}
//parse protection_descriptor
static void ci_protection_descriptor(UINT8 *data, UINT8 len, struct ci_protection_info *info)
{
    int i;
    UINT8 descriptor_length;

    APPL_PRINTF("ci_protection_descriptor: %d\n",len);
    if (CI_PROTECTION_DESC_TAG != *data || len < 3)
    {
        APPL_PRINTF("ci_protection_descriptor: tag 0x%X error!\n",*data);
        return;
    }

    descriptor_length = data[1];
    if (len < descriptor_length + 2)
    {
        APPL_PRINTF("ci_protection_descriptor: bad descriptor len %d!\n",descriptor_length);
        return;
    }
    info->free_ci_mode_flag = data[2] >> 7; //1 bslbf
    info->match_brand_flag = (data[2]>>6)&0x01;    //1 bslbf

    data += 3;
    len -= 3;
    if(info->match_brand_flag == 1)
    {
        if (len < 1)
        {
            APPL_PRINTF("ci_protection_descriptor: len %d error!\n",len);
            return;
        }
        info->number_of_entries = *data++;    //8  uimsbf
        len --;

        if (info->number_of_entries > CI_MAX_CAM_BRAND_ID_COUNT)
        {
            APPL_PRINTF("ci_protection_descriptor: number_of_entries %d > %d[max]!\n",info->number_of_entries,CI_MAX_CAM_BRAND_ID_COUNT);
            info->number_of_entries = CI_MAX_CAM_BRAND_ID_COUNT;
        }
        if (len < 2*info->number_of_entries)
        {
            APPL_PRINTF("ci_protection_descriptor: len %d error!\n",len);
            return;
        }
        for (i=0; i<info->number_of_entries; i++)
        {
            info->cicam_brand_identifier[i] = uimsbf16(data);    //16  uimsbf
            APPL_PRINTF("ci_protection_descriptor: cicam_brand_id 0x%02X\n",info->cicam_brand_identifier[i]);
            data += 2;
        }
    }
}
//parse the descriptor loop of sdt
static void ci_sdt_descriptor_loop(UINT8 *data, int len, UINT32 *specifier, struct ci_protection_info *info)
{
    APPL_PRINTF("ci_sdt_descriptor_loop: %d\n",len);
    while (len > 0)
    {
        switch (data[0])    //tag
        {
            case SI_PRIVATE_DATA_SPECIFIER_DESC_TAG:
                ci_private_data_specifier_descriptor(data, len, specifier);
                break;
            case CI_PROTECTION_DESC_TAG:
                ci_protection_descriptor(data, len, info);
                break;
        }

        len -= (data[1]+2);
        data += (data[1]+2);
    }
}
//parse sdt for ci plus Host Service Shunning function
static void ciplus_sdt_parser(UINT8 *data, int len, struct ci_protection_info *info, int *num)
{
    int n = 0, max_num = *num;
    //struct ci_protection_info info[CI_SDT_MAX_PROGRAM_NUMBER];
    UINT16 service_id;
    UINT32 specifier;
    UINT16 descriptors_loop_length;

    APPL_PRINTF("ciplus_sdt_parser: data len %d, max_num = %d\n",len, *num);
    *num = 0;
    if (data == NULL)
        return;

    data += 11;
    len -= 11;

    //service loop
    while (len > 0)
    {
        service_id = uimsbf16(data);    //16 uimsbf
        data += 3;
        len -= 3;
//        reserved_future_use 6 bslbf
//        EIT_schedule_flag 1 bslbf
//        EIT_present_following_flag 1 bslbf
//        running_status 3 uimsbf
//        free_CA_mode 1 bslbf
        descriptors_loop_length = uimsbf16(data) & 0x0FFF;    //12 uimsbf
        data += 2;
        len -= 2;
        if (descriptors_loop_length > len)
            break;
        //descriptor loop
        specifier = 0;
        ci_sdt_descriptor_loop(data, descriptors_loop_length, &specifier, &info[n]);

        if (specifier != CI_PRIVATE_DATA_SPECIFIER)
        {
            info[n].service_id = service_id;
            n ++;
            APPL_PRINTF("ciplus_sdt_parser: service 0x%X not find CI_PRIVATE_DATA_SPECIFIER\n",service_id);
        }
        else
        {
            info[n].service_id = service_id;
            n ++;
            APPL_PRINTF("ciplus_sdt_parser: service[%d] - 0x%X\n",n,service_id);
            if (n >= max_num)
            {
               *num = n;
                APPL_PRINTF("ciplus_sdt_parser: service num %d >= max num\n",n);
                return;
            }
        }

        data += descriptors_loop_length;
        len -= descriptors_loop_length;
    }

    *num = n;
    APPL_PRINTF("ciplus_sdt_parser: service num = %d\n",n);
}

static UINT32 get_crc(UINT8* section, UINT16 sec_len)
{
    return (section[sec_len - 1] | section[sec_len] << 8 | section[sec_len + 1] << 16 | section[sec_len + 2] << 24);
}

//when factory set, need reset pre_sdt_crc too!
#define PRE_SDTCRC_MAXSIZ 16
static UINT32 pre_sdt_crc[PRE_SDTCRC_MAXSIZ] = {0};
static UINT8 ss_sdt_cnt = 0xFF;
static BOOL ss_need_send_msg_to_app(UINT8 section_cnt)
{
    UINT8 i, flag = 1;

    for (i=0; i<section_cnt; i++)
        if (pre_sdt_crc[i] == 0)
            flag = 0; // not all SDT section recv
    if((section_cnt == 0) || flag) {
//        libc_printf("%s, return TRUE\n", __FUNCTION__);
        return TRUE;
    }
//    libc_printf("%s, return FALSE\n", __FUNCTION__);
    return FALSE;
}

static BOOL ss_need_parse_sdt(UINT8 sec_num, UINT32 crc, UINT32 tpid)
{
    if ((pre_sdt_crc[sec_num] != crc) || (ci_pre_tp_id != tpid)) {
//        libc_printf("%s, return TRUE\n", __FUNCTION__);
        return TRUE;
    }
//    libc_printf("%s, return FALSE\n", __FUNCTION__);
    return FALSE;
}
static INT32 ci_sdt_callback(UINT8 *data, INT32 len, UINT32 param)
{
    struct sim_cb_param *cb_param = (struct sim_cb_param *)param;
    UINT16 sec_len;
    UINT32 sdt_crc;
    UINT8  section_cnt;
    UINT8  section_num;
    if (data)
    {
        sec_len = ((data[1] & 0x0f) << 8) | data[2];
        sdt_crc = get_crc(data, sec_len);
        section_num = data[6];
        section_cnt = data[7];

        if ((ss_sdt_cnt != section_cnt) || (ci_pre_tp_id != cb_param->param))
            MEMSET(pre_sdt_crc, 0, sizeof(UINT32)*PRE_SDTCRC_MAXSIZ);

        if (ss_need_parse_sdt(section_num, sdt_crc, cb_param->param))//fix issue: (pre_sdt_crc != sdt_crc) sometimes do not meet after search
        {
            ci_service_shunning_cnt = CI_SDT_MAX_PROGRAM_NUMBER;
            ciplus_sdt_parser(data, len, ci_service_shunning_info, &ci_service_shunning_cnt);

            pre_sdt_crc[section_num] = sdt_crc;

            //ci_pre_tp_id = current SDT's tp id;
            //...
            ci_pre_tp_id = cb_param->param;

            //send sdt update msg to ap
            if ( ss_need_send_msg_to_app(section_cnt)) {
                api_ci_msg_to_app(ci_slot, API_MSG_SERVICE_SHUNNING_UPDATE);
            }
            ss_sdt_cnt = section_cnt;
        }
    }

    return 0;
}

#if 0
static si_handler_t ci_sdt_callback(UINT16 pid, struct si_filter_t *filter,
                                UINT8 reason, UINT8 *data, INT32 len)
{
    UINT16 sec_len;
    static UINT32 pre_sdt_crc = 0xFFFFFFFF;
    UINT32 sdt_crc;

    if (data)
    {
        sec_len = ((data[1] & 0x0f) << 8) | data[2];
        sdt_crc = get_crc(data, sec_len);

        if (pre_sdt_crc != sdt_crc)
        {
            ci_service_shunning_cnt = CI_SDT_MAX_PROGRAM_NUMBER;
            ciplus_sdt_parser(data, len, ci_service_shunning_info, &ci_service_shunning_cnt);

            pre_sdt_crc = sdt_crc;

            //ci_pre_tp_id = current SDT's tp id;
            //...
            ci_pre_tp_id = (UINT32)filter->priv[0];

            //send sdt update msg to ap
            //...
            api_ci_msg_to_app(ci_slot, API_MSG_SERVICE_SHUNNING_UPDATE);
        }
    }
}
#endif

//get CICAM Service Shunning Operating Mode
//Table 10.2: CI Plus Protected Service modes.
static enum ci_protect_mode ci_get_protect_mode(struct ci_protection_info *info, enum ci_cam_type cicam_type, UINT16 brand_id)
{
    int i;

    ASSERT(info != NULL);

    if (info->free_ci_mode_flag == 0)
    {
        return CI_PROTECTED_IN_ACTIVE;
    }
    else // (free_ci_mode_flag == 1)
    {
        if (CICAM_CI_PLUS == cicam_type)
        {
            if (info->match_brand_flag == 0 || info->number_of_entries == 0)
                return CI_PROTECTED_IN_ACTIVE;
            else    // (match_brand_flag == 1 && number_of_entries > 0)
            {
                for (i=0; i<info->number_of_entries; i++)
                {
                    if (info->cicam_brand_identifier[i] == brand_id)
                        return CI_PROTECTED_IN_ACTIVE;
                }
                return CI_PROTECTED_ACTIVE;
            }
        }
        else    //CICAM_DVB_CI == CICAM_type
        {
            return CI_PROTECTED_ACTIVE;
        }
    }

    return CI_PROTECTED_ACTIVE;
}
static enum ci_protect_mode ci_get_service_protect_mode(UINT16 service_id, struct ci_protection_info *info, int n, enum ci_cam_type cicam_type, UINT16 brand_id)
{
    int i;

    for (i=0; i<n; i++)
    {
        if (service_id == info[i].service_id && info[i].free_ci_mode_flag != 0)
            return ci_get_protect_mode(&info[i], cicam_type, brand_id);
    }

    return CI_PROTECTED_IN_ACTIVE;
}

/**
 * This function use to acquire current received SDT's service list.
 * It will be called after ci_sdt_callback send update msg by APP.
 * @param service_list, The buffer provided by Caller, use to save service list.
 * @param len, The buffer length of servcie_list
 * @return the service count in servcie list.
 */
INT32 api_ci_get_sdt_service_list(UINT16 *service_list, UINT16 len)
{
    UINT32 i;
    if (service_list == 0)
        return 0;

    for (i=0; i<ci_service_shunning_cnt && i<len; i++) {
        service_list[i] = ci_service_shunning_info[i].service_id;
    }
    return i;
}

/**
 * This function use to acquire current received SDT's service's count
 * It should be called before call ci_get_sdt_service_list
 * @return The number of current received SDT's service
 */
UINT32 api_ci_get_sdt_service_count()
{
    return ci_service_shunning_cnt;
}

#define CI_SIM_INVALID_MON_ID    0xFFFFFFFF

UINT32 g_ciplus_sdt_monitor_id = CI_SIM_INVALID_MON_ID;
static UINT32 prev_tp_id = 0xFFFFFFFF;

static BOOL need_start_new_filter(UINT32 tp_id)
{
    if ((g_ciplus_sdt_monitor_id == CI_SIM_INVALID_MON_ID)
        ||(prev_tp_id != 0xFFFFFFFF && prev_tp_id != tp_id))
        return TRUE;

    return FALSE;
}
INT32 ci_start_sdt_filter(UINT32 tp_id)
{
    struct dmx_device * dmx;

    APPL_PRINTF("%s\n\n",__FUNCTION__);

    if (need_start_new_filter(tp_id)==FALSE)
        /* no need start filter */
        return 0;

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    if (g_ciplus_sdt_monitor_id != CI_SIM_INVALID_MON_ID) {
        sim_stop_monitor(g_ciplus_sdt_monitor_id);
        g_ciplus_sdt_monitor_id = CI_SIM_INVALID_MON_ID;
    }

    g_ciplus_sdt_monitor_id = sim_start_monitor(dmx, MONITE_TB_SDT, SI_SDT_PID, tp_id);

    if(g_ciplus_sdt_monitor_id == CI_SIM_INVALID_MON_ID)
    {
        return -1;//faild to start sdt monitor
    }

    sim_register_scb(g_ciplus_sdt_monitor_id, ci_sdt_callback, tp_id);

    return 0;
}

//static INT32 ci_stop_sdt_filter()
INT32 ci_stop_sdt_filter()
{
    APPL_PRINTF("%s\n\n",__FUNCTION__);

    if (g_ciplus_sdt_monitor_id != CI_SIM_INVALID_MON_ID)
    {
        sim_stop_monitor(g_ciplus_sdt_monitor_id);
        g_ciplus_sdt_monitor_id = CI_SIM_INVALID_MON_ID;
    }

    return 0;
}

#if 0
INT32 ci_start_sdt_filter(UINT32 tp_id)
{
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;
    INT32 ret;

    APPL_PRINTF("%s - tp %X\n\n",__FUNCTION__,tp_id);
    if (tp_id != ci_pre_tp_id)
    {
        //reset service shunning info
        ci_service_shunning_cnt = 0;
        ci_pre_tp_id = 0xFFFFFFFF;
    }

    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    fparam.timeout = OSAL_WAIT_FOREVER_TIME;
    fparam.attr[0] = SI_ATTR_HAVE_CRC32;
    fparam.section_event = NULL;
    fparam.section_parser = (si_handler_t)ci_sdt_callback;
    {
        fparam.mask_value.mask_len = 6;
        fparam.mask_value.mask[0] = 0xFF;
        fparam.mask_value.mask[1] = 0x80;
        fparam.mask_value.mask[5] = 0x01;
        fparam.mask_value.value_num = 1;
        fparam.mask_value.value[0][0] = SI_SDT_TABLE_ID;
        fparam.mask_value.value[0][1] = 0x80;
        fparam.mask_value.value[0][5] = 0x01;
    }

    filter = sie_alloc_filter(SI_SDT_PID, ciplus_sdt_buffer, 1024,1024);
    filter->priv[0] = (void *)tp_id;

    if(filter != NULL)
    {
        sie_config_filter(filter,&fparam);

        ret = sie_enable_filter(filter);
        if (SI_SUCCESS == ret)
        {
          APPL_PRINTF("%s - enable SDT filter success.\n\n",__FUNCTION__);
          ci_sdt_filter = filter;
        }
        else
        {
          APPL_PRINTF("%s - enable SDT filter failed!\n\n",__FUNCTION__);
        }
    }
    else
    {
      APPL_PRINTF("%s - alloc SDT filter failed!\n\n",__FUNCTION__);
    }
}
INT32 ci_stop_sdt_filter()
{
    if (ci_sdt_filter)
    {
        //sie_abort_filter(ci_sdt_filter);
        sie_abort(SI_SDT_PID, NULL);
        ci_sdt_filter = NULL;
        APPL_PRINTF("%s - stop SDT filter success.\n\n",__FUNCTION__);
    }
}
#endif
/*
INT32 ci_deliver_brandid(UINT16 cam_bid)
{
    cicam_brand_id = cam_bid;
}
*/
enum ci_protect_mode ci_service_shunning(UINT32 tp_id, UINT16 service_id)    //20100804, fix service_id UINT8 to UINT16
{
    enum ci_cam_type cicam_type;
    enum ci_protect_mode ret = CI_PROTECTED_ACTIVE;
    UINT16 cicam_brand_id = 0;

    APPL_PRINTF("%s - tp %X, service %X\n\n",__FUNCTION__,tp_id,service_id);
    if (ci_pre_tp_id == tp_id) //already got SDT
    {
        //get brand_id;
        //get CICAM_type;
        cicam_type = api_ci_get_cam_type(ci_slot);
        if(cicam_type == CICAM_CI_PLUS)
            cicam_brand_id = api_ci_get_cicam_brand_id(ci_slot);

        ret = ci_get_service_protect_mode(service_id, ci_service_shunning_info, ci_service_shunning_cnt,
                       cicam_type, cicam_brand_id);

        if (CI_PROTECTED_IN_ACTIVE == ret)
        {
             ci_del_timer((ci_timer_handler)api_ci_msg_to_app, (void*)ci_slot,
                          (void*)API_MSG_SERVICE_SHUNNING_UPDATE);
             ciplus_service_shunning_timeout = 0;
        }
    }
//20100805
/*    else//not got SDT    //always start this filter?    20100803
    {
        ci_start_sdt_filter(tp_id);
    }*/

    return ret;
    }

void ci_service_shunning_reset()
{
    ci_pre_tp_id = 0xFFFFFFFF;
    //when factory set, need reset pre_sdt_crc too!
//    pre_sdt_crc = 0xFFFFFFFF;
    MEMSET(pre_sdt_crc, 0, sizeof(UINT32)*PRE_SDTCRC_MAXSIZ);
}
//CI+ Host Service Shunning

#endif

