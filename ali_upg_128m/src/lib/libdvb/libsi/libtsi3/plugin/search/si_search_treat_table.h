/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_search_treat_table.h
*
*    Description: config parameter for SI table when do program search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SI_SEARCH_TREAT_TABLE__
#define __SI_SEARCH_TREAT_TABLE__

#ifdef __cplusplus
extern "C" {
#endif

#include <api/libsi/si_bat.h>

#define POST_FN_ARRAY_SIZE 5

struct psi_module_info
{
    struct dmx_device *dmx;
    struct nim_device *nim;

    UINT16 search_scheme;
    UINT16 storage_scheme;

    UINT16 valid_counter;
    UINT16 pmt_counter;
    UINT16 nit_counter;
    UINT16 flag256_nr;

    T_NODE xponder[2];
    P_NODE program[3];

    struct si_filter_param fparam;

    struct pat_section_info pat;
    struct sdt_section_info sdt;
    struct nit_section_info nit;
#ifdef COMBOUI
    struct bat_section_info bat;
#endif

    struct section_parameter param[5];
    struct program_map maps[PSI_MODULE_MAX_PROGRAM_NUMBER];
    struct list_head roots[PSI_MODULE_MAX_PROGRAM_NUMBER];
    UINT8 flag256[16][PSI_MODULE_MSECTION_NUMBER>>3];

    struct pmt_es_info es_buff[PSI_MODULE_COMPONENT_NUMBER];
    UINT8 buffer[PSI_SHORT_SECTION_LENGTH*PSI_MODULE_CONCURRENT_FILTER];
};

typedef void(* psi_pre_fn_t)(struct psi_module_info *info, struct section_parameter *sparam);
typedef INT32(* psi_post_fn_t)(struct psi_module_info *info,UINT32 search_scheme,psi_event_t on_event,BOOL *need_tune);

extern OSAL_ID psi_flag;
extern struct psi_module_info *psi_info;
extern const psi_pre_fn_t pre_fn_array[];
extern const psi_post_fn_t post_fn_array[];
extern struct desc_table nit_loop2[];

INT32 psi_wait_stat(UINT32 good, UINT32 clear);
INT32 prog_pre_process(struct service_data *item, PROG_INFO *pg_info);
void psi_init_sr(struct si_filter_param *fparam, const struct section_info *info, struct section_parameter *param,
    si_parser_t parser, INT32 idx);
INT32 psi_store_program(PROG_INFO *program, UINT8 *service_name, UINT8 *provider_name, UINT32 progress,
    UINT16 search_scheme, UINT16 storage_scheme, psi_event_t on_event);
sie_status_t psi_analyze_pmt(UINT16 pid, UINT16 program_number, struct program_map *map, UINT8 *buffer, INT32 length);
void set_search_timeout_for_dvbs(struct section_info *psi_sections, UINT8 length);
UINT8* sdt_get_name(UINT8 *data, UINT32 len);
INT32 psi_prepare_pmt(struct psi_module_info *info,struct section_parameter *sparam);

#ifdef __cplusplus
}
#endif

#endif
