/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_dtg.c
*
*    Description: SI monitor adding DTG (SI specification in UK) character
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#include <api/libpub/lib_cc.h>
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>

#ifdef SUPPORT_FRANCE_HD
#include <api/libsi/si_nit.h>
#endif

#define DTG_DEBUG_LEVEL         0
#if (DTG_DEBUG_LEVEL>0)
#define DTG_PRINTF          libc_printf
#else
#define DTG_PRINTF(...)         do{}while(0)
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
#if 0
#define DYN_SERV_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define DYN_SERV_DEBUG(...) do{} while(0)
#endif
#endif

#define MAX_NAME_LEN      512
#define MAX_LANG_CNT      10

#ifdef SUPPORT_FRANCE_HD
INT32 si_on_service_list_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
static void receive_nit_callback(UINT8 *section, INT32 length, UINT32 param);

UINT32 nit_monitor_id = 0xFFFFFFFF;
struct NIT_TABLE_INFO nit_table_info;
struct NIT_TABLE_INFO t_nit_table_info;
struct extension_info nit_ext;
static nit_change_cb nit_change = NULL;
UINT8  nit_flag[PSI_MODULE_MSECTION_NUMBER>>3] = {0};
#ifdef _ISDBT_ENABLE_
static nit_change_cb nit_ews_detect= NULL;
#define brazil_network_id   0x7dc4
#endif
#endif
//static UINT8 ori_service_index = 0;

#ifdef _MULTILINGAL_SERVICE_NAME_ENABLE_
struct mult_service_name
{
    UINT32 iso_639_language_code:24;
    UINT8 service_provider_name_length;
    UINT8 *service_provider_name;
    UINT8 service_name_length;
    UINT8 *service_name;
};
#endif

#ifdef _PREFERRED_NAME_ENABLE_
struct pref_name_list
{
    UINT8 name_id;
    UINT8 name_len;
    UINT8 *name;
};

struct pref_name_data
{
    UINT8 iso_639_language_code[3];
    UINT8 name_cnt;
    struct pref_name_list name_list[5];
};
#endif

struct program_data
{
    UINT16 program_number;

#ifdef _SERVICE_NAME_UPDATE_ENABLE_
    UINT8 service_provider_length;
    UINT8 *service_provider;
    UINT8 service_name_length;
    UINT8 *service_name;
#endif

#ifdef _SHORT_SERVICE_NAME_ENABLE_
    UINT8 short_name_len;
    UINT8 *short_service_name;
#endif

#ifdef _MULTILINGAL_SERVICE_NAME_ENABLE_
    UINT8 mult_name_count;
    struct mult_service_name ml_name[MAX_LANG_CNT];
#endif

#ifdef _PREFERRED_NAME_ENABLE_
    struct pref_name_data pref_name[MAX_LANG_CNT];
#endif
    UINT8 service_running_status;

#ifdef SUPPORT_FRANCE_HD
    UINT16 linkage_ts_id;
    UINT16 linkage_ori_network_id;
    UINT16 linkage_service_id;
    UINT8 linkage_dec_type;
#endif

    INT32 name_pos;
    UINT8 names[MAX_NAME_LEN];
};

#ifdef SUPPORT_FRANCE_HD
struct linkage_service_info
{
    UINT16 ts_id;
    UINT16 ori_network_id;
    UINT16 service_id;
    UINT8 private_data;
    UINT8 private_len;
    UINT8 linkage_type;
};

#endif

enum RUNNING_STATUS
{
    RS_UNDEF = 0,
    RS_NOT_RUNNING,
    RS_START_IN_FEW_SEC,
    RS_PAUSE,
    RS_RUNNING,
};

struct running_data_t
{
    UINT16 service_id;
    UINT8 running_status;
};
struct sdt_running_info_t
{
    UINT8 sd_count;
    struct running_data_t running_data[PSI_MODULE_MAX_PROGRAM_NUMBER];
};

struct SDT_TABLE_INFO
{
    UINT8 sd_count;
    struct program_data sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
};

#ifdef SUPPORT_FRANCE_HD
struct NIT_TABLE_INFO
{
    struct nit_section_info nit;
};
#endif

#if(defined (NEWLAND_UPDATE)||defined(SDT_MONITOR))
typedef struct
{
    UINT16 ts_id;
    UINT16 network_id;
    UINT8 version;
    UINT8 bit_map[PSI_MODULE_MSECTION_NUMBER>>3];
} SDT_PARAM_EVENT;

SDT_PARAM_EVENT sdt_info;
static UINT8 sdt_buff[PSI_SHORT_SECTION_LENGTH] = {0};
static UINT16 name_buff[MAX_SERVICE_NAME_LENGTH+1] = {0};
#endif

#ifdef SUPPORT_FRANCE_HD
struct SDT_TABLE_INFO sdt_other_info;
struct extension_info sdt_other_ext;
UINT8 sdt_other_flag[PSI_MODULE_MSECTION_NUMBER>>3] = {0};
LINKAGE_STATUS link_status;
UINT8 sdt_trigge_channel_change = FALSE;
static P_NODE  program_other;
static on_sdt_change_t sdt_on_change = NULL;
on_sdt_other_return_t sdt_other_return = NULL;
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
static sdt_service_change_cb sdt_service_update_cb = NULL;
#endif

#if(defined(_SERVICE_NAME_UPDATE_ENABLE_)||defined(_MULTILINGAL_SERVICE_NAME_ENABLE_)\
    ||defined(_PREFERRED_NAME_ENABLE_)||defined(_SHORT_SERVICE_NAME_ENABLE_))

struct SDT_TABLE_INFO sdt_info;
struct extension_info sdt_ext;
UINT8 sdt_flag[PSI_MODULE_MSECTION_NUMBER>>3] = {0};
UINT32 sdt_monitor_id = 0xFFFFFFFF;
static UINT16 m_cur_chan_index = 0xFFFF;
static P_NODE program;
static struct sdt_running_info_t *p_sdt_running_info = NULL;

/*start monitoring SDT*/
static on_pid_change_t on_change = NULL;

static INT32 sdt_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 sdt_on_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
static INT32 sdt_on_multilingual_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
static INT32 sdt_on_preferred_name_list_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
static INT32 sdt_on_short_service_name_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);

#ifdef SUPPORT_FRANCE_HD
static INT32 tnt_sdt_callback(UINT8 *section, INT32 length, UINT32 param);
void sdt_other_monitor_on(UINT32 index);
static INT32 sdt_other_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 sdt_on_linkage_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif

void sdt_monitor_off();

void sdt_monitor_callback(on_pid_change_t pid_change ,void *p_sdt_info)
{
    on_change = pid_change;
    p_sdt_running_info = p_sdt_info;
}
void sdt_monitor_on(UINT32 index)
{
    struct dmx_device *dmx = NULL;
    P_NODE node;

    dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
    if(get_prog_at(index, &node) != SUCCESS)
    {
        return ;
    }

    m_cur_chan_index = index;
    if(0 == node.user_modified_flag)
    {
        MEMCPY(&program, &node, sizeof(P_NODE));
    }
    if (sdt_monitor_id != 0xFFFFFFFF)
    {
        sdt_monitor_off();
    }

    /*start monitoring*/
    sdt_monitor_id = sim_start_monitor(dmx, MONITE_TB_SDT, PSI_SDT_PID, 0);

    /*register callback function*/
#ifdef SUPPORT_FRANCE_HD
    sim_register_scb(sdt_monitor_id, tnt_sdt_callback, NULL);
#else
    sim_register_scb(sdt_monitor_id, sdt_callback, NULL);
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
    T_NODE t_node;
    if (get_tp_by_id(node.tp_id, &t_node) == DB_SUCCES)
    {
        sdt_ext.version = t_node.sdt_version;
    }
    else
    {
        sdt_ext.version = 0xFF; /* set to invalid value */
    }
#endif
    sdt_ext.flag = sdt_flag;
    sdt_ext.hit = 0;
}
/*stop monitoring*/
void sdt_monitor_off()
{
    if(0xFFFFFFFF == sdt_monitor_id)
    {
        return;
    }

    sim_stop_monitor(sdt_monitor_id);
    sdt_monitor_id = 0xFFFFFFFF;
}

static void sdt_desc_parser(UINT8 *data, UINT32 len, struct program_data *sd)
{
    UINT32 i = 0;
    UINT8  desc_len = 0;
    UINT8  desc_tag = 0;
    UINT32 priv_data_spec = 0;
    UINT32 priv_data_specifier_id = 0x0000233a;

    if((NULL == data) || (NULL == sd))
    {
        return ;
    }

    for(i=0; i<len; i+=(desc_len+2))
    {
        desc_tag = data[i];
        desc_len = data[i+1];

        switch (desc_tag)
        {
#ifdef _SERVICE_NAME_UPDATE_ENABLE_
            case SERVICE_DESCRIPTOR:
                sdt_on_service_desc(desc_tag, desc_len, data+i+2, sd);
                break;
#endif
#ifdef _MULTILINGAL_SERVICE_NAME_ENABLE_
            case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR:
                sdt_on_multilingual_service_desc(desc_tag, desc_len, data+i+2, sd);
                break;
#endif
            case PRIVATE_DATA_SPECIFIER_DESCRIPTOR:
                priv_data_spec = (data[i+2]<<24)|(data[i+3]<<16)|(data[i+4]<<8)|(data[i+5]);
                break;
#ifdef _PREFERRED_NAME_ENABLE_
            case PREFERRED_NAME_LIST_DESCRIPTOR:
                if(priv_data_specifier_id == priv_data_spec)
                {
                    sdt_on_preferred_name_list_desc(desc_tag, desc_len, data+i+2, sd);
                }
                break;
#endif
#ifdef _SHORT_SERVICE_NAME_ENABLE_
            case SHORT_SERVICE_NAME_DESCRIPTOR:
                if(priv_data_specifier_id == priv_data_spec)
                {
                    sdt_on_short_service_name_desc(desc_tag, desc_len, data+i+2, sd);
                }
                break;
#endif
#ifdef SUPPORT_FRANCE_HD
            case LINKAGE_DESCRIPTOR:
                //if((priv_data_spec==0x0000233a)||(priv_data_spec==0x00000028))
                    sdt_on_linkage_desc(desc_tag, desc_len, data+i+2, sd);
                break;
#endif
            default:
                break;
        }
    }
}

/*check if need monitor*/
static BOOL sdt_event(UINT8 *buffer)
{
    if(NULL == buffer)
    {
        return FALSE;
    }

    BOOL need_reset = FALSE;
    struct section_header *sh = (struct section_header *)buffer;

    UINT8 section_number = sh->section_number;
    UINT8 last_section_number = sh->last_section_number;

    //this section first hit, need set table sections hit flag bitmap
    if (0 == sdt_ext.hit)
    {
        sdt_ext.version = sh->version_number;
        sdt_ext.last_section_number = last_section_number;
        sdt_ext.hit++;
        need_reset = TRUE;
    }
    //this section not first hit, but table version changed,need reget the table sections
    else if (sh->version_number != sdt_ext.version)
    {
        sdt_ext.version = sh->version_number;
        need_reset = TRUE;
    }
    //this section not first hit, not need reget
    else
    {
        need_reset = FALSE;
    }
    //set table sections hit flag bitmap
    if (TRUE == need_reset)
    {
        MEMSET(sdt_ext.flag, 0xFF, (last_section_number>>3)+1);
    }
    //this section not the one wanted, return false
    else if (!(sdt_ext.flag[section_number>>3]&(1<<(section_number&7))))
    {
        return FALSE;
    }

    sdt_ext.flag[section_number>>3] &= ~(1<<(section_number&7));

    return TRUE;
}

static BOOL update_service_name_by_sdt(const struct program_data *sd, UINT16 service_id,
    UINT16 ts_id, UINT16 original_network_id)
{
    UINT32 i = 0;
    UINT32 len = 0;
    UINT32 refcode = 0;
    BOOL   db_changed = FALSE;
    P_NODE node;
    T_NODE t_node;
    UINT16 name[MAX_SERVICE_NAME_LENGTH+1] = {0};

    if(NULL == sd)
    {
        return FALSE;
    }

    MEMSET(&node, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));

#ifdef GB2312_SUPPORT
    refcode = 2312;
#endif
    len = (sd->service_name_length > MAX_SERVICE_NAME_LENGTH ? MAX_SERVICE_NAME_LENGTH : sd->service_name_length);
    if(0 == len)
    {
        return FALSE;
    }
    dvb_to_unicode(sd->service_name, len, &name, MAX_SERVICE_NAME_LENGTH, refcode);

    //Only programs that belongs to the current db view type can be updated.
    //So radio programs can not be updated if now is TV mode, and vice versa.
    while(SUCCESS == get_prog_at(i, &node))
    {
        if(node.prog_number == service_id)
        {
            get_tp_by_id(node.tp_id, &t_node);
            if((t_node.t_s_id == ts_id) && (t_node.network_id == original_network_id))
            {
                if(0 != com_uni_str_cmp(name, (UINT16*)(&node.service_name)) )
                {
                    db_changed = TRUE;
                    com_uni_str_copy_ex(&node.service_name, &name, len);
                    modify_prog(node.prog_id, &node);
                }
                break;
            }
        }
        i++;
    }

    return db_changed;
}

static INT32 sdt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct SDT_TABLE_INFO *sdt = NULL;
    UINT8 *loop = NULL;
    INT32 i = 0;
    INT32 j = 0;
    INT32 descriptors_length = 0;
    INT32 ret = SI_PERROR;
    UINT16 service_id = 0;
    UINT16 ts_id = 0;
    UINT16 original_network_id = 0;
    UINT8 running_status = 0;
    UINT8 ori_running_status = 0;
    UINT8 run = 0;
    UINT8 ori_run = 0;
    UINT8 k = 0;
//  BOOL db_changed = FALSE;

    //if section unchanged, return immediately
    if(!sdt_event(section))
    {
        return SI_SUCCESS;
    }

    sdt = &sdt_info;
    ts_id = (UINT16)((section[3])<<8)|section[4];
    original_network_id = (UINT16)((section[8])<<8)|section[9];
    loop = section+11;
    length -= (11+PSI_SECTION_CRC_LENGTH);

    for(i=0; i<length; i+=(descriptors_length+5))
    {
        service_id = (UINT16)((loop[i]<< 8)|loop[i+1]);
        descriptors_length = (UINT16)(((loop[i+3]&0x0F)<< 8)|loop[i+4]);

        for(j=0; j < sdt->sd_count; j++)
        {
            if(service_id == sdt->sd[j].program_number)
            {
                break;
            }
        }

        if (sdt->sd_count == PSI_MODULE_MAX_PROGRAM_NUMBER)
        {
            break;
        }

        ori_running_status = sdt->sd[j].service_running_status;
        running_status = (UINT8)((loop[i+3]&0xE0) >> 5);

        if((ori_running_status == RS_UNDEF) ||(ori_running_status == RS_RUNNING))
        {
            ori_run = 1;//running
        }
        else
        {
            ori_run=0;//Not running
        }

        sdt->sd[j].service_running_status = running_status;

        if((running_status == RS_UNDEF) || (running_status == RS_RUNNING))
        {
            run = 1;//running
        }
        else
        {
            run = 0;//Not running
        }

        if( (((0 == run) && (0==ori_run)) || (run != ori_run)) && (program.prog_number == service_id))
        {
            if(on_change)
            {
                /* the callback [ on_change ] use to start/stop running the service: service_id
                 * here do not stop running even if running_status = 1(not running).
                 * But: if need open this function describe as spec, please open the following line */
                //on_change(run); // you can open this line if needed
            }
        }

        sdt->sd[j].program_number = service_id;
        sdt->sd[j].name_pos = 0;
        sdt_desc_parser(loop+i+5, descriptors_length, &(sdt->sd[j]));

        //Remark it because it's not a good idea to change database directly at middle layer.
//      db_changed |= update_service_name_by_sdt(&(sdt->sd[j]),service_id, ts_id, original_network_id);

        if(j == sdt->sd_count)
        {
            sdt->sd_count++;
        }

        //pass sdt running status to AP
        if(p_sdt_running_info)
        {
            p_sdt_running_info->sd_count = sdt->sd_count;
            for(k=0; k<sdt->sd_count; k++)
            {
                p_sdt_running_info->running_data[k].service_id = sdt->sd[k].program_number;
                p_sdt_running_info->running_data[k].running_status = sdt->sd[k].service_running_status;
            }
        }
        ret = SI_SUCCESS;
    }

//    if (db_changed)
//        update_data();

    return ret;
}
#endif

#ifdef _SERVICE_NAME_UPDATE_ENABLE_
/*parse service descriptor*/
static INT32 sdt_on_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 name_len = 0;
    UINT8 provider_len = 0;
    struct program_data *sd = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    sd = (struct program_data *)priv;
    provider_len = desc[1];
    name_len = desc[provider_len+2];

    if (sd->name_pos + provider_len < MAX_NAME_LEN)
    {
        sd->service_provider = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc+2, provider_len);
        sd->service_provider_length = provider_len;
        sd->name_pos += provider_len;
        DTG_PRINTF("service provider name len: %d\n", provider_len);
    }

    if (sd->name_pos+name_len < MAX_NAME_LEN)
    {
        sd->service_name = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc+provider_len+3, name_len);
        sd->service_name_length = name_len;
        sd->name_pos += name_len;
        DTG_PRINTF("service name len: %d\n", name_len);
    }

    return SI_SUCCESS;
}

UINT32 si_get_service_name(UINT16 program_number, UINT8 *name, UINT8 *name_len)
{
    UINT32 prog_idx = 0;

    if((NULL == name) || (NULL == name_len))
    {
        return SI_SBREAK;
    }

    for(prog_idx=0; prog_idx<sdt_info.sd_count; prog_idx++)
    {
        if(sdt_info.sd[prog_idx].program_number == program_number)
        {
            *name_len = sdt_info.sd[prog_idx].service_name_length;
            MEMCPY(name, sdt_info.sd[prog_idx].service_name, *name_len);
            return SI_SUCCESS;
        }
    }

    return SI_SBREAK;
}

UINT32 si_get_provider_name(UINT16 program_number, UINT8 *name, UINT8 *name_len)
{
    UINT32 prog_idx = 0;

    if((NULL == name) || (NULL == name_len))
    {
        return SI_SBREAK;
    }

    for(prog_idx=0; prog_idx<sdt_info.sd_count; prog_idx++)
    {
        if(sdt_info.sd[prog_idx].program_number == program_number)
        {
            *name_len = sdt_info.sd[prog_idx].service_provider_length;
            MEMCPY(name, sdt_info.sd[prog_idx].service_provider, *name_len);
            return SI_SUCCESS;
        }
    }

    return SI_SBREAK;
}
#endif

#ifdef _MULTILINGAL_SERVICE_NAME_ENABLE_
/*parse multilingual service name descriptor*/
static INT32 sdt_on_multilingual_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 name_len = 0;
    UINT8 prov_len = 0;
    INT32 i = 0;
    struct program_data *sd = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    sd = (struct program_data *)priv;

    while(len > 0)
    {
        prov_len = desc[3];

        sd->ml_name[i].iso_639_language_code = (desc[0]<<16)|(desc[1]<<8)|desc[2];
        sd->ml_name[i].service_provider_name_length = prov_len;
        sd->ml_name[i].service_provider_name = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc+4,prov_len);

        desc += (prov_len+4);
        name_len = desc[0];
        sd->name_pos += prov_len;

        sd->ml_name[i].service_name_length = name_len;
        sd->ml_name[i].service_name = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc+1, name_len);
        sd->name_pos += name_len;

        len -= 5+prov_len+name_len;
        desc += (name_len+1);
        i++;
    }

    sd->mult_name_count = i;
    return SI_SUCCESS;
}

#endif

#ifdef _PREFERRED_NAME_ENABLE_
/*parse preferred name list descriptor*/
static INT32 sdt_on_preferred_name_list_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    INT32 i = 0;
    INT32 j = 0;
    struct program_data *sd = NULL;
    struct pref_name_list *namelist = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    sd = (struct program_data *)priv;
    while(len > 0)
    {
        MEMCPY(sd->pref_name[i].iso_639_language_code, desc, 3);
        sd->pref_name[i].name_cnt = desc[3];
        desc += 4;
        len -= 4;

        for(j=0; j<sd->pref_name[i].name_cnt; j++)
        {
            namelist = &sd->pref_name[i].name_list[j];
            namelist->name_id = desc[0];
            namelist->name_len = desc[1];
            namelist->name = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc+2, namelist->name_len);
            sd->name_pos += namelist->name_len;

            len -= (namelist->name_len+2);
            desc += (namelist->name_len+2);
        }

        i++;
    }
    return SI_SUCCESS;
}

UINT32 si_get_preferred_name(UINT16 program_number, char *iso_lang, UINT8 name_id, UINT8 *name, UINT8 *name_len)
{
    UINT8 cmp_rst = 0;
    UINT32 prog_idx = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    struct program_data *sd = NULL;
    INT32 ret = ERR_FAILUE;

    if((NULL == iso_lang) || (NULL == name) || (NULL == name_len))
    {
        return ERR_FAILUE;
    }

    for(prog_idx=0; prog_idx<sdt_info.sd_count; prog_idx++)
    {
        if(sdt_info.sd[prog_idx].program_number == program_number)
        {
            ret = SUCCESS;
            break;
        }
    }

    if(ret != SUCCESS)
    {
        return ERR_FAILUE;
    }

    ret = ERR_FAILUE;
    sd = &sdt_info.sd[prog_idx];

    for(i=0; i<10; i++)
    {
        cmp_rst = MEMCMP(sd->pref_name[i].iso_639_language_code, iso_lang, 3);
        if(!cmp_rst)
        {
            for(j=0; j< sd->pref_name[i].name_cnt; j++)
            {
                if(sd->pref_name[i].name_list[j].name_id == name_id)
                {
                    *name_len = sd->pref_name[i].name_list[j].name_len;
                    MEMCPY(name, sd->pref_name[i].name_list[j].name, *name_len);
                    ret = SUCCESS;
                    break;
                }
            }
        }
    }

    return ret;
}
#endif

#ifdef _SHORT_SERVICE_NAME_ENABLE_
/*parse short service name descriptor*/
static INT32 sdt_on_short_service_name_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct program_data *sd = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    sd = (struct program_data *)priv;
    sd->short_name_len = len;

    if((sd->name_pos+len) < MAX_NAME_LEN)
    {
        sd->short_service_name = (UINT8*)MEMCPY(sd->names+sd->name_pos, desc, len);
        sd->name_pos += len;
    }

    return SI_SUCCESS;
}

UINT32 si_get_short_service_name(UINT16 program_number, UINT8 *name, UINT8 *name_len)
{
    UINT32 prog_idx = 0;

    if((NULL == name) || (NULL == name_len))
    {
        return SI_SBREAK;
    }

    for(prog_idx=0; prog_idx<sdt_info.sd_count; prog_idx++)
    {
        if(sdt_info.sd[prog_idx].program_number == program_number)
        {
            *name_len = sdt_info.sd[prog_idx].short_name_len;
            MEMCPY(name, sdt_info.sd[prog_idx].short_service_name, *name_len);
            return SI_SUCCESS;
        }
    }

    return SI_SBREAK;
}
#endif

#if(defined (NEWLAND_UPDATE)||defined(SDT_MONITOR))
static BOOL sdt_event(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *buffer, INT32 length)
{
    if(NULL == buffer)
    {
        return FALSE;
    }

    struct section_header *sh = (struct section_header *)buffer;
    UINT8 section_number = sh->section_number;
    UINT8 last_section_number = sh->last_section_number;
    UINT16 ts_id = (buffer[3]<<8)|buffer[4];
    UINT16 net_id = (buffer[8]<<8)|buffer[9];

    //table version changed,need reset the map
    if((sh->version_number != sdt_info.version) || (ts_id != sdt_info.ts_id || net_id != sdt_info.network_id))
    {
        sdt_info.ts_id = ts_id;
        sdt_info.network_id = net_id;
        sdt_info.version = sh->version_number;
        MEMSET(sdt_info.bit_map, 0, (last_section_number>>3)+1);
    }
    //this section not the one wanted, return false
    else if ((sdt_info.bit_map[section_number>>3]&(1<<(section_number&7))) != 0)
    {
        return FALSE;
    }

    sdt_info.bit_map[section_number>>3] |= (1<<(section_number&7));

    return TRUE;
}

static BOOL process_sdt(UINT16 service_id, UINT8 *name, UINT16 ts_id, UINT16 network_id)//, UINT8 *ISO639_code)
{
    UINT32 i = 0;
    UINT32 len = 0;
    UINT32 refcode = 1;
    BOOL changed = FALSE;
    P_NODE node;
    T_NODE t_node;

#ifdef GB2312_SUPPORT
    refcode = 2312;//GB2312 coding
#endif
    if(NULL == name)
    {
        return FALSE;
    }

    len = *name;
    if(len == 0)
    {
        return FALSE;
    }

    dvb_to_unicode(name+1, len, name_buff, MAX_SERVICE_NAME_LENGTH, refcode);

    while(SUCCESS == get_prog_at(i, &node))
    {
        if(node.prog_number == service_id)
        {
            get_tp_by_id(node.tp_id, &t_node);

            if((t_node.t_s_id == ts_id) && (t_node.network_id == network_id))
            {
                if(0 != DB_STRCMP(name_buff, node.service_name))
                {
                    changed = TRUE;
                    DB_STRCPY(node.service_name, name_buff);
                    modify_prog(node.prog_id, &node);
                }
                break;
            }
        }
        i++;
    }

    return changed;
}

static sie_status_t sdt_handle(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *buf, INT32 length)
{
    if(NULL == buf)
    {
        return sie_started;
    }

    INT32 loop_len = 0;
    INT32 tmp_len = 0;
    UINT16 service_id = 0;
    UINT8 desc_len = 0;
    UINT8 desc_tag = 0;
    UINT8 *desc = NULL;
    UINT8 *name = NULL;
    BOOL changed = FALSE;
    UINT16 ts_id = (buf[3]<<8)|buf[4];
    UINT16 net_id = (buf[8]<<8)|buf[9];

    buf += 11;
    length -= (11+PSI_SECTION_CRC_LENGTH);

    while(length > 0)
    {
        service_id = (UINT16)((buf[0]<< 8)|buf[1]);
        loop_len = (UINT16)(((buf[3]&0x0F)<< 8)|buf[4]);
        desc = buf+5;
        tmp_len = loop_len;

        while(tmp_len > 0)
        {
            desc_tag = desc[0];
            desc_len = desc[1];

            if(desc_tag == SERVICE_DESCRIPTOR)
            {
                name = desc+desc[3]+4;
                if(process_sdt(service_id, name, ts_id, net_id))
                {
                    changed = TRUE;
                }
                break;
            }

            desc += (desc_len+2);
            tmp_len -= (desc_len+2);
        }
        buf += (loop_len+5);
        length -= (loop_len+5);
    }

    //update to flash
    if(changed)
    {
        update_data();
    }

    return sie_started;
}

void sdt_monitor_on()
{
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;

    sdt_info.version = 0xFF;
    sdt_info.network_id = 0;
    sdt_info.ts_id = 0;

    fparam.timeout = OSAL_WAIT_FOREVER_TIME;
    fparam.attr[0] = SI_ATTR_HAVE_CRC32;
    fparam.mask_value.mask_len = 6;
    fparam.mask_value.mask[0] = 0xFF;
    fparam.mask_value.mask[1] = 0x80;
    fparam.mask_value.mask[5] = 0x01;

    fparam.mask_value.value_num = 1;
    fparam.mask_value.value[0][0] = PSI_SDT_TABLE_ID;
    fparam.mask_value.value[0][1] = 0x80;
    fparam.mask_value.value[0][5] = 0x01;
    fparam.section_event = sdt_event;
    fparam.section_parser = sdt_handle;

    filter = sie_alloc_filter(PSI_SDT_PID, sdt_buff, PSI_SHORT_SECTION_LENGTH, PSI_SHORT_SECTION_LENGTH);
    sie_config_filter(filter, &fparam);
    sie_enable_filter(filter);
}

void sdt_monitor_off()
{
    sie_abort(PSI_SDT_PID, NULL);
}

#endif

#ifdef SUPPORT_FRANCE_HD
void nit_monitor_register(nit_change_cb change_callback)
{
    nit_change = change_callback;
}

void nit_monitor_unregister()
{
    nit_change = NULL;
}

#ifdef _ISDBT_ENABLE_
void nit_monitor_ews_register(nit_change_cb change_callback)
{
    nit_ews_detect = change_callback;
}

void nit_monitor_ews_unregister()
{
    nit_ews_detect = NULL;
}
#endif

static BOOL nit_event(UINT8 *buffer)
{
    if(NULL == buffer)
    {
        return FALSE;
    }

    BOOL need_reset = FALSE;
    struct section_header *sh = (struct section_header *)buffer;
    UINT8 section_number = sh->section_number;
    UINT8 last_section_number = sh->last_section_number;

    //this section first hit, need set table sections hit flag bitmap
    if (0 == nit_ext.hit)
    {
        nit_ext.version = sh->version_number;
        nit_ext.last_section_number = last_section_number;
        //nit_ext.hit++;
        need_reset = TRUE;
    }
    //this section not first hit, but table version changed,need reget the table sections
    else if (sh->version_number != nit_ext.version)
    {
        nit_ext.version = sh->version_number;
        need_reset = TRUE;
    }
    //this section not first hit, not need reget
    else
    {
        need_reset = FALSE;
    }
    //set table sections hit flag bitmap
    if (TRUE == need_reset)
    {
        MEMSET(nit_ext.flag, 0xFF, (last_section_number>>3)+1);
        nit_ext.hit = 0;
    }
    //this section not the one wanted, return false
    else if (!(nit_ext.flag[section_number>>3]&(1<<(section_number&7))))
    {
        return FALSE;
    }

    nit_ext.flag[section_number>>3] &= ~(1<<(section_number&7));
    nit_ext.hit++;

    return TRUE;
}


static INT32 nit_callback(UINT8 *section, INT32 length, UINT32 param)
{
    UINT16 net_id = 0;
    UINT8 version_num = 0;
    UINT8* data = NULL;
    UINT8 initalized = 0;
    UINT8 version = 0;
    UINT8 version_change = 0;
    UINT8 full = FALSE;
    UINT8 change = FALSE;
    UINT16 i = 0;
    UINT16 j = 0;

    struct section_parameter sec_param;
    struct nit_section_info *nit_info = NULL;
    struct nit_section_info *tmp_nit_info = NULL;

    struct desc_table nit_loop2[] =
    {
        {
            .tag = SERVICE_LIST_DESCRIPTOR,
            .bit = 0,
            .parser = si_on_service_list_desc,
        },
    };

    if(NULL == section)
    {
        return !SI_SUCCESS;
    }

    data = section;
    net_id = (data[3]<<8) | data[4];

#ifndef POLAND_SPEC_SUPPORT
#ifdef _ISDBT_ENABLE_
    if((net_id == france_hd_network_id)||(net_id == brazil_network_id))
#else
    if(net_id == france_hd_network_id)
#endif
#endif
    {
        if(!nit_event(section))
        {
            return SI_SUCCESS;
        }

        nit_info = &(nit_table_info.nit);

        MEMSET(&sec_param, 0, sizeof(sec_param));
        MEMSET(nit_info, 0, sizeof(struct nit_section_info));

        sec_param.priv = (void*)nit_info;
        nit_info->lp1_nr = 0;
        nit_info->lp2_nr = ARRAY_SIZE(nit_loop2);
        nit_info->loop2 = nit_loop2;

        if(si_nit_parser(section, length, &sec_param)==SI_SUCCESS)
        {}

        #ifdef _ISDBT_ENABLE_
        if(get_ews_des_exist())
        {
            if(nit_ews_detect != NULL)
            {
                nit_ews_detect(0,0);
            }
        }
        #endif

        if(nit_ext.hit > nit_ext.last_section_number)
        {
            full = TRUE;
            nit_ext.hit = 0;
        }

        if(TRUE == full)
        {
            nit_info = &(nit_table_info.nit);
            tmp_nit_info = &(t_nit_table_info.nit);

            if(nit_info->version_num != tmp_nit_info->version_num)
            {
                if(nit_info->s_nr != tmp_nit_info->s_nr)
                {
                    change = TRUE;
                }
                else
                {
                    for(i=0; i<nit_info->s_nr; i++)
                    {
                        for(j=0; j<tmp_nit_info->s_nr; j++)
                        {
                            if(nit_info->s_info[i].onid == tmp_nit_info->s_info[j].onid
                                && nit_info->s_info[i].tsid== tmp_nit_info->s_info[j].tsid
                                &&nit_info->s_info[i].sid== tmp_nit_info->s_info[j].sid)
                            {
                                break;
                            }
                        }

                        if(j == tmp_nit_info->s_nr)
                        {
                            change = TRUE;
                            break;
                        }
                    }
                }

                if(TRUE == change)
                {
                    if(nit_change != NULL)
                    {
                        nit_change(0,0);
                    }
                }
                MEMCPY(&t_nit_table_info, &nit_table_info, sizeof(struct NIT_TABLE_INFO));
            }
        }
    }

    return SI_SUCCESS;
}

void nit_monitor_on()
{
    struct dmx_device *dmx = NULL;
    dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);

    /*start monitoring*/
    nit_monitor_id = sim_start_monitor(dmx, MONITE_TB_NIT, 0x0010, 0);

    /*register callback function*/
    sim_register_scb(nit_monitor_id, nit_callback, NULL);

    nit_ext.flag = nit_flag;
    nit_ext.hit = 0;
}

void nit_monitor_off()
{
    if(0xFFFFFFFF == nit_monitor_id)
    {
        return;
    }

    sim_stop_monitor(nit_monitor_id);
    nit_monitor_id = 0xFFFFFFFF;
}


void nit_monitor_init(struct nit_section_info *nit_info)
{
    UINT8 len = 0;

    if(NULL == nit_info)
    {
        return ;
    }

    MEMSET(&t_nit_table_info, 0, sizeof(struct NIT_TABLE_INFO));
    t_nit_table_info.nit.version_num = nit_info->version_num;
    t_nit_table_info.nit.s_nr = nit_info->s_nr;
    if(t_nit_table_info.nit.s_nr>PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        t_nit_table_info.nit.s_nr=PSI_MODULE_MAX_NIT_SINFO_NUMBER;
    }

    len = t_nit_table_info.nit.s_nr*sizeof(struct nit_service_info);
    MEMCPY(&t_nit_table_info.nit.s_info[0], &nit_info->s_info[0], len);

    PRINTF(" version = %x\n",t_nit_table_info.nit.version_num);
    PRINTF(" num = %x\n",t_nit_table_info.nit.s_nr);
}

void load_nit_monitor_info(struct nit_section_info *nit_info)
{
    if(NULL != nit_info)
    {
        MEMCPY(nit_info,  &t_nit_table_info.nit, sizeof(struct nit_section_info));
    }
}
//=================SDT Other monitoir========================

void sdt_other_monitor_callback(on_sdt_change_t pid_change ,void * p_sdt_info)
{
    sdt_on_change = pid_change;
}

void sdt_other_monitor_on(UINT32 index)
{
    struct dmx_device *dmx = NULL;
    UINT32 param[2] = {0};
    P_NODE node;
    T_NODE t_node;

    dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
    MEMSET(&param[0], 0, sizeof(param));

    if(get_prog_at(index, &node) != SUCCESS)
    {
        return ;
    }

    if(get_tp_by_id(node.tp_id, &t_node) != SUCCESS)
    {
        return ;
    }

    param[0] = t_node.t_s_id;
    param[1] = t_node.network_id;

    //m_CurChanIndex = index;
    if(0 == node.user_modified_flag)
    {
        MEMCPY(&program_other, &node, sizeof(P_NODE));
    }
    /*start monitoring*/

    sdt_monitor_id = sim_start_monitor(dmx, MONITE_TB_SDT_OTHER, PSI_SDT_PID, (UINT32)&param[0]);
    // unregister to ensure new index set success.
    sim_unregister_scb(sdt_monitor_id, sdt_other_callback);

    /*register callback function*/
    sim_register_scb(sdt_monitor_id, sdt_other_callback, index);

    sdt_other_ext.flag = sdt_other_flag;
    sdt_other_ext.hit = 0;
}

static BOOL sdt_other_event(UINT8 *buffer)
{
    if(NULL == buffer)
    {
        return FALSE;
    }

    BOOL need_reset = FALSE;
    struct section_header *sh = (struct section_header *)buffer;

    UINT8 section_number = sh->section_number;
    UINT8 last_section_number = sh->last_section_number;

    //this section first hit, need set table sections hit flag bitmap
    if (0 == sdt_other_ext.hit)
    {
        sdt_other_ext.version = sh->version_number;
        sdt_other_ext.last_section_number = last_section_number;
        sdt_other_ext.hit++;
        need_reset = TRUE;
    }
    //this section not first hit, but table version changed,need reget the table sections
    else if (sh->version_number != sdt_other_ext.version)
    {
        sdt_other_ext.version = sh->version_number;
        need_reset = TRUE;
    }
    //this section not first hit, not need reget
    else
    {
        need_reset = FALSE;
    }
    //set table sections hit flag bitmap
    if (TRUE == need_reset)
    {
        MEMSET(sdt_other_ext.flag, 0xFF, (last_section_number>>3)+1);
    }
    //this section not the one wanted, return false
    else if (!(sdt_other_ext.flag[section_number>>3]&(1<<(section_number&7))))
    {
        return FALSE;
    }
    sdt_other_ext.flag[section_number>>3] &= ~(1<<(section_number&7));

    return TRUE;
}

#ifdef DYNAMIC_SERVICE_SUPPORT
void sdt_service_monitor_register(sdt_service_change_cb callback)
{
    sdt_service_update_cb = callback;
}

void sdt_service_monitor_unregister()
{
    sdt_service_update_cb = NULL;
}

/* NOTE: MUST free (*pg_number_array) after calling this function */
static int get_prog_number_under_tp(T_NODE *t_node, UINT16 **_pg_number_array)
{
    //extern DB_TABLE db_table[];
    P_NODE p_node;
    UINT16 i = 0;
    UINT16 pg_num = 0;
    UINT32 node_id = 0;
    UINT32 node_addr = 0;
    UINT32 ret = 0;
    UINT16 *pg_number_array = NULL;
    DB_TABLE *t = db_get_table(TYPE_PROG_NODE);//(DB_TABLE *)&db_table[TYPE_PROG_NODE];

    if((NULL == t_node) || (NULL == _pg_number_array))
    {
        return 0;
    }

    if (t->node_num == 0)
    {
        (*_pg_number_array) = NULL;
        return 0;
    }

    pg_number_array = (UINT16 *)MALLOC(t->node_num * sizeof(UINT16));
    if(NULL == pg_number_array)
    {
        DYN_SERV_DEBUG("%s(): malloc pg_number_array failed!\n", __FUNCTION__);
        return 0;
    }
    for (i = 0, pg_num = 0; i < t->node_num; ++i)
    {
        // get program info
        ret = db_get_node_by_pos_from_table(t, i, &node_id, &node_addr);
        if (ret == DB_SUCCES)
        {
            ret = db_read_node(node_id, node_addr,(UINT8 *)&p_node, sizeof(P_NODE));
        }
        if (ret != DB_SUCCES)
        {
            DYN_SERV_DEBUG("%s(): get program info failed!\n", __FUNCTION__);
            continue;
        }

        if ((p_node.sat_id == t_node->sat_id) && (p_node.tp_id == t_node->tp_id))
        {
            pg_number_array[pg_num++] = p_node.prog_number;
        }
    }
    (*_pg_number_array) = pg_number_array;
    return pg_num;
}

/* check services in SDT with database */
static int sdt_db_check_service_change(struct SDT_TABLE_INFO *sdt, UINT32 chk_type)
{
    int ret = 0;
    int pg_num = 0;
    int i = 0;
    int j = 0;
    T_NODE t_node;
    UINT16 *pg_number_array = NULL;

    if(NULL == sdt)
    {
        return 0;
    }

    MEMSET(&t_node, 0, sizeof(T_NODE));

    if (get_tp_by_id(program.tp_id, &t_node) != DB_SUCCES)
    {
        return ret;
    }

    pg_num = get_prog_number_under_tp(&t_node, &pg_number_array);
    DYN_SERV_DEBUG("In database: %d services under TP 0x%X\n", pg_num, t_node.tp_id);

    if (chk_type & SDT_SERVICE_ADD) // check if any service is added
    {
        for (i = 0; i < sdt->sd_count; ++i)
        {
            for (j = 0; j < pg_num; ++j)
            {
                if (pg_number_array[j] == sdt->sd[i].program_number)
                {
                    break; // find service in database
                }
            }

            if (j >= pg_num)
            {
                DYN_SERV_DEBUG("service 0x%X added\n", sdt->sd[i].program_number);
                ret |= SDT_SERVICE_ADD;
            }
        }
    }

    if (chk_type & SDT_SERVICE_REMOVE) // check if any service is removed
    {
        for (i = 0; i < pg_num; ++i)
        {
            for (j = 0; j < sdt->sd_count; ++j)
            {
                if (sdt->sd[j].program_number == pg_number_array[i])
                {
                    break; // find old service in new SDT
                }
            }

            if (j >= sdt->sd_count)
            {
                DYN_SERV_DEBUG("service 0x%X removed\n", pg_number_array[i]);
                ret |= SDT_SERVICE_REMOVE;
            }
        }
    }

    if ((ret != 0) && sdt_service_update_cb)
    {
        sdt_service_update_cb(ret, 0);
    }

    if (pg_number_array != NULL)
    {
        FREE(pg_number_array);
        pg_number_array = NULL;
    }

    return ret;
}

static int sdt_check_service_change(struct SDT_TABLE_INFO *sdt_new,struct SDT_TABLE_INFO *sdt_old,UINT32 chk_type)
{
    int ret = 0;
    UINT8 i = 0;
    UINT8 j = 0;

    if((NULL == sdt_new) || (NULL == sdt_new))
    {
        return 0;
    }

    if (chk_type & SDT_SERVICE_ADD) // check if any service is added
    {
        for (i = 0; i < sdt_new->sd_count; ++i)
        {
            for (j = 0; j < sdt_old->sd_count; ++j)
            {
                if (sdt_old->sd[j].program_number == sdt_new->sd[i].program_number)
                {
                    break; // find service in old SDT
                }
            }

            if (j >= sdt_old->sd_count)
            {
                DYN_SERV_DEBUG("service 0x%X added\n", sdt_new->sd[i].program_number);
                ret |= SDT_SERVICE_ADD;
            }
        }
    }

    if (chk_type & SDT_SERVICE_REMOVE) // check if any service is removed
    {
        for (i = 0; i < sdt_old->sd_count; ++i)
        {
            for (j = 0; j < sdt_new->sd_count; ++j)
            {
                if (sdt_new->sd[j].program_number == sdt_old->sd[i].program_number)
                {
                    break; // find old service in new SDT
                }
            }

            if (j >= sdt_new->sd_count)
            {
                DYN_SERV_DEBUG("service 0x%X removed\n", sdt_old->sd[i].program_number);
                ret |= SDT_SERVICE_REMOVE;
            }
        }
    }

    if ((ret != 0) && sdt_service_update_cb)
    {
        sdt_service_update_cb(ret, 0);
    }
    return ret;
}
#endif /* DYNAMIC_SERVICE_SUPPORT */

static INT32 tnt_sdt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct SDT_TABLE_INFO *sdt = NULL;
    UINT8 *loop = NULL;
    INT32 i = 0;
    INT32 j = 0;
    INT32 descriptors_length,ret=SI_PERROR;
    UINT16 service_id = 0;
    UINT8 running_status=0;
    UINT8 ori_running_status = 0;
    UINT8 run=0;
    UINT8 ori_run=0;
    UINT8 k = 0;
    UINT16 lk_ts_id = 0;
    UINT16 lk_ori_network_id = 0;
    UINT16 lk_service_id = 0;
    UINT8 lk_dec_type = 0;
    UINT16 num = 0;
    P_NODE p_node;
    T_NODE t_node;
    T_NODE cur_t_node;
    UINT32 cur_tp_id = 0;
    UINT32 index = 0;
    BOOL first_hit = FALSE;
    UINT8 sdt_ver_old = sdt_ext.version;
    UINT8 service_replacement_type = 0x05;
    struct SDT_TABLE_INFO *sdt_old = NULL; // old SDT info from previous SDT

    first_hit = (sdt_ext.hit == 0) ? TRUE : FALSE; // this section first hit

    if(!sdt_event(section)  //if section unchanged, return immediately)
    {
        return SI_SUCCESS;
    }

    sdt = &sdt_info;
    loop = section+11;
    length -= (11+PSI_SECTION_CRC_LENGTH);
    sdt_old = (struct SDT_TABLE_INFO *)MALLOC(sizeof(struct SDT_TABLE_INFO));
    if(NULL == sdt_old)
    {
        DYN_SERV_DEBUG("%s(): malloc sdt_old failed!\n",__FUNCTION__);
        return SI_FAILED;
    }
    if (first_hit)
    {
        MEMSET(sdt_old, 0, sizeof(struct SDT_TABLE_INFO)); // first hit
    }
    else
    {
        MEMCPY(sdt_old, sdt, sizeof(struct SDT_TABLE_INFO)); // save the old SDT info
    }

    MEMSET(sdt, 0, sizeof(struct SDT_TABLE_INFO));

    for(i=0; i<length; i+=(descriptors_length+5))
    {
        service_id = (UINT16)((loop[i]<< 8)|loop[i+1]);
        descriptors_length = (UINT16)(((loop[i+3]&0x0F)<< 8)|loop[i+4]);

        for(j=0; j<sdt_old->sd_count; j++)
        {
            if(service_id == sdt_old->sd[j].program_number)
            {
                break;
            }
        }

        if (sdt->sd_count == PSI_MODULE_MAX_PROGRAM_NUMBER)
        {
            break;
        }

        if (j >= sdt_old->sd_count)
        {
            ori_running_status = RS_NOT_RUNNING; // not exist in previous SDT
        }
        else
        {
            ori_running_status = sdt_old->sd[j].service_running_status;
        }

        running_status = (UINT8)((loop[i+3]&0xE0)>>5);
        if((ori_running_status == RS_UNDEF) ||(ori_running_status == RS_RUNNING))
        {
            ori_run = 1;//running
        }
        else
        {
            ori_run=0;//Not running
        }

        if((running_status == RS_UNDEF) ||(running_status == RS_RUNNING))
        {
            run = 1;//running
        }
        else
        {
            run=0;//Not running
        }
        if((run != ori_run) && (program.prog_number == service_id))
        {
            if(!run)
            {
                if(service_replacement_type == sdt_old->sd[j].linkage_dec_type)//sevice replace
                {
                    cc_set_cur_channel_idx(m_cur_chan_index);
                    lk_ori_network_id=sdt_old->sd[j].linkage_ori_network_id;
                    lk_ts_id=sdt_old->sd[j].linkage_ts_id;
                    lk_service_id=sdt_old->sd[j].linkage_service_id;

                    get_tp_by_id(program.tp_id, &cur_t_node);//cur play t_node
                    num = get_prog_num(VIEW_ALL|PROG_TVRADIO_MODE, 0);
                    for(i = 0; i < num; i++)
                    {
                        get_prog_at(i, &p_node);
                        get_tp_by_id(p_node.tp_id, &t_node);
                        if((lk_ori_network_id== t_node.network_id)
                            &&(lk_ts_id== t_node.t_s_id)
                            &&(lk_service_id == p_node.prog_number))
                        {
                                if(t_node.tp_id==cur_t_node.tp_id)//ACTAUL  ???????????
                                {
                                    link_status=LINKAGE_ACTAUL;
                                    index=i;
                                    break;
                                }
                                else if((t_node.tp_id!=cur_t_node.tp_id))//OTHER
                                {
                                    link_status=LINKAGE_OTHER;
                                    index=i;
                                    break;
                                }
                        }
                    }
                    sdt_on_change(i);   //send message
                }
                else
                {
                    link_status=LINKAGE_NOT;
                }
            }
        }

        sdt->sd[sdt->sd_count].service_running_status = running_status;
        sdt->sd[sdt->sd_count].program_number = service_id;
        sdt->sd[sdt->sd_count].name_pos = 0;
        sdt_desc_parser(loop+i+5, descriptors_length, &(sdt->sd[sdt->sd_count]));
        sdt->sd_count++;
        ret = SI_SUCCESS;
    }

    //pass sdt running status to AP
    if(p_sdt_running_info)
    {
        p_sdt_running_info->sd_count=sdt->sd_count;
        for(k=0; k<sdt->sd_count; k++)
        {
            p_sdt_running_info->running_data[k].service_id = sdt->sd[k].program_number;
            p_sdt_running_info->running_data[k].running_status = sdt->sd[k].service_running_status;
        }
    }

#ifdef DYNAMIC_SERVICE_SUPPORT
    if (sdt_ver_old != sdt_ext.version)
    {
        DYN_SERV_DEBUG("SDT version change: %d --> %d\n", sdt_ver_old, sdt_ext.version);
        sdt_db_check_service_change(sdt, SDT_SERVICE_ADD);

        if (!first_hit)
        {
            sdt_check_service_change(sdt, sdt_old, SDT_SERVICE_REMOVE);
        }
    }
#endif
    FREE(sdt_old);
    sdt_old = NULL;
    return ret;
}

static INT32 sdt_other_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct SDT_TABLE_INFO *sdt = NULL;
    UINT8 *loop = NULL;
    INT32 i = 0;
    INT32 j = 0;
    INT32 descriptors_length,ret = SI_PERROR;
    UINT16 service_id = 0;
    UINT8 running_status = 0;
    UINT8 ori_running_status = 0;
    UINT8 run = 0;
    UINT8 ori_run = 0;
    UINT8 k = 0;
    UINT16 lk_ts_id = 0;
    UINT16 lk_ori_network_id = 0;
    UINT16 lk_service_id = 0;
    UINT8 lk_dec_type = 0;
    UINT16 num = 0;
    P_NODE p_node;
    T_NODE t_node;
    T_NODE cur_t_node;
    UINT32 cur_tp_id = 0;
    UINT32 index = 0;
    static UINT8 count = 0;
    struct sim_cb_param *cb_param = (struct sim_cb_param *)(param);

    index =(UINT16)cb_param->priv;
    if(!sdt_other_event(section))
    {
        return SI_SUCCESS;
    }

    sdt = &sdt_other_info;
    loop = section+11;
    length -= (11+PSI_SECTION_CRC_LENGTH);

    for(i=0; i<length; i+=(descriptors_length+5))
    {
        service_id = (UINT16)((loop[i]<< 8)|loop[i+1]);
        descriptors_length = (UINT16)(((loop[i+3]&0x0F)<< 8)|loop[i+4]);

        for(j=0; j<sdt->sd_count; j++)
        {
            if(service_id == sdt->sd[j].program_number)
            {
                break;
            }
        }

        if (sdt->sd_count == PSI_MODULE_MAX_PROGRAM_NUMBER)
        {
            break;
        }

        ori_running_status = sdt->sd[j].service_running_status;
        running_status = (UINT8)((loop[i+3]&0xE0)>>5);

        if((running_status == RS_UNDEF) ||(running_status == RS_RUNNING))
        {
           run = 1;//running
        }
        else
        {
           run=0;//Not running
        }

        if((run)&&(program_other.prog_number == service_id))
        {
            if(sdt_other_return)
            {
                sdt_other_return(run,index,link_status);
            }
        }

        sdt->sd[j].program_number = service_id;
        sdt->sd[j].name_pos = 0;
        sdt_desc_parser(loop+i+5, descriptors_length, &(sdt->sd[j]));

        if(j==sdt->sd_count)
        {
            sdt->sd_count++;
        }
                //pass sdt running status to AP
        if(p_sdt_running_info)
        {
            p_sdt_running_info->sd_count=sdt->sd_count;
            for( k=0 ; k<sdt->sd_count ; k++)
            {
                p_sdt_running_info->running_data[k].service_id = sdt->sd[k].program_number;
                p_sdt_running_info->running_data[k].running_status = sdt->sd[k].service_running_status;
            }
        }
        ret = SI_SUCCESS;
    }

    return ret;
}

void sdt_monitor_retrun_callback(on_sdt_other_return_t pid_change ,void *p_sdt_info)
{
    sdt_other_return = pid_change;
    p_sdt_running_info = p_sdt_info;
}

static INT32  sdt_on_linkage_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    struct program_data *sd = (struct program_data *)priv;

    sd->linkage_ts_id =(desc[0]<<8)|desc[1];
    sd->linkage_ori_network_id = (desc[2]<<8)|desc[3];
    sd->linkage_service_id =(desc[4]<<8)|desc[5];
    sd->linkage_dec_type=desc[6];

    return SI_SUCCESS;
}

#endif

