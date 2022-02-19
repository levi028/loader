/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_nit.c
*
*    Description: parse NIT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/desc_loop_parser.h>
#ifdef ITALY_HD_BOOK_SUPPORT
#include <api/libtsi/psi_db.h>
#endif

#define SN_DEBUG_LEVEL          0
#if (SN_DEBUG_LEVEL > 0)
#include <api/libc/printf.h>
#define SN_PRINTF           libc_printf
#else
#define SN_PRINTF(...)          do{}while(0)
#endif

#ifdef _ISDBT_ENABLE_
static INT8 ews_des_exist = 0;   //default:not exist;
#endif

#if(defined(_LCN_ENABLE_) ||defined(_SERVICE_ATTRIBUTE_ENABLE_))

enum LCN_state_valid_flag LCN_describe_flag = LCN_INIT_FLAG;
struct nit_service_info *lcn_bak;
static UINT16 lcn_cnt = 0;

INT32 si_on_priv_spec_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    n_info->priv_spec = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|(data[3]);

    return SI_SUCCESS;
}
#endif

#ifdef _INVW_JUICE
#if(defined(SUPPORT_NETWORK_NAME))
INT32 nit_network_name_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = (struct nit_section_info *)priv;
    UINT8 pos = 0;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    while (length > 0)
    {
        n_info->network_provider_name[pos]= data[pos];
        data += 1;
        length -= 1;
        pos++;
    }

    return SI_SUCCESS;
}
#endif

#endif
#if(defined( _MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
INT32 si_on_lcn_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 min_data_len = 4;
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    Setup_LCN_describe_flag(LCN_VALID_FLAG);
    if((NULL == n_info) || (NULL == data))
    {
        return SI_SUCCESS;
    }

    if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
        return SI_SUCCESS;
    }

    while (length >= min_data_len)
    {
#ifdef SUPPORT_FRANCE_HD
        UINT16 i;
        for(i=0; i<n_info->s_nr; i++)
        {
            if(n_info->s_info[i].sid == ((data[0]<<8)|data[1]))
            {
                break;
            }
        }

        if(i == n_info->s_nr)
        {
            n_info->s_info[n_info->s_nr].onid = n_info->onid;
            n_info->s_info[n_info->s_nr].tsid = n_info->tsid;
            n_info->s_info[n_info->s_nr].sid = (data[0]<<8)|data[1];
            n_info->s_info[n_info->s_nr].lcn = ((data[2]&0x03)<<8)|data[3];
        #ifdef ITALY_HD_BOOK_SUPPORT
            n_info->s_info[n_info->s_nr].visible_service_flag = (data[2] >> 7);
        #endif
            SN_PRINTF("LCN[%d]: %d - onid: 0x%X, tsid: 0x%X, sid: 0x%X\n",
                        n_info->s_nr,
                        n_info->s_info[n_info->s_nr].lcn,
                        n_info->s_info[n_info->s_nr].onid,
                        n_info->s_info[n_info->s_nr].tsid,
                        n_info->s_info[n_info->s_nr].sid
                        );

            n_info->s_nr ++;
        }
        else
        {
            n_info->s_info[i].onid = n_info->onid;
            n_info->s_info[i].tsid = n_info->tsid;
            n_info->s_info[i].sid = (data[0]<<8)|data[1];
            n_info->s_info[i].lcn = ((data[2]&0x03)<<8)|data[3];
        #ifdef ITALY_HD_BOOK_SUPPORT
            n_info->s_info[i].visible_service_flag = (data[2] >> 7);
        #endif
            SN_PRINTF("LCN[%d]: %d - onid: 0x%X, tsid: 0x%X, sid: 0x%X\n",
                        n_info->s_nr,
                        n_info->s_info[i].lcn,
                        n_info->s_info[i].onid,
                        n_info->s_info[i].tsid,
                        n_info->s_info[i].sid
                        );
        }

#else
        n_info->s_info[n_info->s_nr].onid = n_info->onid;
        n_info->s_info[n_info->s_nr].tsid = n_info->tsid;
        n_info->s_info[n_info->s_nr].sid = (data[0]<<8)|data[1];
        n_info->s_info[n_info->s_nr].lcn = ((data[2]&0x03)<<8)|data[3];
        SN_PRINTF("LCN[%d]: %d - onid: 0x%X, tsid: 0x%X, sid: 0x%X\n",
                    n_info->s_nr,
                    n_info->s_info[n_info->s_nr].lcn,
                    n_info->s_info[n_info->s_nr].onid,
                    n_info->s_info[n_info->s_nr].tsid,
                    n_info->s_info[n_info->s_nr].sid
                    );
        n_info->s_nr ++;
#endif
        if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
            return SI_SUCCESS;
        }

        data += 4;
        length -= 4;
    }

    return SI_SUCCESS;
}

INT32 si_nit_get_info(struct nit_section_info *nsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
    INT32 i = 0;
    INT32 ret_code = ERR_FAILUE;

    if((NULL == lcn_bak) || (NULL == pg))
    {
        return ret_code;
    }
#ifdef HD_SIMULCAST_LCD_SUPPORT
    for(i=0; i<nsi->s_nr; i++)
    {
        if ((nsi->s_info[i].sid == pg->prog_number) && (nsi->s_info[i].tsid == tsid) && (nsi->s_info[i].onid == onid))
        {
            pg->lcn = nsi->s_info[i].lcn;
        #ifndef ITALY_HD_BOOK_SUPPORT
            if(nsi->s_info[i].lcn== 0)  //for DTG test SI05
            {
                pg->lcn_true = FALSE;
            }
            else
            {
                pg->lcn_true = TRUE;
            }
        #else
            if(nsi->s_info[i].lcn== INVALID_LCN_NUM)  //for DTG test SI05
            {
                pg->lcn_true = FALSE;
            }
            else
            {
                pg->lcn_true = TRUE;
            }
        #endif
            SN_PRINTF("found lcn: %d\n",pg->lcn);
        #ifdef ITALY_HD_BOOK_SUPPORT
            pg->lcn_visible_service_flag = nsi->s_info[i].visible_service_flag;
        #endif
        }
    }
    SN_PRINTF("not found lcn of prog 0x%X!!\n",pg->prog_number);

    for(i=0; i<nsi->hd_nr; i++)
    {
        if ((nsi->hd_info[i].sid == pg->prog_number)&&(nsi->hd_info[i].tsid == tsid)&&(nsi->hd_info[i].onid == onid))
        {
            pg->hd_lcn = nsi->hd_info[i].hd_lcn;
            if(nsi->hd_info[i].hd_lcn== 0)  //for DTG test SI05
            {
                pg->hd_lcn_true = FALSE;
            }
            else
            {
                pg->hd_lcn_true = TRUE;
                SN_PRINTF("found hd_lcn: %d,hd_lcn=%x\n",pg->hd_lcn,pg->hd_lcn_true );
            }
            return SI_SUCCESS;
        }
    }
    SN_PRINTF("not found lcn of prog 0x%X!!\n",pg->prog_number);
#else
    for(i=0; i<lcn_cnt; i++)
    {
        if ((lcn_bak[i].sid == pg->prog_number)&&(lcn_bak[i].tsid == tsid)&&(lcn_bak[i].onid == onid))
        {
            pg->lcn = lcn_bak[i].lcn;
            if(lcn_bak[i].lcn== 0)  //for DTG test SI05
            {
                pg->lcn_true = FALSE;
            }
            else
            {
                pg->lcn_true = TRUE;
            }
            SN_PRINTF("found lcn: %d\n",pg->lcn);
            return SI_SUCCESS;
        }
    }

    SN_PRINTF("not found lcn of prog 0x%X!!\n",pg->prog_number);

#endif
    return ret_code;
}

#ifdef _ISDBT_ENABLE_
INT32 si_nit_check_prog_info(struct nit_section_info *nsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
    INT32 i = 0;

    if((NULL == nsi) || (NULL == pg))
    {
        return ERR_FAILUE;
    }

    for(i=0; i<nsi->s_nr; i++)
    {
        if ((nsi->s_info[i].sid == pg->prog_number)&&(nsi->s_info[i].tsid == tsid)&&(nsi->s_info[i].onid == onid))
        {
            nsi->network_sel[i]=TRUE;
            return SI_SUCCESS;
        }
        else
        {
            nsi->network_sel[i]=FALSE;
        }
    }
    SN_PRINTF("not found lcn of prog 0x%X!!\n",pg->prog_number);
    return ERR_FAILUE;
}

void set_ews_des_exist(INT8 exist)
{
    ews_des_exist = exist;
}

INT8 get_ews_des_exist(void)
{
    return ews_des_exist;
}

INT32 nit_sys_management_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 len = length;
    UINT8 i   = 0;
    UINT8 additional_identification_info[64] = {0};
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    n_info->system_manage_id = (data[0]<<8)|data[1];
    len -= 2;

    for(i = 0; i < len; ++i)
    {
        additional_identification_info[i] = data[i+2];
    }

    return SI_SUCCESS;
}

INT32 nit_emergency_information_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = (struct nit_section_info *)priv;
    UINT16 area_code_len = 0;
    UINT8  j = 0;
    INT16  length_bak = length;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
        return SI_SUCCESS;
    }

    while (length_bak > 0) /*>= otherwise it will always true*/
    {
        n_info->ews_info[n_info->ews_nr].sid = (data[0]<<8)|data[1];
        n_info->ews_info[n_info->ews_nr].start_end_flag =data[2]&0x01;

        if(n_info->ews_info[n_info->ews_nr].start_end_flag)
        {
          set_ews_des_exist(1);
        }
        else
        {
          set_ews_des_exist(0);
        }

        n_info->ews_info[n_info->ews_nr].signal_level = (data[2]&0x02)>>1;
        area_code_len = data[3];

        for (j=0;j<area_code_len ;j++)
        {
            n_info->ews_info[n_info->ews_nr].area_code[2*j]=data[4+2*j];//bit[1:5]:state,bit[6:12]:region
            n_info->ews_info[n_info->ews_nr].area_code[2*j+1]=data[4+2*j+1]&0xf;
            area_code_len -= 2;
            length_bak -= 2;
        }

        n_info->ews_nr ++;

        if (n_info->ews_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            SN_PRINTF("%s: maximum hd_simul_lcn collected!\n", __FUNCTION__);
            return SI_SUCCESS;
        }

        data += 4;
        length_bak -= 4;
    }
    return SI_SUCCESS;
}
#endif

#ifdef HD_SIMULCAST_LCD_SUPPORT
INT32 si_on_hdsimul_lcn_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 min_data_len = 4;
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
        return SI_SUCCESS;
    }

#if(PRIVATE_DATA_SPEC != INVALID_PRIVATE_DATA_SPEC)
    if(n_info->priv_spec != PRIVATE_DATA_SPEC)
    {
        return SI_SUCCESS;
    }
#endif

    //parse lcn loop
    while (length >= min_data_len)
    {
        n_info->hd_info[n_info->hd_nr].onid = n_info->onid;
        n_info->hd_info[n_info->hd_nr].tsid = n_info->tsid;
        n_info->hd_info[n_info->hd_nr].sid = (data[0]<<8)|data[1];
        n_info->hd_info[n_info->hd_nr].hd_lcn = ((data[2]&0x03)<<8)|data[3];
        SN_PRINTF("tag=%x,LCN[%d]: %d - onid: 0x%X, tsid: 0x%X, sid: 0x%X\n",
                    tag,
                    n_info->hd_nr,
                    n_info->hd_info[n_info->hd_nr].hd_lcn,
                    n_info->hd_info[n_info->hd_nr].onid,
                    n_info->hd_info[n_info->hd_nr].tsid,
                    n_info->hd_info[n_info->hd_nr].sid
                    );
        n_info->hd_nr ++;

        if (n_info->hd_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            SN_PRINTF("%s: maximum hd_simul_lcn collected!\n", __FUNCTION__);
            return SI_SUCCESS;
        }

        data += 4;
        length -= 4;
    }

    return SI_SUCCESS;
}
#endif
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
INT32 si_on_attribute_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 min_data_len = 3;
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    if (n_info->attrib_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        return SI_SUCCESS;
    }

#if(PRIVATE_DATA_SPEC != INVALID_PRIVATE_DATA_SPEC)
    if(n_info->priv_spec != PRIVATE_DATA_SPEC)
    {
        return SI_SUCCESS;
    }
#endif

    while (length >= min_data_len)
    {
        n_info->attrib_info[n_info->attrib_nr].onid = n_info->onid;
        n_info->attrib_info[n_info->attrib_nr].tsid = n_info->tsid;
        n_info->attrib_info[n_info->attrib_nr].sid = (data[0]<<8)|data[1];
        n_info->attrib_info[n_info->attrib_nr].nsf = (data[2] & 0x2)>>1;
        n_info->attrib_info[n_info->attrib_nr].vsf = data[2] & 0x1;

        n_info->attrib_nr ++;

        if (n_info->attrib_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            return SI_SUCCESS;
        }

        data += 3;
        length -= 3;
    }

    return SI_SUCCESS;
}
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
INT32 si_service_get_attrib(struct nit_section_info *nsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
    INT32 i = 0;

    if((NULL == nsi) || (NULL == pg))
    {
        return ERR_FAILUE;
    }

    for(i=0; i<nsi->attrib_nr; i++)
    {
        if ((nsi->attrib_info[i].sid == pg->prog_number)
            &&(nsi->attrib_info[i].tsid == tsid)
            &&(nsi->attrib_info[i].onid == onid))
        {
            pg->numeric_selection_flag = nsi->attrib_info[i].nsf;
            pg->visible_service_flag = nsi->attrib_info[i].vsf;
            return SI_SUCCESS;
        }
    }

    pg->numeric_selection_flag = 1;
    pg->visible_service_flag = 1;

    return ERR_FAILUE;
}
#endif

#ifdef SUPPORT_FRANCE_HD
INT32 si_on_service_list_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = (struct nit_section_info *)priv;
    INT16 i = 0;
    UINT8 min_data_len = 3;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
    {
        SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
        return SI_SUCCESS;
    }

    while (length >= min_data_len)   //parse lcn loop
    {
        for(i=0; i<n_info->s_nr; i++)
        {
            if(n_info->s_info[i].sid == ((data[0]<<8)|data[1]))
            {
                break;
            }
        }

        if(i == n_info->s_nr)
        {
            n_info->s_info[n_info->s_nr].onid = n_info->onid;
            n_info->s_info[n_info->s_nr].tsid = n_info->tsid;
            n_info->s_info[n_info->s_nr].sid = (data[0]<<8)|data[1];
            n_info->s_nr ++;
        }

        if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
        {
            SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
            return SI_SUCCESS;
        }

        data += 3;
        length -= 3;
    }

    return SI_SUCCESS;
}

#endif

#if defined(_ISDBT_ENABLE_)
INT32 si_on_ts_information_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    struct nit_section_info *n_info = (struct nit_section_info *)priv;

    if((NULL == data) || (NULL == n_info))
    {
        return !SI_SUCCESS;
    }

    n_info->remote_control_key_id = data[0];

    return SI_SUCCESS;
}
#endif


INT32 si_on_extension_descriptor(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    if(NULL == data)
    {
        return !SI_SUCCESS;
    }

#if defined(DVBT2_SUPPORT)
    if(T2_DELIVERY_SYSTEM_DESCRIPTOR == data[0])
    {
        return t2_delivery_system_descriptor_parser(tag, length, data, priv);
    }
#endif

    return SI_SUCCESS;
}

INT32 si_nit_parser(UINT8 *data, INT32 len, struct section_parameter *param)
{
    INT32 i         = 0;
    INT32 ret       = 0;
    INT32 dloop_len = 0;
    struct network_descriptor    *nw_desc = NULL;
    struct nit_section_info      *n_info  = NULL;
    struct transport_stream_info *ts_info = NULL;

    if ((NULL == data) || (NULL == param))
    {
        return SI_SUCCESS;
    }
    if (lcn_bak == NULL)
    {
        lcn_cnt = 0;
        lcn_bak = (struct nit_service_info *)MALLOC(sizeof(struct nit_service_info)*PSI_MODULE_MAX_NIT_SINFO_NUMBER);
    }
    if (LCN_describe_flag == LCN_INIT_FLAG)
        Setup_LCN_describe_flag(LCN_INVALID_FLAG);
    n_info = (struct nit_section_info *)param->priv;
#if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_) \
    || defined(NETWORK_ID_ENABLE) || defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT) || defined(SUPPORT_FRANCE_HD))
    n_info->net_id = (data[3]<<8) | data[4];
#endif

    n_info->ext[0].version = (data[5]&0x3E)>>1;
#ifdef SUPPORT_FRANCE_HD
    n_info->version_num = (data[5] &0x3e) >> 1;
#endif

    nw_desc = &((struct nit_section *)data)->network;
    dloop_len = SI_MERGE_HL8(nw_desc->network_descriptor_length);

    if (n_info->lp1_nr&&(dloop_len > 0))
    {
#if (defined(AUTO_OTA) || defined(_ISDBT_ENABLE))
        ret = desc_loop_parser(nw_desc->descriptor, dloop_len, n_info->loop1,
            n_info->lp1_nr, n_info->p_firstloopinfo.descriptors_parse_stat, n_info);
#else
        ret = desc_loop_parser(nw_desc->descriptor, dloop_len, n_info->loop1, n_info->lp1_nr, NULL, n_info);
#endif
        if (ret != SI_SUCCESS)
        {
            SN_PRINTF("%s: loop1 parse failure!\n", __FUNCTION__);
            return ret;
        }
#ifdef AUTO_OTA
        if(0 == (n_info->p_firstloopinfo.descriptors_parse_stat)&(1<<NIT_LINKAGE_EXIST_BIT))
        {
            set_linkagetype09_exist(0);
        }
#endif
    }

    if (0 == n_info->lp2_nr)
    {
        return SI_SUCCESS;
    }

    for(i=sizeof(struct nit_section)+dloop_len-4; i<len-4; i += sizeof(struct transport_stream_info)+dloop_len)
    {
        ts_info      = (struct transport_stream_info *)(data+i);
        n_info->tsid = SI_MERGE_UINT16(ts_info->transport_stream_id);
        n_info->onid = SI_MERGE_UINT16(ts_info->original_network_id);
        dloop_len    = SI_MERGE_HL8(ts_info->transport_stream_length);
        if (dloop_len>0)
        {
             ret = desc_loop_parser(ts_info->descriptor,dloop_len, n_info->loop2, n_info->lp2_nr, NULL, n_info);
             if (ret != SI_SUCCESS)
             {
                 SN_PRINTF("%s: loop2 parse failure!\n", __FUNCTION__);
                 if (lcn_bak != NULL)
                 {
                    lcn_cnt = n_info->s_nr;
                    MEMCPY(lcn_bak, n_info->s_info, sizeof(struct nit_service_info)*PSI_MODULE_MAX_NIT_SINFO_NUMBER);//LCN信息备份
                 }
                 return ret;
             }
        }
    }
     if (lcn_bak != NULL)
     {
        lcn_cnt = n_info->s_nr;
        MEMCPY(lcn_bak, n_info->s_info, sizeof(struct nit_service_info)*PSI_MODULE_MAX_NIT_SINFO_NUMBER);//LCN信息备份
     }
    return SI_SUCCESS;
}

#if defined(AUTO_OTA)

INT32 get_otaservice_linkage_from_nit(struct nit_section_info *nit_info, INT8 *plink_exist, UINT16 cur_tsid)
{
    INT32 ret = !SI_SUCCESS;
    INT8 linkagetype09_exist = 0;
    UINT8 update_service_type = 0x09;
    struct NITSSU_INFO *pssuinfo = NULL;

    if((NULL == nit_info) || (NULL == plink_exist))
    {
        return !SI_SUCCESS;
    }

    pssuinfo = &nit_info->p_firstloopinfo.ssu_location_info;

    if(0 == linkagetype09_exist)
    {
        linkagetype09_exist = get_linkagetype09_exist();
    }

    if(update_service_type == pssuinfo->linkage_type)
    {
        if(pssuinfo->des_tsid == cur_tsid)
        {
            if(pssuinfo->OUI == si_get_otaconfig_oui())
            {
                si_set_ota_serviceid(pssuinfo->des_serviceid);
                ret = SUCCESS;
            }
        }
    }

    *plink_exist = linkagetype09_exist;

    return ret;
}
#endif

void Setup_LCN_describe_flag(enum LCN_state_valid_flag flag)
{
	LCN_describe_flag = flag;
}
