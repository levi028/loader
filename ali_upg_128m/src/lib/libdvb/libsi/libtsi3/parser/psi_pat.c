/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: psi_pat.c
*
*    Description: parse PAT table
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
#include <api/libsi/psi_pat.h>

#define PP_DEBUG_LEVEL          0
#if (PP_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define PP_PRINTF           libc_printf
#else
#define PP_PRINTF(...)          do{}while(0)
#endif

INT32 psi_pat_parser(UINT8 *section, INT32 length, struct section_parameter *param)
{
    INT32  i = 0;
    INT32  k = 0;
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    UINT32 crc_value = 0;
    UINT32 *crc_addr = NULL;
#endif
    UINT16 program_number           = 0;
    struct pat_stream_info  *info   = NULL;
    struct program_map      *maps   = NULL;
    struct pat_section_info *s_info = NULL;

    if(NULL == param)
    {
        return SI_SUCCESS;
    }

    s_info = (struct pat_section_info *)param->priv;

    if (NULL == section)
    {
        s_info->map_counter = 0;

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
        s_info->crcvalue = 0;
#endif

        return SI_SUCCESS;
    }

    if (0 == section[6])
    {
        s_info->ts_id = (section[3]<<8)|section[4];
    }

    maps = s_info->map;
    PP_PRINTF("\nPAT parse:\n");
    PP_PRINTF("ts_id=%d\n",s_info->ts_id);

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
    crc_addr = (UINT32 *)(section + (length - PSI_SECTION_CRC_LENGTH));
    crc_value = (UINT32)*crc_addr;
    s_info->crcvalue = crc_value;
#endif

    for(i=sizeof(struct pat_section)-PSI_SECTION_CRC_LENGTH;
        i<length-PSI_SECTION_CRC_LENGTH;
        i+= sizeof(struct pat_stream_info))
    {
        info = (struct pat_stream_info *)(section+i);
        program_number = SI_MERGE_UINT16(info->program_number);

        if (program_number)
        {
            for(k=0; k<s_info->map_counter; k++)
            {
                if (maps[k].pm_number == program_number)
                {
                    PP_PRINTF("%s: pgnumber[%d] duplicate at pos %d!\n", __FUNCTION__, program_number, k);
                    break;
                }
            }

            if (k == s_info->map_counter)
            {
                maps[k].pm_number = program_number;
                maps[k].pm_pid = SI_MERGE_HL8(info->pid);

                #ifdef RAPS_SUPPORT
                if(program_number == raps_get_sevice_id())
                {
                    raps_set_pmt_pid(maps[k].pm_pid);
                }
                #endif

                PP_PRINTF("pmt pid[%d], program number[%d]\n", maps[k].pm_pid, maps[k].pm_number);

                if ((++s_info->map_counter) >= s_info->max_map_nr)
                {
                    PP_PRINTF("%s: total prog cnt[%d]>=max cnt[%d]!\n",
                        __FUNCTION__, s_info->map_counter,s_info->max_map_nr);
                    return SI_SBREAK;
                }
            }
        }
        else
        {
            s_info->nit_pid = SI_MERGE_HL8(info->pid);
        }
    }
    PP_PRINTF("%s: parse end!\n", __FUNCTION__);
    return SI_SUCCESS;
}

