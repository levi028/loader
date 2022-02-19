/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_sdt.c
*
*    Description: parse SDT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_sdt.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_service_type.h>

#define SS_DEBUG_LEVEL          0
#if (SS_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SS_PRINTF           libc_printf
#else
#define SS_PRINTF(...)          do{}while(0)
#endif
#ifdef CI_SERVICE_SHUNNING_DEBUG_PRINT
#define CI_SHUNNING_DEBUG libc_printf
#else
#define CI_SHUNNING_DEBUG(...) do{} while(0)
#endif

#define SDT_DESCRIPTOR_MAX_CNT      8
#define CI_PRIVATE_DATA_SPECIFIER   0x00000040

static INT32 sdt_on_service_descriptor(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
static INT32 sdt_on_mtilingual_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif
static INT32 sdt_on_nvod_ref_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
static INT32 sdt_on_timeshift_service_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
static INT32 sdt_on_media_service_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
#endif
#ifdef SUPPORT_DEFAULT_AUTHORITY
static INT32 sdt_on_default_authority_desc(UINT8 tag, UINT8 length, UINT8 *data,void *priv);
#endif
#ifdef CI_PLUS_SERVICESHUNNING
INT32 sdt_private_data_specifier_desc(UINT8 tag, UINT8 len, UINT8 *desc_data, void *priv);
INT32 sdt_ci_protection_desc(UINT8 tag, UINT8 len, UINT8 *desc_data, void *priv);
#endif

struct nvod_reference_descriptor_ripped
{
    struct
    {
        UINT8 transport_stream_id[2];
        UINT8 original_network_id[2];
        UINT8 service_id[2];
    }__attribute__((packed)) ref_list[0];
}__attribute__((packed));

static struct desc_table sdt_desc_table[SDT_DESCRIPTOR_MAX_CNT] = {
    {
        .tag = SERVICE_DESCRIPTOR,
        .bit = 0,
        .parser = sdt_on_service_descriptor,
    },
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    {
        .tag = MULTILINGUAL_SERVICE_NAME_DESCRIPTOR,
        .bit = 0,
        .parser = sdt_on_mtilingual_service_desc,
    },
#endif
    {
        .tag = NVOD_REFERENCE_DESCRIPTOR,
        .bit = 0,
        .parser = sdt_on_nvod_ref_desc,
    },
    {
        .tag = TIME_SHIFTED_SERVICE_DESCRIPTOR,
        .bit = 0,
        .parser = sdt_on_timeshift_service_desc,
    },
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
    {
        .tag = 0xA1,
        .bit = 0,
        .parser = sdt_on_media_service_desc,
    },
#endif
#ifdef _INVW_JUICE

#ifdef SUPPORT_DEFAULT_AUTHORITY
    {
        .tag = DEFAULT_AUTHORITY_DESCRIPTOR,
        .bit = 0,
        .parser = sdt_on_default_authority_desc,
    },
#endif

#endif
#ifdef CI_PLUS_SERVICESHUNNING
    {
        .tag    = PRIVATE_DATA_SPECIFIER_DESCRIPTOR,
        .bit    = 0,
        .parser = sdt_private_data_specifier_desc,
    },
    {
        .tag    = CI_PROTECTION_DESCRIPTOR,
        .bit    = 0,
        .parser = sdt_ci_protection_desc,
    }
#endif
};

#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
static UINT32 sdt_parse_descriptor_cnt = 5;
#else
static UINT32 sdt_parse_descriptor_cnt = 4;
#endif

#else
static UINT32 sdt_parse_descriptor_cnt = 5;
#endif

#ifdef NVOD_ZHANGZHOU
static struct nvod_zhangzhou_data  nvod_sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
static UINT8 nvod_sd_count;
#endif

static UINT16 sd_prog_nr = 0;

static INT32 sdt_on_service_descriptor(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct sdt_section_info *s_info = NULL;
    struct service_data *desc = NULL;
    struct service_descriptor_ripped *sdr = NULL;
    INT32 name_pos = -1;
    INT32 pos = -1;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    s_info = (struct sdt_section_info *)priv;
    desc = s_info->sd;
    name_pos = s_info->name_pos;
    pos = s_info->sd_nr;
    sdr = (struct service_descriptor_ripped *)data;
#ifdef CELRUN_SOUTH_AMERICA /* modify to support DVB-C channel in South America */
    switch( sdr->service_type )
    {
          /* 0x80~0xA0 : Service provider defined */
          case 0x82: // 130
          case 0x80: // 128
          case 0x86: // 134
          case 0x99: // 153
          case 0x9b: // 155
             desc[pos].service_type = 0x01; //SERVICE_TYPE_DTV;
             s_info->service_type = 0x01;   //SERVICE_TYPE_DTV;
             break;
          default:
             desc[pos].service_type = sdr->service_type;
             s_info->service_type = sdr->service_type;
             break;
    }
#else
    desc[pos].service_type = sdr->service_type;
    s_info->service_type = sdr->service_type;
#endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    if ((name_pos+sdr->service_provider_name_length)<s_info->max_name_len)
    {
        desc[pos].service_provider_length = sdr->service_provider_name_length;
        desc[pos].service_provider = (UINT8 *)MEMCPY(s_info->names+name_pos,
            sdr->service_provider_name,
            sdr->service_provider_name_length);
        name_pos += sdr->service_provider_name_length;
    }
    else
    {
        return SI_SBREAK;
    }
#endif

    sdr = (struct service_descriptor_ripped *)(data+sdr->service_provider_name_length);
    if (name_pos+sdr->service_name_length<s_info->max_name_len)
    {
        desc[pos].service_name_length = sdr->service_name_length;
        desc[pos].service_name = (UINT8 *)MEMCPY(s_info->names+name_pos, sdr->service_name, sdr->service_name_length);
        name_pos += sdr->service_name_length;
    }
    else
    {
        s_info->name_pos = name_pos;
        return SI_SBREAK;
    }
    s_info->name_pos = name_pos;
    SS_PRINTF("%s(): service id [%d],service type[%d],name len [%d], provider name len [%d]\n",
       __FUNCTION__,s_info->sid,desc[pos].service_type,desc[pos].service_name_length,desc[pos].service_provider_length);

    return SI_SUCCESS;
}

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
static INT32 sdt_on_media_service_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct sdt_section_info *s_info = NULL;
    UINT8 media_service_flag = STAR_SDT_SERVICE_UNSAVE;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    s_info = (struct sdt_section_info *)priv;
    MEMSET(s_info->media_service_code, 0, STAR_SDT_SERVICE_CODE_LEN+1);
    MEMCPY(s_info->media_service_code, data, STAR_SDT_SERVICE_CODE_LEN);

    if(0 == STRCMP(s_info->media_service_code, STAR_SDT_SERVICE_CODE_VALUE))
    {
        media_service_flag = STAR_SDT_SERVICE_SAVE;
    }

    if(s_info->sd_nr < PSI_MODULE_MAX_PROGRAM_NUMBER)
    {
       s_info->sd[s_info->sd_nr].media_service_flag = media_service_flag;
    }

    return SI_SUCCESS;
}
#endif

#ifdef _INVW_JUICE
#ifdef SUPPORT_DEFAULT_AUTHORITY
static INT32 sdt_on_default_authority_desc(UINT8 tag, UINT8 length, UINT8 *data,void *priv)
{
    struct sdt_section_info *s_info = NULL;
    UINT8 pos = 0;

    if((NULL == data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    s_info = (struct nit_section_info *)priv;
    while (length >0)
    {
        s_info->service_provider_name[pos]= data[pos];
        data += 1;
        length -= 1;
        pos++;
        SS_PRINTF("%s(): default_authority[%x] : %x \n",__FUNCTION__,pos,data[pos]);
    }

    return SI_SUCCESS;
}
#endif

#endif
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
static INT32 sdt_on_mtilingual_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 i        = 0;
    UINT8 k        = 0;
    UINT8 name_len = 0;
    UINT8 prov_len = 0;
    UINT8 *dst     = NULL;
    struct sdt_section_info *s_info = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    s_info = (struct sdt_section_info *)priv;
    k = s_info->sd_nr;
    i = s_info->mtl_s_data[k].mult_name_count;

    while(len > 0)
    {
        prov_len = desc[3];

        //s_info->mtl_s_data[0].mtl_name[i].ISO_639_language_code = (desc[k]<<16)|(desc[k+1]<<8)|desc[k+2];
        s_info->mtl_s_data[k].mtl_name[i].iso_639_language_code[0] = desc[0];
        s_info->mtl_s_data[k].mtl_name[i].iso_639_language_code[1] = desc[1];
        s_info->mtl_s_data[k].mtl_name[i].iso_639_language_code[2] = desc[2];

        dst = s_info->mtl_s_data[k].names+s_info->mtl_s_data[k].name_pos;
        s_info->mtl_s_data[k].mtl_name[i].service_provider_name_length = prov_len;
        s_info->mtl_s_data[k].mtl_name[i].service_provider_name =(UINT8*)MEMCPY(dst, desc+4,prov_len);
        desc += (prov_len+4);
        name_len = desc[0];
        //k = (prov_len+4+k);
        //name_len = desc[k];
        s_info->mtl_s_data[k].name_pos += prov_len;

        dst = s_info->mtl_s_data[k].names+s_info->mtl_s_data[k].name_pos;
        s_info->mtl_s_data[k].mtl_name[i].service_name_length = name_len;
        s_info->mtl_s_data[k].mtl_name[i].service_name =(UINT8*)MEMCPY(dst, desc+1, name_len);
        s_info->mtl_s_data[k].name_pos+=name_len;

        len -= 5+prov_len+name_len;
        desc += (name_len+1);
        //k = (name_len+1+k);
        i++;
    }

    s_info->mtl_s_data[k].program_number = s_info->sd[k].program_number;
    s_info->mtl_s_data[k].mult_name_count = i;
    return SI_SUCCESS;
}

#endif

static INT32 sdt_on_nvod_ref_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    INT32 i   = 0;
    INT32 j   = 0;
    INT32 cnt = length/6;
    struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
    struct nvod_reference_descriptor_ripped *nrd = NULL;
    UINT8 max_ref_service_cnt = sizeof(s_info->nvod.ref_id)/sizeof(s_info->nvod.ref_id[0]);
    UINT8 max_service_cnt     = sizeof(s_info->nvod.sid)/sizeof(s_info->nvod.sid[0]);
    UINT16 t_s_id       = 0;
    UINT16 origin_netid = 0;
    UINT16 service_id   = 0;

    if(NULL == data)
    {
        return !SI_SUCCESS;
    }

    nrd = (struct nvod_reference_descriptor_ripped *)data;

    //if(s_info->service_type==SERVICE_TYPE_NVOD_REF)
    {
        for(j=0; j < s_info->nvod.ref_cnt; j++)
        {
            if(s_info->sid==s_info->nvod.ref_id[j])
            {
                break;
            }
        }

        if(j == s_info->nvod.ref_cnt)
        {
            if(s_info->nvod.ref_cnt < max_ref_service_cnt)
            {
                s_info->nvod.ref_id[s_info->nvod.ref_cnt++] = s_info->sid;
            }
            else
            {
                SS_PRINTF("nvod_reference descriptor: ref service cnt to max cnt\n");
            }
        }
    }

    for(i = 0; i<cnt; i++)
    {
        t_s_id       = SI_MERGE_UINT16(nrd->ref_list[i].transport_stream_id);
        origin_netid = SI_MERGE_UINT16(nrd->ref_list[i].original_network_id);
        service_id   = SI_MERGE_UINT16(nrd->ref_list[i].service_id);
        SS_PRINTF("nvod_reference descriptor: ts_id[%d],origin_netid[%d], service_id[%d]\n",
            t_s_id,origin_netid,service_id);
        if((t_s_id==s_info->t_s_id) )
        {
            for(j=0; j< s_info->nvod.scnt; j++)
            {
                if(service_id==s_info->nvod.sid[j].sid)
                {
                    break;
                }
            }

            if(j==s_info->nvod.scnt)
            {
                if(s_info->nvod.scnt < max_service_cnt)
                {
                    s_info->nvod.sid[s_info->nvod.scnt].sid = service_id;
                    s_info->nvod.sid[s_info->nvod.scnt].ref_id = s_info->sid;
                    s_info->nvod.sid[s_info->nvod.scnt].t_s_id = t_s_id;
                    s_info->nvod.sid[s_info->nvod.scnt].origin_netid = origin_netid;
                    s_info->nvod.scnt++;
                }
                else
                {
                    SS_PRINTF("nvod_reference descriptor: service cnt to max cnt\n");
                }
            }
        }
    }

    return SI_SUCCESS;
}

#ifdef CI_PLUS_SERVICESHUNNING
INT32 sdt_private_data_specifier_desc(UINT8 tag, UINT8 len, UINT8 *desc_data, void *priv)
{
    if((NULL == desc_data) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
    struct service_data *service    = &s_info->sd[s_info->sd_nr];

    service->private_data_specifier = (desc_data[0]<<24)|(desc_data[1]<<16)|(desc_data[2]<<8)|(desc_data[3]);
    CI_SHUNNING_DEBUG("\t private_data_specifier = 0x%X\n", service->private_data_specifier);

    if (0x00000040 == service->private_data_specifier)
    {
        CI_SHUNNING_DEBUG("\t YES, specifier RIGHT\n");
    }
    else
    {
        CI_SHUNNING_DEBUG("\t NO, specifier WRONG\n");
    }

    return SI_SUCCESS;
}

INT32 sdt_ci_protection_desc(UINT8 tag, UINT8 len, UINT8 *desc_data, void *priv)
{
    UINT8 data = 0;
    struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
    struct service_data *service = &s_info->sd[s_info->sd_nr];

    if ((tag != CI_PROTECTION_DESCRIPTOR) || (NULL == desc_data))
    {
        return SI_EINVAL;
    }

    /* Before ci protection descriptor, should recv private data specifier descriptor */
    if (service->private_data_specifier != CI_PRIVATE_DATA_SPECIFIER)
    {
        return SI_SUCCESS;
    }

    CI_SHUNNING_DEBUG("# %s\n", __FUNCTION__);

    data = desc_data[0];

    /* free_ci_mode_flag */
    if (0x00 == (desc_data[0] & 0x80))
    {
        service->shunning_protected = 0;
        CI_SHUNNING_DEBUG("\t free_ci_mode_flag = 0\n");
    }
    else
    {
        service->shunning_protected = 1;
    }

    if (service->shunning_protected)
    {
        CI_SHUNNING_DEBUG("\t Service [ %d ] HAS shunning protected information\n", service->program_number );
    }
    else
    {
        CI_SHUNNING_DEBUG("\t Service [ %d ] NO shunning protected information\n", service->program_number );
    }

    return SI_SUCCESS;
}
#endif

static INT32 sdt_on_timeshift_service_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
    struct service_data *pservice = s_info->sd;
    INT32 pos = s_info->sd_nr;
    INT32 i = 0;
    UINT16 nvod_ref_sid = 0;
    UINT8 max_ref_service_cnt = sizeof(s_info->nvod.ref_id)/sizeof(s_info->nvod.ref_id[0]);
    UINT8 max_service_cnt     = sizeof(s_info->nvod.sid)/sizeof(s_info->nvod.sid[0]);

    if((0 == length) || (NULL == data))
    {
        return SI_SUCCESS;
    }

    nvod_ref_sid = (data[0]<<8)|data[1];
    SS_PRINTF("time shift service descriptor, reference_service_id[%d]\n",nvod_ref_sid);

    if(pos < PSI_MODULE_MAX_PROGRAM_NUMBER)
    {
        pservice[pos].timeshift_flag = 1;
        pservice[pos].ref_service_id = nvod_ref_sid;
    }
    else
    {
        return SI_SUCCESS;
    }

    for(i=0; i< s_info->nvod.ref_cnt; i++)
    {
       if(nvod_ref_sid==s_info->nvod.ref_id[i])
       {
            break;
       }
    }

    if(i==s_info->nvod.ref_cnt)
    {
        if(s_info->nvod.ref_cnt < max_ref_service_cnt)
        {
            s_info->nvod.ref_id[s_info->nvod.ref_cnt++] = nvod_ref_sid;
        }
        else
        {
            SS_PRINTF("time shift service descriptor: ref service cnt to max cnt\n");
        }
    }

    //check if this service_id already saved
    for(i=0; i< s_info->nvod.scnt; i++)
    {
        if(pservice[pos].program_number==s_info->nvod.sid[i].sid)
        {
            break;
        }
    }

    if(i==s_info->nvod.scnt)
    {
        if(s_info->nvod.scnt < max_service_cnt)
        {
            s_info->nvod.sid[s_info->nvod.scnt].sid = pservice[pos].program_number;
            s_info->nvod.sid[s_info->nvod.scnt].ref_id = nvod_ref_sid;
            s_info->nvod.sid[s_info->nvod.scnt].t_s_id = s_info->t_s_id;
            s_info->nvod.sid[s_info->nvod.scnt].origin_netid = s_info->onid;
            s_info->nvod.scnt++;
        }
        else
        {
            SS_PRINTF("time shift service descriptor: service cnt to max cnt\n");
        }
    }


    return SI_SUCCESS;
}

/*static INT32 si_sdt_add_priv_descriptor(struct desc_table *pdescriptor, UINT8 descriptor_cnt)
{
    INT32 i = 0;
    INT8 add_cnt = 0;

    if(descriptor_cnt >=SDT_DESCRIPTOR_MAX_CNT)
    {
        SS_PRINTF("%s(), invalid param\n",__FUNCTION__);
        return -1;
    }

    for(i = 0; i < SDT_DESCRIPTOR_MAX_CNT; i++)
    {
        if((0==sdt_desc_table[i].tag)&&(NULL == sdt_desc_table[i].parser))
        {
            sdt_parse_descriptor_cnt = i;
            break;
        }
    }
    if (SDT_DESCRIPTOR_MAX_CNT <= i)
    {
        SS_PRINTF("%s(), no valid!\n");
        return -1;
    }
    if((i+descriptor_cnt) >SDT_DESCRIPTOR_MAX_CNT)
    {
        SS_PRINTF("%s(), add descriptor cnt[%d] exceed max cnt!\n",__FUNCTION__,descriptor_cnt);
        add_cnt = SDT_DESCRIPTOR_MAX_CNT - i ;
    }
    else
    {
        add_cnt = descriptor_cnt;
    }

    if(add_cnt > 0)
    {
        MEMCPY(&sdt_desc_table[i], pdescriptor, add_cnt*sizeof(struct desc_table));
        sdt_parse_descriptor_cnt = i+add_cnt;
    }

    return SI_SUCCESS;
}*/

#ifdef NVOD_ZHANGZHOU
UINT8 get_nvod_sd_count()
{
    return nvod_sd_count;
}
void set_nvod_sd_count(UINT8 count)
{
    nvod_sd_count = count;
}
void* get_nvod_sd( UINT8 * nvod_count)
{
    *nvod_count = nvod_sd_count;
    return (void *)&nvod_sd;
}
#endif

INT32 si_sdt_parser(UINT8 *section, INT32 length, struct section_parameter *param)
{
    INT32 i = 0;
#ifdef NVOD_ZHANGZHOU
    UINT16 name_len = 0;
#endif
    INT32 descriptors_length = 0;
    INT32 ret = 0;
    struct sdt_stream_info *info    = NULL;
    struct sdt_section_info *s_info = (struct sdt_section_info *)param->priv;
    UINT8  default_service_name[7]  = {0x4E,0x6F,0x20,0x4E,0x61,0x6D,0x65};

    if (NULL == section)
    {
        s_info->sd_nr    = 0;
        s_info->name_pos = 0;
        s_info->nd_nr    = 0;
        sd_prog_nr       = 0;
        return SI_SUCCESS;
    }

    s_info->onid        = (section[8]<<8)|section[9];
    s_info->sdt_version = (section[5]&0x3E)>>1;
    s_info->t_s_id      = (section[3]<<8)|section[4];

#ifdef NVOD_ZHANGZHOU
    MEMSET(nvod_sd, 0, sizeof(struct nvod_zhangzhou_data)*PSI_MODULE_MAX_PROGRAM_NUMBER);
    nvod_sd_count=0;
#endif

    SS_PRINTF("\n%s(): ts_id[%d], origin_netid[%d]\n", __FUNCTION__,s_info->t_s_id, s_info->onid);
    for(i=sizeof(struct sdt_section)-PSI_SECTION_CRC_LENGTH;
        i<length-PSI_SECTION_CRC_LENGTH;
        i+=sizeof(struct sdt_stream_info)+descriptors_length)
    {
        info = (struct sdt_stream_info *)(section+i);
        descriptors_length = SI_MERGE_HL8(info->descriptor_loop_length);
        s_info->sid = SI_MERGE_UINT16(info->service_id);
        s_info->sd[s_info->sd_nr].program_number = s_info->sid;
        SS_PRINTF("service id [%d]\n", s_info->sid);

        ret = desc_loop_parser(info->descriptor, descriptors_length,
            sdt_desc_table, sdt_parse_descriptor_cnt, NULL, s_info);

#ifdef NVOD_ZHANGZHOU
        if(s_info->sd[s_info->sd_nr].service_type==SERVICE_TYPE_NVOD_REF)//0x04
        {
            nvod_sd[nvod_sd_count].program_number=s_info->sd[s_info->sd_nr].program_number;
            nvod_sd[nvod_sd_count].ref_service_id=s_info->sd[s_info->sd_nr].ref_service_id;
            nvod_sd[nvod_sd_count].service_name_length=s_info->sd[s_info->sd_nr].service_name_length;
            nvod_sd[nvod_sd_count].service_type=SERVICE_TYPE_NVOD_REF;
            if(s_info->sd[s_info->sd_nr].service_name_length<=NVOD_REF_MAX_LEN)
            {
                name_len = s_info->sd[s_info->sd_nr].service_name_length;
                MEMCPY(nvod_sd[nvod_sd_count].service_name,s_info->sd[s_info->sd_nr].service_name, name_len);
            }
            nvod_sd_count++;
        }
#endif
        if(0 == s_info->sd[s_info->sd_nr].service_name_length)
        {//Set NoName as default name
         s_info->sd[s_info->sd_nr].service_name_length = 7;
         s_info->sd[s_info->sd_nr].service_name=(UINT8 *)MEMCPY(s_info->names+s_info->name_pos,&default_service_name,7);
         s_info->name_pos+=7;
        }
        s_info->sd_nr++;
        sd_prog_nr=s_info->sd_nr;

        if (ret != SI_SUCCESS)
        {
            SS_PRINTF("%s: name space not big enough!\n", __FUNCTION__);
            return SI_SUCCESS;
        }
    }
    return SI_SUCCESS;
}

static void get_hstr(UINT8 *hstr, UINT8 *src, INT32 len)
{
    *hstr = len;
    MEMCPY(hstr+1, src, len);
}

INT32 psi_get_service(struct sdt_section_info *info,UINT16 program_number,  UINT8 *service_name, UINT8 *provider_name)
{
    INT32 i = 0;
    struct service_data *sdesc = NULL;

    if((NULL == info) || (NULL == service_name))
    {
        return !SI_SUCCESS;
    }

    sdesc = info->sd;
    for(i=0; i<info->sd_nr; i++, sdesc++)
    {
        if (sdesc->program_number == program_number)
        {
            if (sdesc->service_name_length > 0)
            {
                get_hstr(service_name,sdesc->service_name,sdesc->service_name_length);
            }
            else
            {
            #ifndef INDIA_LCN
                psi_fake_name(service_name, program_number);
            #else
                psi_fake_name(service_name, program_number);
            #endif
            }

#if (SERVICE_PROVIDER_NAME_OPTION>0)
            if(sdesc->service_provider_length > 0)
            {
                get_hstr(provider_name,sdesc->service_provider,sdesc->service_provider_length);
            }
            else
            {
                psi_fake_name(provider_name, 0);
            }
#endif
            return SI_SUCCESS;
        }
    }
    SS_PRINTF("program_number: %d, no service name!\n", program_number);
    #ifndef INDIA_LCN
    psi_fake_name(service_name, program_number);
    #else
    psi_fake_name(service_name, program_number);
    #endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    psi_fake_name(provider_name, 0);
#endif
    return SI_SUCCESS;
}

INT32 psi_get_service_type(struct sdt_section_info *info, UINT16 program_number, UINT8 *service_type)
{
    INT32 i = 0;
    struct service_data *sdesc = NULL;

    if((NULL == info) || (NULL == service_type))
    {
        return !SI_SUCCESS;
    }

    sdesc = info->sd;

    for(i=0; i<info->sd_nr; i++, sdesc++)
    {
        if (sdesc->program_number == program_number)
        {
            *service_type = sdesc->service_type;
            return SI_SUCCESS;
        }
    }

    return SI_SKIP;
}

#if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
INT32 psi_get_star_service_code(struct sdt_section_info *info, UINT16 program_number, UINT8 *service_flag)
{
    INT32 i = -1;
    struct service_data *sdesc = NULL;

    if((NULL == info) || (NULL == service_flag))
    {
        return SI_SKIP;
    }

    sdesc = info->sd;

    for(i=0; i<info->sd_nr; i++, sdesc++)
    {
        if (sdesc->program_number == program_number)
        {
            *service_flag = sdesc->media_service_flag;
            return SI_SUCCESS;
        }
    }

    return SI_SKIP;
}
#endif

#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
struct mtl_service_data *psi_get_multi_service_name(struct sdt_section_info *info, UINT16 program_number)
{
    INT32 i = 0;
    struct mtl_service_data *ret = NULL;
    struct mtl_service_data *data = NULL;

    if(NULL == info)
    {
        return NULL;
    }

    data = info->mtl_s_data;

    for(i=0; i < i<info->sd_nr && i<PSI_MODULE_MAX_PROGRAM_NUMBER; i++, data++)
    {
        if (data->program_number == program_number)
        {
            ret = data;
            break;
        }
    }

    return ret;
}
#endif


