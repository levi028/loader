/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_epg.c
*
*    Description: include main function of EPG module
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/sie.h>
#include "epg_common.h"
#include <api/libsi/lib_epg.h>
#include <api/libsi/si_eit.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libsi/lib_multifeed.h>
#include "eit_parser.h"
#include "epg_db.h"
#include "x_alloc.h"
#include "../libsi/libtsi3/plugin/eit/sie_eit.h"
#include <api/libpub/lib_frontend.h>

//#define _DEBUG
#define INVALID_TP_ID   0xFFFFFFFF
#define RESERVED_VER        0x3F
//section status
#define VERSION_MASK        0xCF

typedef enum
{
    NONE                 = 0x00,
    SECTION_EXIST        = 0xFF,
    ERROR_NO_SERVICE     = 0x01,
    ERROR_PF_SECTION_NUM = 0x02,
    ERROR_DB_FULL        = 0x03,

    PF_SECTION_UPDATE    = 0x00, // bit 4~5: [0,0]
    PF_VERSION_UPDATE    = 0x10, //[0,1]
    SCH_SECTION_UPDATE   = 0x20, //[1,0]
    SCH_VERSION_UPDATE   = 0x30, //[1,1]

} section_status;

//sevice section bitmap
struct SERVICE_T
{
    UINT32 tp_id;
    UINT16 service_id;

    UINT8  pf_sec_mask: 2;
    UINT8  pf_ver: 6;

    //bit map for handle
    UINT8  pf_sec_mask_h: 2;
    UINT8  pf_ver_h: 6;

    UINT8  sch_sec_mask[SCHEDULE_TABLE_ID_NUM][32];    // 32 segment , each 8 section.
    UINT8  sch_ver[SCHEDULE_TABLE_ID_NUM];             //a table, a version

    //bit map for handle
    UINT8  sch_sec_mask_h[SCHEDULE_TABLE_ID_NUM][32];  // 32 segment , each 8 section.
    UINT8  sch_ver_h[SCHEDULE_TABLE_ID_NUM];           //a table, a version
};

//sevice list index
struct SERVICE_INDEX_T
{
    UINT32 tp_id;
    UINT16 service_id;
    UINT16 idx;
};

//epg control block, for sie & ap callback
static struct epg_info_t
{
    UINT8* buffer;          //buffer for sie's filter
    UINT32 buf_len;
    BOOL   internal_buf;    //internal alloced buffer or external buffer

    struct sie_eit_config config;
    UINT32 tp_id;
    UINT16 service_id;

    EIT_CALL_BACK epg_call_back;

    //service list of current tp
    struct SERVICE_T *service;  //sort by service_id from small to big
    struct SERVICE_INDEX_T *service_idx;
    UINT8 service_num;

#ifdef EPG_MULTI_TP
    UINT8 service_id_num;
    UINT16 service_ids[TP_MAX_SERVICE_COUNT];   //service ids of current TP
#endif

#ifdef EPG_OTH_SCH_SUPPORT
    UINT16 pre_sat_id;
    UINT16 tp_id_num;
    UINT32 tp_ids[1024][2];     //tp_id table of a sat.
                                //[0]: (ts_id, o_net_id)
                                //[1]: tp_id
#endif

} epg_info;

static OSAL_ID  lib_epg_flag = OSAL_INVALID_ID;

//epg module status
static  enum EPG_STATUS epg_status = STATUS_NOT_READY;

#ifdef EPG_MULTI_TP
#define EPG_UPDATE_VIEW_SIZE 512
static UINT32 epg_update_view_buff[EPG_UPDATE_VIEW_SIZE] = {0};
#endif

//set the active services
static UINT8 active_service_cnt = 0;
static struct ACTIVE_SERVICE_INFO active_service[MAX_ACTIVE_SERVICE_CNT];

void epg_enter_mutex(void)
{
    UINT32 flag = 0;

    osal_flag_wait(&flag, lib_epg_flag, EPG_FLAG_MUTEX, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
void epg_leave_mutex(void)
{
    osal_flag_set(lib_epg_flag, EPG_FLAG_MUTEX);
}

void epg_api_set_select_time(date_time *cur_dt, date_time *start_dt, date_time *end_dt)
{
    epg_set_select_time(cur_dt, start_dt, end_dt);
}

static void reset_epg_internal(void)
{
    INT32 ret = ERR_FAILED;

    EPG_PRINT("reset_epg_internal\n\n");
    ret = reset_epg_db();
    ret = x_reset_mem();
    epg_info.service_num = 0;

    if (SUCCESS == ret)
    {
        return;
    }
}

INT32 epg_reset(void)
{
    if (STATUS_OFF != epg_status)
    {
        EPG_PRINT("epg_release - status: %d error!\n",epg_status);
        return !SUCCESS;
    }

    epg_enter_mutex();
    reset_epg_internal();

#ifdef EPG_MULTI_TP
    epg_info.service_id_num = 0;
#endif
#ifdef EPG_OTH_SCH_SUPPORT
    epg_info.tp_id_num = 0;
    epg_info.pre_sat_id = 0;
#endif

    epg_leave_mutex();

    return SUCCESS;
}

#ifdef EPG_MULTI_TP
static struct SERVICE_T *get_service(UINT32 tp_id, UINT16 service_id, UINT8 *idx)
{
    int m = 0;
    int n = 0;
    int mid = 0;

    if ((NULL == idx) || (0 == epg_info.service_num))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }
    if ((tp_id < epg_info.service_idx[0].tp_id) ||
        ((tp_id == epg_info.service_idx[0].tp_id) && (service_id < epg_info.service_idx[0].service_id)))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }
    if ((tp_id > epg_info.service_idx[epg_info.service_num-1].tp_id) ||
        ((tp_id == epg_info.service_idx[epg_info.service_num-1].tp_id) &&
        (service_id > epg_info.service_idx[epg_info.service_num-1].service_id)))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }

    if (1 == epg_info.service_num)
    {
        if((tp_id == epg_info.service_idx[0].tp_id) && (service_id == epg_info.service_idx[0].service_id))
        {
            *idx = epg_info.service_idx[0].idx;
            return &epg_info.service[epg_info.service_idx[0].idx];
        }
        else
        {
            MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
            return NULL;
        }
    }

    //Dichotomy Quick Search
    m = 0;
    n = epg_info.service_num - 1;
    if((tp_id == epg_info.service_idx[m].tp_id) && (service_id == epg_info.service_idx[m].service_id))
    {
        *idx = epg_info.service_idx[m].idx;
        return &epg_info.service[epg_info.service_idx[m].idx];
    }
    if((tp_id == epg_info.service_idx[n].tp_id) && (service_id == epg_info.service_idx[n].service_id))
    {
        *idx = epg_info.service_idx[n].idx;
        return &epg_info.service[epg_info.service_idx[n].idx];
    }

    do
    {
        mid = (m + n) / 2;

        if ((mid == m) || (mid == n))
        {
            break;
        }
        if ((tp_id == epg_info.service_idx[mid].tp_id) && (service_id == epg_info.service_idx[mid].service_id))
        {
            *idx = epg_info.service_idx[mid].idx;
            return &epg_info.service[epg_info.service_idx[mid].idx];
        }
        else if ((tp_id > epg_info.service_idx[mid].tp_id) ||
            ((tp_id == epg_info.service_idx[mid].tp_id) && (service_id > epg_info.service_idx[mid].service_id)))
        {
            m = mid;
        }
        else
        {
            n = mid;
        }

    } while (1);

    return NULL;
}

static void sort_service(struct SERVICE_INDEX_T *service_idx, UINT8 cnt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    struct SERVICE_INDEX_T serv;

    MEMSET(&serv, 0, sizeof(struct SERVICE_INDEX_T));

    if ((NULL == service_idx) || (cnt <= 1))
    {
        return;
    }
    for (i=0; i<cnt-1; i++)
    {
        k = i;
        for (j=i+1; j<cnt; j++)
        {
            if ((service_idx[j].tp_id < service_idx[k].tp_id)
                || ((service_idx[j].tp_id == service_idx[k].tp_id)
                && (service_idx[j].service_id < service_idx[k].service_id)))
            {
                k = j;
            }
        }
        if (k != i)
        {
            MEMCPY(&serv, &service_idx[i], sizeof(struct SERVICE_INDEX_T));
            MEMCPY(&service_idx[i], &service_idx[k], sizeof(service_idx[i]));
            MEMCPY(&service_idx[k], &serv, sizeof(service_idx[k]));
        }
    }
}

#else
static struct SERVICE_T* get_service(UINT32 tp_id, UINT16 service_id, UINT8 *idx)
{
    UINT8 m = 0;
    UINT8 n = 0;
    UINT8 mid = 0;

    if ((NULL = idx) || (0 == epg_info.service_num))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }

    if ((tp_id < epg_info.service[0].tp_id)
        || ((tp_id == epg_info.service[0].tp_id) && (service_id < epg_info.service[0].service_id)))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }

    if ((tp_id > epg_info.service[epg_info.service_num-1].tp_id)
        || ((tp_id == epg_info.service[epg_info.service_num-1].tp_id)
        && (service_id > epg_info.service[epg_info.service_num-1].service_id)))
    {
        MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
        return NULL;
    }

    if (1 == epg_info.service_num)
    {
        if((tp_id == epg_info.service[0].tp_id) && (service_id == epg_info.service[0].service_id))
        {
            *idx = 0;
            return &epg_info.service[0];
        }
        else
        {
            MSG_PRINT("[%s]: not find tp 0x%X, service 0x%X\n",__FUNCTION__,tp_id,service_id);
            return NULL;
        }
    }

    //Dichotomy Quick Search
    m = 0;
    n = epg_info.service_num - 1;
    if((tp_id == epg_info.service[m].tp_id) && (service_id == epg_info.service[m].service_id))
    {
        *idx = m;
        return &epg_info.service[m];
    }
    if((tp_id == epg_info.service[n].tp_id) && (service_id == epg_info.service[n].service_id))
    {
        *idx = n;
        return &epg_info.service[n];
    }
    do
    {
        mid = (m + n) / 2;

        if ((mid == m) || (mid == n))
        {
            break;
        }
        if ((tp_id == epg_info.service[mid].tp_id) && (service_id == epg_info.service[mid].service_id))
        {
            *idx = mid;
            return &epg_info.service[mid];
        }
        else if ((tp_id > epg_info.service[mid].tp_id)
                || ((tp_id == epg_info.service[mid].tp_id) && (service_id > epg_info.service[mid].service_id)))
        {
            m = mid;
        }
        else
        {
            n = mid;
        }
    } while (1);

    return NULL;
}

//sort service list by tp_id & service_id
static void sort_service(struct SERVICE_T *service, UINT8 cnt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    struct SERVICE_T serv;

    if ((NULL == service) || (cnt <= 1))
    {
        return;
    }
    for (i=0; i<cnt-1; i++)
    {
        k = i;
        for (j=i+1; j<cnt; j++)
        {
            if ((service[j].tp_id < service[k].tp_id)
                || ((service[j].tp_id == service[k].tp_id)
                && (service[j].service_id < service[k].service_id)))
            {
                k = j;
            }
        }
        if (k != i)
        {
            MEMCPY(&serv, &service[i], sizeof(struct SERVICE_T));
            MEMCPY(&service[i], &service[k], sizeof(service[i]));
            MEMCPY(&service[k], &serv, sizeof(service[k]));
        }
    }
}
#endif


#ifdef EPG_MULTI_TP
static BOOL add_service(UINT32 tp_id, UINT16 service_id)
{
    struct SERVICE_T *service = epg_info.service;
    struct SERVICE_INDEX_T *service_idx = epg_info.service_idx;

    if (epg_info.service_num >= TP_MAX_SERVICE_COUNT)
    {
        MSG_PRINT("add_service: service full!!\n");
        return FALSE;
    }

    if (service != NULL)
    {
        service[epg_info.service_num].tp_id = tp_id;
        service[epg_info.service_num].service_id = service_id;
        service[epg_info.service_num].pf_sec_mask = 0;
        service[epg_info.service_num].pf_ver = RESERVED_VER;
        service[epg_info.service_num].pf_sec_mask_h = 0;
        service[epg_info.service_num].pf_ver_h = RESERVED_VER;
        MEMSET(service[epg_info.service_num].sch_sec_mask, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[epg_info.service_num].sch_ver, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[epg_info.service_num].sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[epg_info.service_num].sch_ver_h, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);

        if(service_idx != NULL)
        {
            service_idx[epg_info.service_num].tp_id = tp_id;
            service_idx[epg_info.service_num].service_id = service_id;
            service_idx[epg_info.service_num].idx = epg_info.service_num;
        }
        epg_info.service_num++;
        MSG_PRINT("add_service: add new service - tp: 0x%X, service: 0x%X\n",tp_id, service_id);
    }
    else
    {
        return FALSE;
    }

    sort_service(service_idx, epg_info.service_num);

    return TRUE;
}

static BOOL check_service(UINT16 service_id)
{
    int m = 0;
    int n = 0;
    int mid = 0;

    if (0 == epg_info.service_id_num)
    {
        return FALSE;
    }
    if (1 == epg_info.service_id_num)
    {
        if (epg_info.service_ids[0] == service_id)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    //Dichotomy Quick Search
    m = 0;
    n = epg_info.service_id_num - 1;

    if((epg_info.service_ids[m] == service_id) || (epg_info.service_ids[n] == service_id))
    {
        return TRUE;
    }

    do
    {
        mid = (m + n) / 2;

        if ((mid == m) || (mid == n))
        {
            break;
        }

        if (epg_info.service_ids[mid] == service_id)
        {
            return TRUE;
        }
        else if (service_id > epg_info.service_ids[mid])
        {
            m = mid;
        }
        else
        {
            n = mid;
        }
    } while (1);

    MSG_PRINT("check_service: not find service 0x%X\n",service_id);
    return FALSE;
}
#endif

static void reset_bitmap(void)
{
    struct SERVICE_T *service = NULL;
    UINT32 num = 0;
    UINT32 i = 0;

    service = epg_info.service;
    num = epg_info.service_num;

    for(i=0; i<num; i++)
    {
        service[i].pf_sec_mask = 0;
        service[i].pf_ver = 0;
        service[i].pf_sec_mask_h= 0;
        service[i].pf_ver_h= 0;

        MEMSET(service[i].sch_sec_mask, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[i].sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[i].sch_ver,  0, SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[i].sch_ver_h,0, SCHEDULE_TABLE_ID_NUM);
    }
}

//check if the section already exists
static section_status find_section(UINT32 tp_id,UINT16 service_id,UINT8 table_id,UINT8 ver,UINT8 sec_num,UINT8 *idx)
{
    UINT8 table_num = 0;    //table number(0-15)
    UINT8 seg = 0;          //segment number(8 section is 1 segment)
    UINT8 seg_sec_num = 0;  //section number within segment(0-7)
    struct SERVICE_T *service = NULL;

    service = get_service(tp_id, service_id, idx);

    if (NULL == service)
    {
#ifdef EPG_MULTI_TP
        //if it's other schedule, add it no check if this service
        //is within that TP, OR check if this service is within that TP???
        if (IS_OTH_SCH(table_id) || IS_OTH_PF(table_id) || check_service(service_id))
        {
            //NOTE: only add service this time. parse this section next time.
            if (!add_service(tp_id, service_id))
            {
                return ERROR_DB_FULL;
            }
        }
#endif
        return ERROR_NO_SERVICE;
    }

#ifdef EPG_MULTI_TP
    // check need update all data or not
    if(TRUE == epg_need_update_all())
    {
        reset_bitmap();
        epg_set_update();
    }
#endif

    if (IS_PF(table_id))
    {
#ifdef _DEBUG
        if (sec_num > 1)
        {
            EPG_PRINT("sec_num: %d overflow!\n",sec_num);
            return ERROR_PF_SECTION_NUM;
        }
#endif
        if (ver != service->pf_ver)
        {
            if (service->pf_ver != RESERVED_VER)
            {
                service->pf_ver = ver;
                service->pf_sec_mask = 0;
                service->pf_sec_mask |= (1<<sec_num);
                EPG_PRINT("service: 0x%X, pf: version update %d\n",service_id,ver);
                return PF_VERSION_UPDATE;
            }

            service->pf_ver = ver;
        }
        if (0 == ((1<<sec_num) & service->pf_sec_mask))
        {
            service->pf_sec_mask |= (1<<sec_num);
            MSG_PRINT("service: 0x%X, pf: section %d\n",service_id,sec_num);
            return PF_SECTION_UPDATE;
        }
    }
    else    //sch
    {
        table_num = table_id & 0x0F;

        if (table_num < SCHEDULE_TABLE_ID_NUM)
        {
            if (ver != service->sch_ver[table_num])
            {
                if (service->sch_ver[table_num] != RESERVED_VER)
                {
                    service->sch_ver[table_num] = ver;
                    MEMSET(service->sch_sec_mask[table_num], 0, 32);
                    seg = sec_num >> 3;
                    seg_sec_num = sec_num & 0x07;
                    service->sch_sec_mask[table_num][seg] |= (1<<seg_sec_num);
                    EPG_PRINT("service: 0x%X, sch[%X]: version update %d\n",service_id,table_id,ver);
                    return SCH_VERSION_UPDATE;
                }

                service->sch_ver[table_num] = ver;
            }

            seg = sec_num >> 3;
            seg_sec_num = sec_num & 0x07;
            if (0 == ((1<<seg_sec_num) & service->sch_sec_mask[table_num][seg]))
            {
                service->sch_sec_mask[table_num][seg] |= (1<<seg_sec_num);
                MSG_PRINT("service: 0x%X, table: 0x%X, sch: section %d\n",service_id, table_id, sec_num);
                return SCH_SECTION_UPDATE;
            }
        }
        else
        {
            EPG_PRINT("table_id 0x%X overflow!\n",table_id);
        }
    }

    return SECTION_EXIST;
}

INT32 retrieve_eit_pf(UINT32 tp_id, UINT16 service_id)
{
    UINT8 idx = 0;
    struct SERVICE_T *service = NULL;

    service = get_service(tp_id, service_id, &idx);

    if(NULL == service)
    {
        return ERR_FAILED;
    }

    service->pf_ver         =   0;
    service->pf_ver_h       =   0;
    service->pf_sec_mask    =   0;
    service->pf_sec_mask_h  =   0;
    return SUCCESS;
}

//copy bitmap from handle to event
static void restore_bitmap(struct SERVICE_T *service, UINT32 num)
{
    UINT32 i = 0;

    if(NULL != service)
    {
        for(i=0; i<num; i++)
        {
            service[i].pf_sec_mask = service[i].pf_sec_mask_h;
            service[i].pf_ver = service[i].pf_ver_h;
            MEMCPY(service[i].sch_sec_mask, service[i].sch_sec_mask_h, 32 * SCHEDULE_TABLE_ID_NUM);
            MEMCPY(service[i].sch_ver, service[i].sch_ver_h, SCHEDULE_TABLE_ID_NUM);
        }
    }
}

//set handle bitmap
static void set_bitmap(UINT8 table_id, UINT8 ver, UINT8 sec_num, UINT8 service_idx)
{
    UINT8 table_num = 0;    //table number(0-15)
    UINT8 seg = 0;          //segment number(8 section is 1 segment)
    UINT8 seg_sec_num = 0;  //section number within segment(0-7)
    struct SERVICE_T *service = NULL;

    service = &epg_info.service[service_idx];
    //ASSERT(service_idx<epg_info.service_num);

    if (IS_PF(table_id))
    {
        if (ver != service->pf_ver_h)
        {
            if (service->pf_ver_h != RESERVED_VER)
            {
                service->pf_sec_mask_h = 0;
            }
            service->pf_ver_h = ver;
        }

        service->pf_sec_mask_h |= (1<<sec_num);
    }
    else    //sch
    {
        table_num = table_id & 0x0F;
        //ASSERT(table_num<SCHEDULE_TABLE_ID_NUM);

        if (table_num < SCHEDULE_TABLE_ID_NUM)
        {
            if (ver != service->sch_ver_h[table_num])
            {
                if (service->sch_ver_h[table_num] != RESERVED_VER)
                {
                    MEMSET(service->sch_sec_mask_h[table_num], 0, 32);
                }
                service->sch_ver_h[table_num] = ver;
            }

            seg = sec_num >> 3;
            seg_sec_num = sec_num & 0x07;

            service->sch_sec_mask_h[table_num][seg] |= (1<<seg_sec_num);
        }
        else
        {
            EPG_PRINT("table_id 0x%X overflow!\n",table_id);
        }
    }
}

#ifdef EPG_MULTI_TP
static UINT8 create_service_list(UINT32 tp_id, UINT16 *service_ids, UINT8 last_cnt, UINT8 max_cnt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    UINT16 s_id = 0;
    UINT8 cnt = last_cnt;
    UINT32 pos = 0;
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));

    if (NULL == service_ids)
    {
        return 0;
    }

    EPG_PRINT("\ncreate_service_list - tp: 0x%X\n",tp_id);
    //find all
    #ifdef DB_SUPPORT_HMAC
    //for 5000 prog num ,channel change
    while ((cnt < max_cnt) && (SUCCESS == get_prog_at_ncheck(pos, &p_node)))
    #else
    while ((cnt < max_cnt) && (SUCCESS == get_prog_at(pos, &p_node)))
    #endif
    {
        if (p_node.tp_id == tp_id)
        {
            if ((0 == last_cnt) || (!check_service(p_node.prog_number)))  //not exist
            {
                service_ids[cnt] = p_node.prog_number;
                cnt++;
                EPG_PRINT("create_service_list - add service: 0x%X\n",p_node.prog_number);
            }
        }
        pos++;
    }

    if (0 == cnt)
    {
        return 0;
    }

    if (cnt != last_cnt)    //add some new programs
    {
        //sort
        for (i=0; i<cnt-1; i++)
        {
            k = i;
            for (j=i+1; j<cnt; j++)
            {
                if (service_ids[j] < service_ids[k])
                {
                    k = j;
                }
            }
            if (k != i)
            {
                s_id = service_ids[i];
                service_ids[i] = service_ids[k];
                service_ids[k] = s_id;
            }
        }
    }

    EPG_PRINT("create_service_list - get %d service\n",cnt);
    return cnt;
}
#else
//create service list of tp_id from db
static UINT8 create_service_list(UINT32 tp_id, struct SERVICE_T *service, UINT8 last_cnt, UINT8 max_cnt)
{
    UINT8 cnt = last_cnt;
    UINT32 pos = 0;
    P_NODE p_node;
    UINT8 idx = 0;

    if (service == NULL)
    {
        return 0;
    }

    EPG_PRINT("\ncreate_service_list - tp: 0x%X\n",tp_id);
    //find all
    while (cnt < max_cnt && SUCCESS == get_prog_at(pos, &p_node))
    {
        if (p_node.tp_id == tp_id)
        {
            if ((0 == last_cnt) || (NULL == get_service(tp_id, p_node.prog_number, &idx)))    //not exist
            {
                service[cnt].tp_id = tp_id;
                service[cnt].service_id = p_node.prog_number;
                service[cnt].pf_sec_mask = 0;
                service[cnt].pf_ver = RESERVED_VER;
                service[cnt].pf_sec_mask_h = 0;
                service[cnt].pf_ver_h = RESERVED_VER;
                MEMSET(service[cnt].sch_sec_mask, 0, 32 * SCHEDULE_TABLE_ID_NUM);
                MEMSET(service[cnt].sch_ver, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
                MEMSET(service[cnt].sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
                MEMSET(service[cnt].sch_ver_h, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
                cnt ++;
                EPG_PRINT("create_service_list - add service: 0x%X\n",p_node.prog_number);
            }
        }
        pos ++;
    }

    if (0 == cnt)
    {
        return 0;
    }

    if (cnt != last_cnt)    //add some new programs
    {
        sort_service(service, cnt);
    }

    EPG_PRINT("create_service_list - get %d service\n",cnt);
    return cnt;
}
#endif

#ifdef EPG_OTH_SCH_SUPPORT
//create tp list table of sat_id, except current tp
static void create_tp_list(UINT16 sat_id, UINT32 tp_id)
{
    //static UINT16 pre_sat_id = 0;
    UINT16 max_num = 1024;
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    INT32 n = 0;
    UINT16 tp_num = 0;
    UINT32 id1 = 0;
    UINT32 id2 = 0;
    T_NODE t_node;

    MEMSET(&t_node, 0, sizeof(T_NODE));

    if ((sat_id == epg_info.pre_sat_id) && (tp_id == epg_info.tp_id))
    {
        return;
    }

    epg_info.pre_sat_id = sat_id;
    tp_num = get_tp_num_sat(sat_id);

    //get tp info from DB
    while ((i < tp_num) && (n < max_num))
    {
        if (SUCCESS == (get_tp_at(sat_id, i, &t_node))
            /*&&(t_node.tp_id != tp_id)*/) /*except current tp*/
        {
            epg_info.tp_ids[n][0] = (t_node.t_s_id<<16)|t_node.network_id;
            epg_info.tp_ids[n][1] = t_node.tp_id;
            n++;
        }
        i++;
    }
    epg_info.tp_id_num = n;
    EPG_PRINT("\ncreate_tp_list: sat 0x%X, tp num = %d\n",sat_id,n);

    //sort
    for (i=0; i<n-1; i++)
    {
        k = i;
        for (j=i+1; j<n; j++)
        {
            if (epg_info.tp_ids[j][0] < epg_info.tp_ids[k][0])
            {
                k = j;
            }
        }
        if (k != i)
        {
            id1 = epg_info.tp_ids[i][0];
            id2 = epg_info.tp_ids[i][1];
            epg_info.tp_ids[i][0] = epg_info.tp_ids[k][0];
            epg_info.tp_ids[i][1] = epg_info.tp_ids[k][1];
            epg_info.tp_ids[k][0] = id1;
            epg_info.tp_ids[k][1] = id2;
        }
    }

    for (i=0; i<n; i++)
    {
        EPG_PRINT("[ts|net]: 0x%08X, [tp]: 0x%08X\n",epg_info.tp_ids[i][0],epg_info.tp_ids[i][1]);
    }
}

//get tp_id by (ts_id, o_net_id)
static UINT32 get_tp_id(UINT16 ts_id, UINT16 o_net_id)
{
    int m = -1;
    int n = -1;
    int mid = -1;
    UINT32 id = (ts_id<<16)|o_net_id;

    if (0 == epg_info.tp_id_num)
    {
        MSG_PRINT("[%s]: not find [ts 0x%X, net 0x%X]\n",__FUNCTION__,ts_id,o_net_id);
        return INVALID_TP_ID;
    }

    if (id < epg_info.tp_ids[0][0])
    {
        MSG_PRINT("[%s]: not find [ts 0x%X, net 0x%X]\n",__FUNCTION__,ts_id,o_net_id);
        return INVALID_TP_ID;
    }

    if (id > epg_info.tp_ids[epg_info.tp_id_num-1][0])
    {
        MSG_PRINT("[%s]: not find [ts 0x%X, net 0x%X]\n",__FUNCTION__,ts_id,o_net_id);
        return INVALID_TP_ID;
    }

    //Dichotomy Quick Search
    m = 0;
    n = epg_info.tp_id_num - 1;
    if (id == epg_info.tp_ids[m][0])
    {
        MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[m][1]);
        return epg_info.tp_ids[m][1];
    }
    else if (id == epg_info.tp_ids[n][0])
    {
        MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[n][1]);
        return epg_info.tp_ids[n][1];
    }

    do
    {
        mid = (m + n) / 2;

        if ((mid == m) || (mid == n))
        {
            break;
        }

        if (id == epg_info.tp_ids[mid][0])
        {
            MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[mid][1]);
            return epg_info.tp_ids[mid][1];
        }
        else if (id > epg_info.tp_ids[mid][0])
        {
            m = mid;
        }
        else
        {
            n = mid;
        }
    } while (1);

    return INVALID_TP_ID;
}
#endif

//on section hit event. only check if the section exists.
static BOOL eit_event(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buf, INT32 length)
{
    UINT32 tp_id = 0;
    UINT16 sec_len = 0;
    UINT16 service_id = 0;
    UINT8  table_id = 0;
    UINT8  ver = 0;
    UINT8  sec_num = 0;
    UINT8 idx = 0;
    UINT8 len_min = 15;
    UINT16 len_max = 4096;
    section_status ret = NONE;

#ifdef EPG_OTH_SCH_SUPPORT

    UINT16 ts_id = 0;
    UINT16 o_net_id = 0;

#endif

#ifdef _DEBUG
    static UINT16 pre_service_id = 0xFFFF;
    static UINT8  pre_table_id = 0x1FFF;
    static UINT8  pre_sec_num = 0;
    UINT32 tick = osal_get_tick();
#endif
    if ((NULL == buf) || (length < (len_min + 3)) || (length > len_max))
    {
        EPG_PRINT("%s: buffer is NULL\n",__FUNCTION__);
        return FALSE;
    }

    table_id = buf[0];
#ifdef _DEBUG
    if (!IS_PF(table_id) && !IS_SCH(table_id))
    {
        EPG_PRINT("%s: invalid table id 0x%X\n",__FUNCTION__,table_id);
        ASSERT(0);
    }
    else if (IS_SCH(table_id) && (table_id&0x0F)>=SCHEDULE_TABLE_ID_NUM)
    {
        EPG_PRINT("%s: table id 0x%X overflow!\n",__FUNCTION__,table_id);
        ASSERT(0);
    }
#endif

    sec_len = ((buf[1]&0x0F)<<8) | buf[2];

    if (((sec_len + 3) > length) || (sec_len <= len_min) || (sec_len > (len_max - 3)))
    {
        EPG_PRINT("%s: length %d < section length %d\n",__FUNCTION__,length, sec_len + 3);
        return FALSE;
    }
    service_id = (buf[3]<<8) | buf[4];
    ver = (buf[5]>>1)&0x1f;
    sec_num = buf[6];

#ifdef EPG_OTH_SCH_SUPPORT
    if (IS_OTH_SCH(table_id)||IS_OTH_PF(table_id))
    {
        ts_id = (buf[8]<<8)|buf[9];
        o_net_id = (buf[10]<<8)|buf[11];
        tp_id = get_tp_id(ts_id, o_net_id);
        if (INVALID_TP_ID == tp_id)
        {
            return FALSE;
        }

        buf[8] = (tp_id>>24)&0xFF;
        buf[9] = (tp_id>>16)&0xFF;
        buf[10] = (tp_id>>8)&0xFF;
        buf[11] = tp_id&0xFF;
    }
    else
    {
        tp_id = epg_info.tp_id;
    }
#else
    tp_id = epg_info.tp_id;
#endif

    //if this section not exists, let handle parse it.
    ret = find_section(tp_id, service_id, table_id, ver, sec_num, &idx);

    //last_sec_num -> service idx
    buf[7] = idx;

    if ((PF_VERSION_UPDATE == ret) || (SCH_VERSION_UPDATE == ret))
    {
        //use byte 1/bit4,5(reserved in si spec) to tell handle the section status.
        //if the buf is protected by sie, then no need mutex to protect it.
        buf[1] = (buf[1] & VERSION_MASK) | ret;
        return TRUE;
    }
    else if ((PF_SECTION_UPDATE == ret) || (SCH_SECTION_UPDATE == ret))
    {
        buf[1] = (buf[1] & VERSION_MASK) | ret;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//parse it
static sie_status_t eit_handle(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buf, INT32 length)
{
    UINT32 tp_id = 0;
    UINT16 service_id = 0;
    UINT8  table_id = 0;
    UINT8  ver = 0;
    UINT8  sec_num = 0;
    UINT8  sec_mask = 0;
    INT32 ret_parse = 0;
    INT32 num = 0;
    INT32 ret_del = ERR_FAILED;

    if ((NULL == buf) || (SIE_REASON_FILTER_TIMEOUT == reason))
    {
        return sie_started;
    }

#ifdef EPG_OTH_SCH_SUPPORT
    if (IS_OTH_SCH(buf[0])||IS_OTH_PF(buf[0]))
    {
        tp_id = (buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
    }
    else
    {
        tp_id = epg_info.tp_id;
    }
#else
    tp_id = epg_info.tp_id;
#endif

    sec_mask = buf[1]&(~VERSION_MASK);
#ifdef EPG_MULTI_TP
#ifdef EPG_OTH_SCH_SUPPORT
#if !defined(DVBT2_SUPPORT) && defined(SUPPORT_FRANCE_HD)
    /* discard other PF SECTION, because it may cause information bar not refresh */
    if((PF_VERSION_UPDATE == sec_mask) || (PF_SECTION_UPDATE == sec_mask))
    {
        if (0xF == (buf[0] & 0xF))
        {
            return sie_started;
        }
    }
#endif
#endif
    //pf or sch data update, prepare delete old version event
    if((PF_VERSION_UPDATE == sec_mask) || (PF_SECTION_UPDATE == sec_mask))
    {
        service_id = (buf[3]<<8) | buf[4];
        if (PF_VERSION_UPDATE == sec_mask)
        { // temp solution for eit pf update problem
            ret_del = del_events(tp_id, service_id, buf[0]&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
            if (SUCCESS != ret_del)
            {
                EPG_PRINT("Failed at line:%d\n", __LINE__);
            }
        }
        num = create_update_view(epg_update_view_buff, EPG_UPDATE_VIEW_SIZE,
            tp_id, service_id, buf[0]&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
        if (num > 0)
        {
            EPG_PRINT("Line:%d, num:%d\n", __LINE__, num);
        }
    }
    else if((SCH_VERSION_UPDATE == sec_mask) || (SCH_SECTION_UPDATE == sec_mask))
    {
        service_id = (buf[3]<<8) | buf[4];
        num = create_update_view(epg_update_view_buff, EPG_UPDATE_VIEW_SIZE,
            tp_id, service_id, buf[0]&0x0F, SCHEDULE_EVENT_MASK);
        if (num > 0)
        {
            EPG_PRINT("Line:%d, num:%d\n", __LINE__, num);
        }
    }
#else
    //pf version update, del pf events first
    if (PF_VERSION_UPDATE == sec_mask)
    {
        service_id = (buf[3]<<8) | buf[4];
        ret_del = del_events(tp_id, service_id, buf[0]&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
    }
    else if (SCH_VERSION_UPDATE == sec_mask)  //sch version update, del sch events first
    {
        service_id = (buf[3]<<8) | buf[4];
        ret_del = del_events(tp_id, service_id, buf[0]&0x0F, SCHEDULE_EVENT_MASK);
    }
#endif

#ifdef _DEBUG
    UINT32 tick = osal_get_tick();
#endif

    //set bitmap of handle itself
    table_id = buf[0];
    ver = (buf[5]>>1)&0x1f;
    sec_num = buf[6];
    set_bitmap(table_id, ver, sec_num, buf[7]);

    ret_parse = eit_sec_parser(tp_id, buf, length, epg_info.epg_call_back);
    if (SUCCESS != ret_parse)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }

#ifdef _DEBUG
    libc_printf("%d/",osal_get_tick()-tick);
#endif
#ifdef MULTIFEED_SUPPORT
    eit_multifeed_handle(tp_id, buf, length, ((filter->dmx->type)& HLD_DEV_ID_MASK));
#endif

    return sie_started;
}

void epg_on_ext(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id)
{
    UINT8 dmx_id=0;
    INT32 ret = 0;

    if (epg_status != STATUS_OFF)
    {
        EPG_PRINT("_epg_on - status: %d error!\n",epg_status);
        return;
    }

    if (NULL == epg_info.buffer)
    {
        return;
    }

    epg_enter_mutex();

#ifdef EPG_MULTI_TP
    if (tp_id != epg_info.tp_id)
    {
        epg_info.service_id_num = 0; //reset current tp's service ids table
    }
    ret = epg_set_selected_tp(&tp_id);
    if (SUCCESS != ret)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }
    
    //add new services into the service ids table of this TP.
    epg_info.service_id_num = create_service_list(tp_id, epg_info.service_ids,
        epg_info.service_id_num, TP_MAX_SERVICE_COUNT);

#ifdef EPG_OTH_SCH_SUPPORT
    create_tp_list(sat_id, tp_id);
#endif

#else
    //a tp a unit. when change tp, do reset all
    if (tp_id != epg_info.tp_id)
    {
        reset_epg_internal();
    }
    //add new services into the struct list of this TP.
    epg_info.service_num = create_service_list(tp_id,
        epg_info.service, epg_info.service_num, TP_MAX_SERVICE_COUNT);
#endif

    epg_info.tp_id = tp_id;
    epg_info.service_id = service_id;
    epg_info.config.cur_service_id = service_id;

    //copy handle service to event
    restore_bitmap(epg_info.service, epg_info.service_num);

    epg_status = STATUS_ON;
    //epg mutex shall NOT nest sie api(called by start_eit)!
    epg_leave_mutex();
#ifndef _BUILD_OTA_E_
    if (NULL == dmx)
    {
        // default use dmx 0
        dmx_id = 0;
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    }
    ret = start_eit_ext((struct dmx_device*)dmx, &epg_info.config, eit_event, eit_handle, 1);
    if (SUCCESS != ret)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }
#endif
}

void epg_on(UINT16 sat_id, UINT32 tp_id, UINT16 service_id)
{
#ifdef _INVW_JUICE
    if(inview_is_started())
    {
        epg_on_by_inview(NULL, sat_id, tp_id, service_id, 0);
    }
#else
    epg_on_ext(NULL, sat_id, tp_id, service_id);
#endif
}

void epg_off(void)
{
    epg_off_ext();
}

void epg_off_ext(void)
{
    INT32 ret = 0;

    if (STATUS_ON != epg_status)
    {
        EPG_PRINT("epg_off_ext - status: %d error!\n",epg_status);
        return;
    }
    epg_enter_mutex();
    epg_status = STATUS_OFF;
    epg_leave_mutex();

    ret = stop_eit_ext();
    if (SUCCESS != ret)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }
}

enum EPG_STATUS epg_get_status(void)
{
    return epg_status;
}

INT32 epg_init(UINT8 mode, UINT8 *buf, UINT32 len, EIT_CALL_BACK call_back)
{
    UINT32 len_two_m = 0x200000;      //memory space 2M
#ifdef DVBC_INDIA
    UINT32 len_five_m = 0x500000;     //memory space 5M
    UINT32 len_one_half_m = 0x180000; //memory space 1.5M
#endif
    UINT32 service_buf_len = 0;
    UINT32 db_buf_len = 0;
    UINT32 x_mem_len = 0;
    UINT32 service_idx_len = 0;
    INT32 ret_mem = 0;
    INT32 ret_db = 0;

    if (STATUS_NOT_READY != epg_status)
    {
        EPG_PRINT("epg_init - status: %d error!\n",epg_status);
        return !SUCCESS;
    }

    MEMSET(&epg_info, 0, sizeof(epg_info));
    if(OSAL_INVALID_ID == lib_epg_flag)
    {
        lib_epg_flag = osal_flag_create(EPG_FLAG_MUTEX);
        if (OSAL_INVALID_ID == lib_epg_flag)
        {
            EPG_PRINT("epg flag create failure!\n");
            return !SUCCESS;
        }
    }

    if (buf != NULL)
    {
        epg_info.internal_buf = FALSE;
    }
    else
    {
        epg_info.internal_buf = TRUE;
        buf = MALLOC(len);
    }

    if (NULL == buf/* || len < EIT_FILTER_BUFFER_LEN*/)
    {
        EPG_PRINT("epg_init - mem not enough!!!\n");
        return !SUCCESS;
    }

    epg_info.epg_call_back = call_back;
    epg_info.buffer = buf;
    epg_info.buf_len = len;

    //SIE_EIT_SINGLE_SERVICE or SIE_EIT_WHOLE_TP
    epg_info.config.eit_mode = mode;
    EPG_PRINT("epg_init -- buffer: 0x%X, len: %d, mode: %d, max TP Service num: %d\n",
        buf, len, mode, TP_MAX_SERVICE_COUNT);

    //si filter buffer, 32k or 128k
    epg_info.config.buffer = buf;
    if (SIE_EIT_SINGLE_SERVICE == mode)
    {
        epg_info.config.buf_len = 32*1024;  //si filter buffer
        db_buf_len = 48*1024;   //epg db buffer
    }
    else
    {
        epg_info.config.buf_len = 64*1024;  //si filter buffer

#ifdef EPG_OTH_SCH_SUPPORT
#ifndef DVBC_INDIA
        if (len >= len_two_m)
        {
            db_buf_len = 1024*1024;//512*1024;  //epg db buffer
        }
        else
        {
            db_buf_len = 256*1024;  //epg db buffer
        }
#else
        if(len >= len_five_m)
        {
            db_buf_len = 2*1024*1024;
        }
        else if(len >= len_one_half_m)
        {
            db_buf_len = 512*1024;//512*1024;   //epg db buffer
        }
        else
        {
            db_buf_len = 256*1024;//256*1024;   //epg db buffer
        }
#endif
#else
        db_buf_len = 256*1024;  //epg db buffer
#endif
    }
    epg_info.service = (struct SERVICE_T*)(buf + epg_info.config.buf_len);
    //service table of tp, 64 * 136 = 8704(9k)
    service_buf_len = TP_MAX_SERVICE_COUNT * sizeof(struct SERVICE_T);
    EPG_PRINT("epg_init - sizeof SERVICE_T: %d, service buffer len: %d\n",sizeof(struct SERVICE_T),service_buf_len);

#ifdef EPG_MULTI_TP
    epg_info.service_idx = (struct SERVICE_INDEX_T*)(buf + epg_info.config.buf_len+service_buf_len);
    service_idx_len = TP_MAX_SERVICE_COUNT * sizeof(struct SERVICE_INDEX_T);
#endif

    x_mem_len = len - epg_info.config.buf_len - service_buf_len - db_buf_len - service_idx_len;

    //epg db buffer
    ret_db = init_epg_db(lib_epg_flag, buf+epg_info.config.buf_len+service_buf_len+service_idx_len, db_buf_len);

    //x_alloc buffer, 256k at least
    ret_mem=x_init_mem(lib_epg_flag, buf+epg_info.config.buf_len+service_buf_len+service_idx_len+db_buf_len,x_mem_len);

    if((ERR_FAILED == ret_mem) || (ERR_FAILED == ret_db))
    {
        return !SUCCESS;
    }

    epg_status = STATUS_OFF;
    return SUCCESS;
}

INT32 epg_release(void)
{
    INT32 ret = 0;

    if (STATUS_OFF != epg_status)
    {
        EPG_PRINT("epg_release - status: %d error!\n",epg_status);
        return !SUCCESS;
    }

#ifdef EPG_CACHE_ENABLE
    //cache pf event
    free_epg_cache();
    if(SUCCESS == init_epg_cache())
    {
        cache_db();
    }
#endif

    if (epg_info.internal_buf && (epg_info.buffer != NULL))
    {
        FREE(epg_info.buffer);
        epg_info.buffer = NULL;
    }

    MEMSET(&epg_info, 0, sizeof(epg_info));

    ret = release_epg_db();
    if (SUCCESS != ret)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }    
    ret = x_release_mem();
    if (SUCCESS != ret)
    {
        EPG_PRINT("Failed at line:%d\n", __LINE__);
    }

    epg_status = STATUS_NOT_READY;
    return SUCCESS;
}

//get current playing tp id
UINT32 epg_get_cur_tp_id(void)
{
    return epg_info.tp_id;
}

INT32 epg_set_active_service(struct ACTIVE_SERVICE_INFO *service, UINT8 cnt)
{
    epg_enter_mutex();

    if (cnt > MAX_ACTIVE_SERVICE_CNT)
    {
        EPG_PRINT("epg_set_active_service - ACTIVE SERVICE COUNT %d overflow!\n",cnt);
        cnt = MAX_ACTIVE_SERVICE_CNT;
    }

    if (service != NULL)
    {
        MEMCPY(active_service, service, cnt*sizeof(struct ACTIVE_SERVICE_INFO));
    }

    active_service_cnt = cnt;

    epg_leave_mutex();
    return cnt;
}

//check active service
BOOL epg_check_active_service(UINT32 tp_id, UINT16 service_id)
{
    int i = 0;

    epg_enter_mutex();

    for (i=0; i<active_service_cnt; i++)
    {
        if ((tp_id == active_service[i].tp_id) && (service_id == active_service[i].service_id))
        {
            epg_leave_mutex();
            return TRUE;
        }
    }
    epg_leave_mutex();
    return FALSE;
}

#ifdef PARENTAL_SUPPORT
#ifdef EPG_MULTI_TP
static UINT8 pvr_create_service_list(UINT16 *service_ids, UINT8 last_cnt, UINT16 service_id)
{
    UINT8 cnt = last_cnt;

    if (NULL == service_ids)
    {
        return 0;
    }

    service_ids[cnt] =service_id;
    cnt++;

    if (0 == cnt)
    {
        return 0;
    }
    EPG_PRINT("create_service_list - get %d service\n",cnt);
    return cnt;
}

#else
//create service list of tp_id from db
static UINT8 pvr_create_service_list(struct SERVICE_T *service, UINT8 last_cnt, UINT16 service_id)
{
    UINT8 cnt = last_cnt;

    if (NULL == service)
    {
        return 0;
    }
    //find all
    if (0 == last_cnt)
    {
        service[cnt].tp_id =  0;
        service[cnt].service_id = service_id;
        service[cnt].pf_sec_mask = 0;
        service[cnt].pf_ver = RESERVED_VER;
        service[cnt].pf_sec_mask_h = 0;
        service[cnt].pf_ver_h = RESERVED_VER;
        MEMSET(service[cnt].sch_sec_mask, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[cnt].sch_ver, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[cnt].sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
        MEMSET(service[cnt].sch_ver_h, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
        cnt ++;
    }
    return cnt;
}
#endif

void pvr_epg_on(void *dmx, UINT16 service_id)
{
    UINT8 dmx_id=0;

    if (STATUS_OFF != epg_status)
    {
        EPG_PRINT("_epg_on - status: %d error!\n",epg_status);
        return;
    }

    if (NULL == epg_info.buffer)
    {
        return;
    }
    epg_enter_mutex();

    //a tp a unit. when change tp, do reset all
    reset_epg_internal();

    epg_info.tp_id = 0x0;
    epg_info.service_id = service_id;

    //add new services into the struct list of this TP.
#ifdef EPG_MULTI_TP
    //add new services into the service ids table of this TP.
    epg_info.service_id_num = pvr_create_service_list(epg_info.service_ids, epg_info.service_id_num, service_id);
#else
    //add new services into the struct list of this TP.
    epg_info.service_num = pvr_create_service_list(epg_info.service, epg_info.service_num, service_id);
#endif
    epg_info.config.cur_service_id = service_id;

    //copy handle service to event
    restore_bitmap(epg_info.service, epg_info.service_num);

    epg_status = STATUS_ON;
    //epg mutex shall NOT nest sie api(called by start_eit)!
    epg_leave_mutex();

#ifndef _BUILD_OTA_E_
   if (NULL == dmx)
   {
       dmx_id=lib_nimng_get_nim_play()-1;
       dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
   }
   start_eit_ext((struct dmx_device*)dmx, &epg_info.config, eit_event, eit_handle, 1);
#endif
}
#endif
#ifdef _INVW_JUICE
static BOOL eit_event_inview(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buf, INT32 length)
{
    if((NULL == buf) || (0 == length))
    {
        return TRUE;
    }
    inview_eit_section_arrival(buf);

    return FALSE;
}

void epg_on_by_inview(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id, EIT_TYPE type)
{
    if (STATUS_OFF != epg_status)
    {
        EPG_PRINT("_epg_on - status: %d error!\n",epg_status);
        return;
    }

    if (NULL == epg_info.buffer)
    {
        return;
    }

    epg_enter_mutex();

#ifdef EPG_MULTI_TP
    if (tp_id != epg_info.tp_id)
    {
        epg_info.service_id_num = 0; //reset current tp's service ids table
    }
    epg_set_selected_tp(&tp_id);
    //add new services into the service ids table of this TP.
    epg_info.service_id_num = create_service_list(tp_id, epg_info.service_ids,
        epg_info.service_id_num, TP_MAX_SERVICE_COUNT);

#ifdef EPG_OTH_SCH_SUPPORT
    create_tp_list(sat_id, tp_id);
#else
#endif

#else
    //a tp a unit. when change tp, do reset all
    if (tp_id != epg_info.tp_id)
    {
        reset_epg_internal();
    }
    //add new services into the struct list of this TP.
    epg_info.service_num = create_service_list(tp_id, epg_info.service, epg_info.service_num, TP_MAX_SERVICE_COUNT);
#endif

    epg_info.tp_id = tp_id;
    epg_info.service_id = service_id;
    epg_info.config.cur_service_id = service_id;

    //copy handle service to event
    restore_bitmap(epg_info.service, epg_info.service_num);

    epg_status = STATUS_ON;
    //epg mutex shall NOT nest sie api(called by start_eit)!
    epg_leave_mutex();

    if (dmx)
    {
        start_eit_ext((struct dmx_device*)dmx, &epg_info.config, eit_event, eit_handle, 1);
    }
    else
    {
        start_eit_by_inview(&epg_info.config, eit_event_inview, eit_handle, type);
    }
}
void epg_init_inview()
{
    epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START/*buffer*/, __MM_EPG_BUFFER_LEN, NULL);
}

#endif
