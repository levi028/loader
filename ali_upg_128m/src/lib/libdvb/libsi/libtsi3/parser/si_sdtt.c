/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_sdtt.c
*
*    Description: parse sdtt table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_sdtt.h>
#include <api/libsi/si_tdt.h>
#include <string.h>

#define PP_DEBUG_LEVEL          0
#if (PP_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define PP_PRINTF           libc_printf
#else
#define PP_PRINTF(...)          do{}while(0)
#endif

#define BCD2DEC(a)      ((a>>4)*10+(a&0x0F))
#define SDTT_MERG_UINT16(u8_high,u8_low)   ((u8_low) | (((UINT16)(u8_high))<<8))

static UINT16 m_sdtt_chan_index = 0;

void st_sdtt_set_channel_index(UINT16 chanindex)
{
    m_sdtt_chan_index = chanindex;
}

//Pasering Module Desc
static UINT16 st_sdtt_parser_module_desc(UINT8 *module_desc)
{
    UINT16 i          = 0;
    UINT16 module_num = 0;
    UINT16 total_size = 0;
    struct SDTT_DL_CONT_MOD *module_desc_dat = NULL;

    if(NULL == module_desc)
    {
        return 0;
    }

    module_num = SDTT_MERG_UINT16(*(module_desc+1), *module_desc);
    module_desc_dat = (struct SDTT_DL_CONT_MOD *)(module_desc+1);

    for(i=0; i<module_num; i++)
    {
        total_size += (module_desc_dat->mod_inf_len + sizeof(struct SDTT_DL_CONT_MOD));
        module_desc_dat = (struct SDTT_DL_CONT_MOD *)(((UINT8 *)module_desc_dat)+total_size);
    }

    return total_size;
}

//Pasering Compatibility Desc
static UINT16 si_sdtt_parser_compab_desc(struct SDTT_COMPAB_DESC_1 *compab_desc1)
{
    struct SDTT_COMPAB_DESC_2 *compab_desc2 = NULL;
    UINT16 k          = 0;
    UINT16 compab_len = 0;
    UINT16 compab_cnt = 0;
    UINT32 module     = 0;
    UINT32 version    = 0;

    if(NULL == compab_desc1)
    {
        return 0;
    }

    compab_desc2 = (struct SDTT_COMPAB_DESC_2 *)(((UINT8 *)compab_desc1) + sizeof(struct SDTT_COMPAB_DESC_1));
    compab_len = SDTT_MERG_UINT16(compab_desc1->desc_len_hi,compab_desc1->desc_len_lo)+2;
    compab_cnt = SDTT_MERG_UINT16(compab_desc1->desc_cnt_hi,compab_desc1->desc_cnt_lo);

    for(k=0; k<compab_cnt; k++)
    {
        module = SDTT_MERG_UINT16(compab_desc2->module_hi, compab_desc2->module_lo);
        version = SDTT_MERG_UINT16(compab_desc2->version_hi, compab_desc2->version_lo);

        switch(compab_desc2->desc_type)
        {
            case SDTT_COMPAB_DESC_TYPE_HW:
                {
                    if(module != SYS_HW_MODEL)
                    {
                        return 0;
                    }

                    if(version != SYS_HW_VERSION)
                    {
                        return 0;
                    }
                }
                break;
            case SDTT_COMPAB_DESC_TYPE_SW:
                {
                    if(module != SYS_SW_MODEL)
                    {
                        return 0;
                    }

                    if(version != SYS_SW_VERSION)
                    {
                        return 0;
                    }
                }
                break;
            default:
                return 0;
        }

        compab_desc2 = (struct SDTT_COMPAB_DESC_2 *)(((UINT8 *)compab_desc2) + compab_desc2->desc_len + 2);
    }

    return compab_len;
}

INT32 si_sdtt_parser(UINT8 *section, INT32 length, struct section_parameter *param)
{
    INT32  i                              = -1;
    INT32  j                              = -1;    // sec_offset;
    UINT32 mjd_time                       = 0;
//    UINT32 set_dur_time                   = 0;
//    UINT32 get_dur_time                   = 0;
//    UINT32 f_force                        = 0;
    UINT8  temp                           = 0;
    UINT16 current_ver                    = 0;
    UINT16 cont_desc_len                  = 0;
    UINT16 sche_desc_len                  = 0;
    UINT16 trg_ver                        = 0;
//    UINT16 new_ver                        = 0;
    UINT16 sch_time_n                     = 0;
    UINT16 desc_len                       = 0;
    UINT8  *desc_point                    = NULL;
    UINT32 down_desc_total_len            = 0;
    UINT32 down_desc_pase_size            = 0;
    struct sch_time *sched_time           = NULL;
    struct SDTT_DL_CONT_DESC_1 *down_desc = NULL;
    struct sdtt_section *info             = NULL;
    struct sdtt_content *content          = NULL;
    date_time dt_set;
    date_time dt_get;
    P_NODE p_node;
    T_NODE t_node;

    if (NULL == section)
    {
        return SI_SUCCESS;
    }

    current_ver=SYS_SW_VERSION;
    info = (struct sdtt_section *)section;
    content = (struct sdtt_content *)(section+sizeof(struct sdtt_section));

    for(i=0;i<info->numer_of_content;i++)
    {
        cont_desc_len = content->content_ds_len_lo|(((UINT16)content->content_ds_len_hi)<<4);
        sche_desc_len =  content->schedule_ds_len_lo|(((UINT16)content->schedule_ds_len_hi)<<4);
        sch_time_n = (sche_desc_len/sizeof(struct sch_time));
        down_desc_total_len = cont_desc_len - sche_desc_len;
        sched_time = (struct sch_time *)(((UINT8 *)content)+sizeof(struct sdtt_content));
        down_desc = (struct SDTT_DL_CONT_DESC_1 *)(((UINT8 *)sched_time)+sche_desc_len);

        while(down_desc_pase_size < down_desc_total_len)
        {
            if(down_desc->compab_flag!=1)
            {
                goto NEXT_CONT;
            }

            desc_point = (((UINT8 *)down_desc) + sizeof(struct SDTT_DL_CONT_DESC_1));
            desc_len = si_sdtt_parser_compab_desc((struct SDTT_COMPAB_DESC_1 *)desc_point);

            if(0 == desc_len)
            {
                goto NEXT_CONT;
            }

            down_desc_pase_size += (sizeof(struct SDTT_DL_CONT_DESC_1) + desc_len);
            desc_point += desc_len;

            if(1 == down_desc->mod_inf_flag)
            {
                desc_len = st_sdtt_parser_module_desc(desc_point);
                down_desc_pase_size += desc_len;
                desc_point += desc_len;
            }

            //Private data
            desc_len = (*desc_point);
            desc_point += (desc_len+1);
            down_desc_pase_size += (desc_len+1);

            //text info
            if(1 == down_desc->txt_inf_flag)
            {
                desc_point += 3;    //3 Bytes, ISO_639_Lang_code
                desc_len = (*desc_point);
                desc_point += (desc_len+1);
                down_desc_pase_size += (desc_len+1+3);
            }

            down_desc = (struct SDTT_DL_CONT_DESC_1 *)desc_point;
        }

        //check version
        trg_ver = SDTT_MERG_UINT16(content->target_ver_hi,content->target_ver_lo);
//        new_ver = SDTT_MERG_UINT16(content->new_ver_hi,content->new_ver_lo);

        switch(content->ver_indicator)
        {
            case SDTT_VER_INDICAT_ALL:
                break;
            case SDTT_VER_INDICAT_LATER:
                if(trg_ver<current_ver)
                {
                    goto NEXT_CONT;
                }
                break;
            case SDTT_VER_INDICAT_EARLIER:
                if(current_ver<trg_ver)
                {
                    goto NEXT_CONT;
                }
                break;
            case SDTT_VER_INDICAT_ONLY:
                if(trg_ver!=current_ver)
                {
                    goto NEXT_CONT;
                }
                break;
            default:
                break;
        }

        //Set Timer
        MEMSET(&dt_set, 0x0, sizeof (date_time));
        MEMSET(&dt_get, 0x0, sizeof (date_time));
        MEMSET(&p_node, 0x0, sizeof (P_NODE));
        MEMSET(&t_node, 0x0, sizeof (T_NODE));
//        f_force = content->download_level;

        if(sch_time_n>0)
        {
            for(j=0;j<sch_time_n;j++)
            {
                mjd_time =((((UINT32)sched_time[j].sch_time_start[0])<<8)|(((UINT32)sched_time[j].sch_time_start[1])));

                dt_set.hour = BCD2DEC(sched_time[j].sch_time_start[2]);
                dt_set.min = BCD2DEC(sched_time[j].sch_time_start[3]);
                dt_set.sec = BCD2DEC(sched_time[j].sch_time_start[4]);

                //set_dur_time =( (((UINT32)sched_time[j].sch_time_duration[0])<<16) |
                //            (((UINT32)sched_time[j].sch_time_duration[1])<<8) |
                //            (((UINT32)sched_time[j].sch_time_duration[2])) );

                mjd_to_ymd(mjd_time,&dt_set.year,&dt_set.month,&dt_set.day,&temp);

                dt_get = dt_set;
                //get_dur_time = set_dur_time;
            }
        }
        else
        {
            get_local_time(&dt_set);
            convert_time_by_offset2(&dt_get,&dt_set,0,0,3);   //Set time as 3 second later...
            //get_dur_time = 1;
        }

        if (get_prog_at(m_sdtt_chan_index, &p_node) != SUCCESS)
        {
            return SI_SUCCESS;
        }

        get_tp_by_id(p_node.tp_id, &t_node);

NEXT_CONT:
        content = (struct sdtt_content *)((UINT8 *)content + sizeof(struct sdtt_content) + cont_desc_len);
    }

    return SI_SUCCESS;
}

